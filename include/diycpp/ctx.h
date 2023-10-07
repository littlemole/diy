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

class Context;

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


namespace detection {

	template <class Default, class AlwaysVoid,
		template<class...> class Op, class... Args>
	struct detector {
		using value_t = std::false_type;
		using type = Default;
	};

	template <class Default, template<class...> class Op, class... Args>
	struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
		// Note that std::void_t is a C++17 feature
		using value_t = std::true_type;
		using type = Op<Args...>;
	};

	struct nonesuch
	{
		nonesuch() = delete;
		~nonesuch() = delete;
		nonesuch(nonesuch const&) = delete;
		void operator=(nonesuch const&) = delete;
	};

	template <template<class...> class Op, class... Args>
	using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;


    template<class T>
    using has_creator_t = decltype(&T::create_instance);

    template<class T>
    using has_creator = is_detected<has_creator_t, T>;


    template <typename T>
    struct get_signature;

    template <typename R, typename... Args>
    struct get_signature<R(*)(Args...)> {
        using type = R(Args...);
        using r_type = R;
        using args_types = std::tuple< Args ...>;
    };

    template<class T>
    struct is_shared_ptr
    {
        constexpr static bool value = false;
    };

    template<class T>
    struct is_shared_ptr<std::shared_ptr<T>>
    {
        constexpr static bool value = true;
    };

    template<class T, size_t I = 0>
    constexpr bool args_are_shared_ptr()
    {
        constexpr size_t n = std::tuple_size<T>();
        if constexpr(I == n ) 
        {
            return true;
        }
        else
        {
            using t = typename std::tuple_element<I, T>::type;
            if constexpr (is_shared_ptr<t>::value)
            {
                return args_are_shared_ptr<T,I+1>();
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    template<class T, class IFace, class Signature, int I = 0, class ... Args>
    void deduce_singleton(Context& ctx);

} // end namespace detection


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

    //! reseolve a type T from context
    template<class T>
    std::shared_ptr<T> resolve();

    //! register Factory for T with T in form of F(Args...)
    template<class T, class I = typename returns<T>::type >
    void register_factory();

    //! register Singleton for T with T in form of F(Args...)
    template<class T, class I = typename returns<T>::type>
    void register_singleton();

    //! register exisitng value for T 
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

//! \private
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

//! \private
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

//! \private
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


//! \private
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
            auto ptr = parent_->resolve<T>(ctx);
            if(ptr) return ptr;
        }

        if constexpr( detection::has_creator<T>::value)
        {
            using Signature = detection::get_signature<decltype(&T::create_instance)>;
            constexpr bool isShared = detection::args_are_shared_ptr<typename Signature::args_types>();
            constexpr bool returnsShared = detection::is_shared_ptr<typename Signature::r_type>::value;

            if constexpr( isShared && returnsShared)
            {
                detection::deduce_singleton<T,typename Signature::r_type::element_type, typename Signature::args_types>(*this);

                auto& p = providers_[ti];
                auto a = p->create(ctx);

                return std::any_cast<std::shared_ptr<T>>(a);
            }
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

    //! \private
    void ctx_register(Context* ctx)
    {
        ctx->register_singleton<F,I>();
    }
};

//! singleton component ctx registration helper
//!
//! registers a singleton for a type with a static create_instance factory method
//! dependencies will be deduced from factory method signature
//! \ingroup api

template<class T>
class component
{
public:

    component()
    {}

    //! \private
    void ctx_register(Context* ctx)
    {
        using Signature = detection::get_signature<decltype(&T::create_instance)>;
        constexpr bool isShared = detection::args_are_shared_ptr<typename Signature::args_types>();
        constexpr bool returnsShared = detection::is_shared_ptr<typename Signature::r_type>::value;

        if constexpr( isShared && returnsShared)
        {
            detection::deduce_singleton<T,typename Signature::r_type::element_type, typename Signature::args_types>(*ctx);
        }
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

    //! \private
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

    //! \private
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
//! };
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


namespace detection {

    template<class T, class IFace, class Signature, int I = 0, class ... Args>
    void deduce_singleton(Context& ctx)
    {
        constexpr size_t n = std::tuple_size<Signature>();
        if constexpr(I == n ) 
        {
            ctx.register_singleton<T(Args...), IFace>();
        }
        else
        {
            using t = typename std::tuple_element<I, Signature>::type::element_type;
            deduce_singleton<T,IFace,Signature,I+1,Args...,t>(ctx);
        }
    }

} // end namespace detection


} // end namespace

#endif /* INCLUDE_PROMISE_WEB_CTX_H_ */
