#ifndef INCLUDE_PROMISE_WEB_CTX_H_
#define INCLUDE_PROMISE_WEB_CTX_H_

//! \file ctx.h
//! \defgroup api

#include <any>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <exception>

namespace diy  {

//! \private
// return value of callable

template<class T>
class returns
{};

//! \private
template<class T>
class returns<T()>
{
public:
	typedef T type;
};

//! \private
template<class T>
class returns<T(*)()>
{
public:
	typedef T type;
};

//! \private
template<class T, class ... Args>
class returns<T(Args...)>
{
public:
	typedef T type;
};

//! \private
template<class T, class ... Args>
class returns<T(*)(Args...)>
{
public:
	typedef T type;
};

//! \brief Exception thrown if context resolution fails
//! \ingroup api
class ContextEx : public std::exception 
{
public:
	ContextEx()
	{}
};     

//! \private
class Provider;

/** \brief Context API
 *
 * Context implements the main IOC context.
 *
 */
//! \ingroup api

class Context
{
template<class T, class I> friend class Singleton;
public:

    Context()
        : parent_(nullptr)
    {}

    Context(Context* p)
        : parent_(p)
    {}

    virtual ~Context()
    {}

    template<class T>
    std::shared_ptr<T> resolve();

    template<class T, class I = typename returns<T>::type >
    void register_factory();

    template<class T, class I = typename returns<T>::type>
    void register_singleton();

    template<class T, class I = T>
    void register_static( std::shared_ptr<I> i);

private:
    template<class T, typename std::enable_if<!std::is_default_constructible<T>::value>::type* = nullptr>
    std::shared_ptr<T> resolve(Context& ctx);

    template<class T, typename std::enable_if<std::is_default_constructible<T>::value>::type* = nullptr>
    std::shared_ptr<T> resolve(Context& ctx);

    Context* parent_;

    std::map<std::type_index,std::unique_ptr<Provider>> providers_; 
};

//! \private
class Provider
{
public:
    virtual ~Provider() {}
    virtual std::any create(Context& ctx) = 0; 
};

//! \private
template<class I, class T>
class Factory {};

//! \private
template<class I, class R,class ... Args>
class Factory<I, R(Args...)> : public Provider 
{
public:

     typedef I type;

     virtual std::any create(Context& ctx)
     {
         auto a =  std::make_shared<R>( ctx.resolve<typename std::remove_reference<Args>::type>() ...);
         std::shared_ptr<I> i = a;
         return std::any( i );
     }
};

//! \private
template<class I, class T>
class Singleton {};

//! \private
template<class I, class R,class ... Args>
class Singleton<I, R(Args...)> : public Provider {
public:

     typedef I type;

     virtual std::any create(Context& ctx)
     {
         if(!singleton_)
         {
             singleton_ = std::make_shared<R>( ctx.resolve<typename std::remove_reference<Args>::type>() ...);
         }
         return std::any(singleton_);
     }

private:
     std::shared_ptr<I> singleton_;
};

//! \private
template<class T>
class Value : public Provider 
{
public:

     typedef T type;

     Value(std::shared_ptr<T> t)
        : value_(t)
    {}

     virtual std::any create(Context& ctx)
     {
         return std::any(value_);
     }

private:
     std::shared_ptr<T> value_;
};

//! register Factory for T with T in form of F(Args...)
//! \ingroup api
template<class T, class I>
void Context::register_factory()
{
     providers_.insert(
         std::make_pair(
             std::type_index(typeid(I)),
             new Factory<I,T>
         )
     );
}

//! register Singleton for T with T in form of F(Args...)
//! \ingroup api
template<class T,class I>
void Context::register_singleton()
{
     providers_.insert(
         std::make_pair(
             std::type_index(typeid(I)),
             new Singleton<I,T>
         )
     );
}

//! register exisitng value for T 
//! \ingroup api
template<class T, class I>
void Context::register_static( std::shared_ptr<I> i)
{
    providers_.insert(
         std::make_pair(
             std::type_index(typeid(I)),
             new Value<I>(i)
         )
     );
}


//! reseolve a type T from context
//! \ingroup api
template<class T>
std::shared_ptr<T> Context::resolve()
{
     return resolve<T>(*this);
}

//! \private
template<class T, typename std::enable_if<!std::is_default_constructible<T>::value>::type*> 
std::shared_ptr<T> Context::resolve(Context& ctx) 
{
    auto ti = std::type_index(typeid(T));

    if(providers_.count(ti) == 0)
    {
        if(parent_)
        {
            return parent_->resolve<T>(ctx);
        }

        throw ContextEx();
    }

    auto& p = providers_[ti];
    auto a = p->create(ctx);

    return std::any_cast<std::shared_ptr<T>>(a);
}

//! \private
template<class T, typename std::enable_if<std::is_default_constructible<T>::value>::type*> 
std::shared_ptr<T> Context::resolve(Context& ctx) 
{
     auto ti = std::type_index(typeid(T));

     if(providers_.count(ti) == 0)
     {
         if(parent_)
         {
             return parent_->resolve<T>(ctx);
         }

         register_singleton<T()>();
     }

     auto& p = providers_[ti];
     auto a = p->create(ctx);

     return std::any_cast<std::shared_ptr<T>>(a);
}



//! helper shortcut to return a std::shared_ptr<T> from Context for type T
//! \ingroup api

template<class T>
std::shared_ptr<T> inject(Context& ctx)
{
	return ctx.resolve<T>();
}



//! singleton ctx registration helper
//!
//! registers a singleton
//! \ingroup api

template<class F, class I = typename returns<F>::type>
class singleton
{
public:

    //! use to register the singleton with a different base class
    //! used to register by interface instead of implementation class
    template<class P>
    using as = singleton<F,typename std::remove_reference<P>::type>;

    singleton()
    {}

    void ctx_register(Context* ctx)
    {
        ctx->register_singleton<F,I>();
    }
};


//! singleton ctx registration helper
//!
//! registers a singleton
//! \ingroup api

template<class F, class I = typename returns<F>::type>
class provider
{
public:

    template<class P>
    using as = provider<F,typename std::remove_reference<P>::type>;

    provider()
    {}

    void ctx_register(Context* ctx)
    {
    	ctx->register_factory<F,I>();
    }
	
};

//! value ctx registration helper
//!
//! registers an existing value
//! \ingroup api

template<class T, class I = T>
class value
{
public:

    template<class P>
    using as = value<T,typename std::remove_reference<P>::type>;

    value(std::shared_ptr<T> ptr)
        : ptr_(ptr)
    {}

    void ctx_register(Context* ctx)
    {
    	ctx->register_static<T,I>(ptr_);
    }

private:
    std::shared_ptr<T> ptr_;
};


//! Application context helper
//!
//! typical usage
//! \code{.cpp}
//! ApplicationContext ctx
//! {
//!     singleton<MyDependency1()>(),
//!     singleton<MyDependency2())>(),
//!     singleton<MyComponent(MyDependency1,MyDependency2)>(),
//!     provider<Session()>
//! }
//! \endcode
//! \ingroup api

class ApplicationContext : public Context
{
public:

	template<class ... Args>
	ApplicationContext(Args&& ... args)
		: Context(nullptr)
	{
		// make Context itself injectable
		std::shared_ptr<Context> ctx = std::shared_ptr<Context>(this, [](Context* c){});
		register_static<ApplicationContext,Context>(ctx);

		register_dependencies<Args&&...>(std::forward<Args&&>(args)...);
	}

private:

	template<class ... Args>
	void register_dependencies()
	{}

	template<class T, class ... Args>
	void register_dependencies(T&& t,Args&& ... args)
	{
		t.ctx_register(this);
		register_dependencies<Args&&...>(std::forward<Args&&>(args)...);
	}



	ApplicationContext(const ApplicationContext& rhs) = delete;
};

} // end namespace

#endif /* INCLUDE_PROMISE_WEB_CTX_H_ */
