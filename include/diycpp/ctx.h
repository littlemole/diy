#ifndef INCLUDE_PROMISE_WEB_CTX_H_
#define INCLUDE_PROMISE_WEB_CTX_H_

//! \file ctx.h

#include <type_traits>
#include <typeindex>
#include <exception>
#include <map>
#include <memory>
 
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
 

class Context;

//! \brief Exception thrown if context resolution fails
class ContextEx : public std::exception 
{
public:
	ContextEx()
	{}
};

//! Constructor creates new objects

class Constructor
{
public:
    virtual ~Constructor() {}
    virtual void* create(Context& ctx) = 0;
};


// Factory API
//! \private
class Factory
{
public:
    virtual ~Factory() {}

};


//! \brief
//!
//! FactoryImpl returns context wide singletons

template<class T>
class FactoryImpl : public Factory
{
public:

	//! construct with an explicit Constructor
	FactoryImpl( Constructor* ctor  )
        : ctor_(ctor)
    {}

    	//! construct from existing shared_ptr
	FactoryImpl( std::shared_ptr<T> t)
        : ptr_(t)
    {}


    	//! default constructor for default constructible T
	FactoryImpl()
    {}


    //! resolve T from context and return std::shared_ptr<T>
    virtual std::shared_ptr<T> resolve(Context& ctx)
    {
        if ( !ptr_.get() )
        {
            ptr_.reset( (T*)(ctor_->create(ctx)) );
        }
        return ptr_;
    }

protected:
    std::shared_ptr<T> ptr_;
    std::unique_ptr<Constructor> ctor_;
};


//! \brief Provider 
//!
//! everytime a Provider resolves a T from context, a new shared_ptr<T> will be constructed and returned

template<class T>
class Provider : public FactoryImpl<T>
{
public:

    //! construct Provider with explicit Constructor
    Provider( Constructor* ctor  )
        : FactoryImpl<T>(ctor)
    {}

    //! resolve T from context and return a newly constructed std::shared_ptr<T>
    std::shared_ptr<T> resolve(Context& ctx)
    {
        return std::shared_ptr<T>((T*)(FactoryImpl<T>::ctor_->create(ctx)));
    }
};



/** \brief Context API
 *
 * Context implements the main IOC context.
 *
 */

class Context
{
public:

    /**
     * construct a new IOC Context.
     */
    Context() : parentCtx_(0)
    {
    }

    /**
     * construct a new IOC Context inheriting from parent context ctx.
     */

    Context(Context* ctx) : parentCtx_(ctx)
    {
    }

    ~Context()
    {
    }

    //! resolve instance of type T from default context, returning a std::shared_ptr<T>
    template<class T>
    std::shared_ptr<T> resolve()
    {
        return resolve<T>( std::type_index(typeid(T)),*this );
    }

    //! resolve instance of type T from given context, returning a std::shared_ptr<T>
    template<class T>
    std::shared_ptr<T> resolve(Context& ctx)
    {
        return resolve<T>(std::type_index(typeid(T)), ctx );
    }

    //! register a factory that can resolve types of given type index using Factory f
    void registerFactory(const std::type_index& idx, Factory* f)
    {
	theMap_[idx] = std::unique_ptr<Factory>(f);
    }


    //! register a factory that can resolve types of type T using Factory f
    template<class F>
    void registerFactory(Factory* f)
    {
	theMap_[std::type_index(typeid(typename returns<F>::type))] = std::unique_ptr<Factory>(f);
    }


    //! register a Factory that can resolve type T with F = T(Args...)
    template<class F>
    void registerFactory();

    //! clear the context
    void clear()
    {
	theMap_.clear();
    }

protected:

    template<class T>
    std::shared_ptr<T> resolve(const std::type_index& idx, Context& ctx, typename std::enable_if<std::is_default_constructible<T>::value>::type* = nullptr);

    template<class T>
    std::shared_ptr<T> resolve(const std::type_index& idx, Context& ctx, typename std::enable_if<!std::is_default_constructible<T>::value>::type* = nullptr);

    Context( const Context& rhs ) : parentCtx_(0) {}

    Context& operator=(const Context& rhs)
    {
        return *this;
    }

    std::map<std::type_index,std::shared_ptr<Factory>> theMap_;
    Context* parentCtx_;
};


template<class T>
std::shared_ptr<T> Context::resolve( const std::type_index& idx, Context& ctx,typename std::enable_if<std::is_default_constructible<T>::value>::type*)
{
	std::cout << "resolve " << typeid(T).name() << std::endl;
    // delegate to parent ctx if not avail
    if( theMap_.count(idx) == 0 )
    {
        if ( parentCtx_ != 0 )
        {
            return parentCtx_->resolve<T>(idx,ctx);
        }

		this->registerFactory<T()>();
    }

    // lookup entity factory and resolve instance
    return dynamic_cast<FactoryImpl<T>*>(
        theMap_[idx].get()
    )
    ->resolve(ctx);
}


template<class T>
std::shared_ptr<T> Context::resolve( const std::type_index& idx, Context& ctx,typename std::enable_if<!std::is_default_constructible<T>::value>::type*)
{
	std::cout << "resolve " << typeid(T).name() << std::endl;

    // delegate to parent ctx if not avail
    if( theMap_.count(idx) == 0 )
    {
        if ( parentCtx_ != 0 )
        {
            return parentCtx_->resolve<T>(idx,ctx);
        }
		
		std::cout << "error resolving " << typeid(T).name() << std::endl;
        throw ContextEx();
    }

    // lookup entity factory and resolve instance
    return dynamic_cast<FactoryImpl<T>*>(
        theMap_[idx].get()
    )
    ->resolve(ctx);
}

//! helper shortcut to return a std::shared_ptr<T> from Context for type T
template<class T>
std::shared_ptr<T> inject(Context& ctx)
{
	return ctx.resolve<T>();
}


//! helper shortcut to return a std::shared_ptr<T> from Context for type idnex idx
template<class T>
std::shared_ptr<T> inject(const std::type_index& idx, Context& ctx)
{
	return ctx.resolve<T>(idx);
}


//! \private
template<class T>
class Creator
{
};


//! \private
template<class T>
class Creator<T()> 
{
public:

	template<class ... Args>
	static T* create(Context& ctx, Args&& ... args)
	{
		return new T(std::forward<Args>(args)...);
	}
};


//! \private
template<class T, class P, class ... Args>
class Creator<T(P, Args...)>
{
public:

	template<class ... VArgs>
	static T* create(Context& ctx, VArgs&& ... args)
	{
		typedef typename std::remove_reference<P>::type PT;
		std::shared_ptr<PT> p = ctx.resolve<PT>();
		return Creator<T(Args...)>::create(ctx, std::forward<VArgs>(args)..., p);
	}
};

//! Constructor Implementation
//! \private
template<class T>
class ConstructorImpl
{
};

//! Constructor Implementation for a default constructible T with no dependencies
template<class T>
class ConstructorImpl<T()> : public Constructor
{
public:

    virtual ~ConstructorImpl() {}

    virtual void* create(Context& ctx)
    {
        return new T;
    }
};


//! Constructor Implementation for a T with dependencies
template<class T, class P, class ... Args>
class ConstructorImpl<T(P,Args...)>  : public Constructor
{
public:

    ConstructorImpl() {}

    virtual ~ConstructorImpl() {}

    virtual void* create(Context& ctx)
    {
        return Creator<T(P,Args...)>::create(ctx);
    }
};


template<class F>
void Context::registerFactory()
{
	registerFactory<F>(
		new diy::FactoryImpl<typename returns<F>::type>(
			new diy::ConstructorImpl<F>()
		)
	);
}

//! helper to construct a Constructor from F = T(Args...)
template<class F>
ConstructorImpl<F>* constructor()
{
    return new ConstructorImpl<F>();
}


//! singleton ctx registration helper
//!
//! registers a singleton

template<class F, class I = typename returns<F>::type>
class singleton
{
public:

    //! use to register the singleton with a different base class
    //! used to register by interface instead of implementation class
    template<class P>
    using as = singleton<F,typename std::remove_reference<P>::type>;

    singleton()
	: ti_(typeid(I))
    {}

    singleton(std::shared_ptr<I> p)
	: ti_(typeid(I)), ptr_(p)
    {}

    void ctx_register(Context* ctx)
    {
	if(ptr_)
	{
	    ctx->registerFactory( ti_, new FactoryImpl<I>( ptr_ ));
	}
	else
	{
	    ctx->registerFactory( ti_, new FactoryImpl<I>(new ConstructorImpl<F>) );
	}
    }

private:	
    std::type_index ti_;
    std::shared_ptr<I> ptr_;
};


//! singleton ctx registration helper
//!
//! registers a singleton

template<class F, class I = typename returns<F>::type>
class provider
{
public:

    template<class P>
    using as = provider<F,typename std::remove_reference<P>::type>;

    provider()
	: ti_(typeid(I))
    {}

    void ctx_register(Context* ctx)
    {
	ctx->registerFactory( ti_, new Provider<I>(new ConstructorImpl<F>) );
    }
	
private:	
    std::type_index ti_;
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
class ApplicationContext : public Context
{
public:

	template<class ... Args>
	ApplicationContext(Args&& ... args)
		: Context(nullptr)
	{
		// make Context itself injectable
		std::shared_ptr<Context> ctx = std::shared_ptr<Context>(this, [](Context* c){});
		registerFactory(std::type_index(typeid(Context)), new FactoryImpl<Context>(ctx));

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
