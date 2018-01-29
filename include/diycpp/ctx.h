#ifndef INCLUDE_PROMISE_WEB_CTX_H_
#define INCLUDE_PROMISE_WEB_CTX_H_

#include <type_traits>
#include <typeindex>
#include <exception>
#include <map>
 
namespace diy  {


// return value of callable

template<class T>
class returns
{};

template<class T>
class returns<T()>
{
public:
	typedef T type;
};


template<class T>
class returns<T(*)()>
{
public:
	typedef T type;
};

template<class T, class ... Args>
class returns<T(Args...)>
{
public:
	typedef T type;
};


template<class T, class ... Args>
class returns<T(*)(Args...)>
{
public:
	typedef T type;
};
 

class Context;

class ContextEx : public std::exception 
{
public:
	ContextEx()
	{}
};

Context& context();

// Constructor API

template<class T>
class Constructor
{
public:
    virtual ~Constructor() {}
    virtual T* create(Context& ctx) = 0;
};


// Factory API

class Factory
{
public:
    virtual ~Factory() {}

};


template<class T>
class FactoryImpl : public Factory
{
public:

	FactoryImpl( Constructor<T>* ctor  )
        : ctor_(ctor)
    {}

	FactoryImpl( std::shared_ptr<T> t)
        : ptr_(t)
    {}


	FactoryImpl()
    {}

    virtual std::shared_ptr<T> resolve(Context& ctx)
    {
        if ( !ptr_.get() )
        {
            ptr_.reset( ctor_->create(ctx) );
        }
        return ptr_;
    }

protected:
    std::shared_ptr<T> ptr_;
    std::unique_ptr<Constructor<T>> ctor_;
};

// Provider impl


template<class T>
class Provider : public FactoryImpl<T>
{
public:

    Provider( Constructor<T>* ctor  )
        : FactoryImpl<T>(ctor)
    {}

    std::shared_ptr<T> resolve(Context& ctx)
    {
        return std::shared_ptr<T>(FactoryImpl<T>::ctor_->create(ctx));
    }
};


// Context API

/**
 * Context implements the main IOC context.
 *
 */

class Context
{
public:

    /**
     * construct a new IOC Context.
     */
    Context() : parentCtx_(&context())
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


    // resolve instance from context
    template<class T>
    std::shared_ptr<T> resolve()
    {
        return resolve<T>( std::type_index(typeid(T)),*this );
    }

    template<class T>
    std::shared_ptr<T> resolve(Context& ctx)
    {
        return resolve<T>(std::type_index(typeid(T)), ctx );
    }

	void registerFactory(const std::type_index& idx, Factory* f)
	{
		theMap_[idx] = std::unique_ptr<Factory>(f);
	}

	void clear()
	{
		theMap_.clear();
	}

private:

	template<class T>
	std::shared_ptr<T> resolve(const std::type_index& idx, Context& ctx);

    Context( const Context& rhs ) : parentCtx_(0) {}

    Context& operator=(const Context& rhs)
    {
        return *this;
    }

    std::map<std::type_index,std::shared_ptr<Factory>> theMap_;
    Context* parentCtx_;
};


template<class T>
std::shared_ptr<T> Context::resolve( const std::type_index& idx, Context& ctx)
{
    // delegate to parent ctx if not avail
    if( theMap_.count(idx) == 0 )
    {
        if ( parentCtx_ != 0 )
        {
            return parentCtx_->resolve<T>(idx,ctx);
        }
        throw ContextEx();
    }

    // lookup entity factory and resolve instance
    return dynamic_cast<FactoryImpl<T>*>(
        theMap_[idx].get()
    )
    ->resolve(ctx);
}


Context& context();


template<class T>
std::shared_ptr<T> inject()
{
    return context().resolve<T>();
}

template<class T>
std::shared_ptr<T> inject(Context& ctx)
{
	return ctx.resolve<T>();
}

template<class T>
std::shared_ptr<T> inject( const std::type_index& idx)
{
    return context().resolve<T>(idx);
}

template<class T>
std::shared_ptr<T> inject(const std::type_index& idx, Context& ctx)
{
	return ctx.resolve<T>(idx);
}


template<class T>
class Creator
{
};

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

template<class T, class P, class ... Args>
class Creator<T(P, Args...)>
{
public:

	template<class ... VArgs>
	static T* create(Context& ctx, VArgs&& ... args)
	{
		std::shared_ptr<P> p = ctx.resolve<P>();
		return Creator<T(Args...)>::create(ctx, std::forward<VArgs>(args)..., p);
	}
};

/*
-- considered evil:

template<class T, class P, class ... Args>
class Creator<T(P&, Args...)>
{
public:

	template<class ... VArgs>
	static T* create(Context& ctx, VArgs&& ... args)
	{
		P* p = ctx.resolve<P>().get();
		return Creator<T(Args...)>::create(ctx, std::forward<VArgs>(args)..., *p);
	}
};
*/

// Constructor Impl

template<class T>
class ConstructorImpl
{
};

template<class T>
class ConstructorImpl<T()> : public Constructor<T>
{
public:

	virtual ~ConstructorImpl() {}

    virtual T* create(Context& ctx)
    {
        return new T;
    }
};



template<class T, class P, class ... Args>
class ConstructorImpl<T(P,Args...)>  : public Constructor<T>
{
public:

    ConstructorImpl() {}

	virtual ~ConstructorImpl() {}

    virtual T* create(Context& ctx)
    {
        return Creator<T(P,Args...)>::create(ctx);
    }
};



template<class T>
ConstructorImpl<T>* constructor()
{
    return new ConstructorImpl<T>();
}

template<class I, class T>
Constructor<I>* constructor(  )
{
    return (Constructor<I>*) (new ConstructorImpl<T>());
}

// ctx register helpers

template<class T>
class singleton
{
public:
	typedef typename returns<T>::type type;

    singleton()
		: singleton(context())
    {}

	template<class I>
	singleton(Constructor<I>* ctor)
		: singleton(ctor, context())
	{}

	singleton(Context& ctx)
		: singleton(new ConstructorImpl<T>,ctx)
	{}

	template<class I>
	singleton(Constructor<I>* ctor, Context& ctx)
	{
		ctx.registerFactory(
			std::type_index(typeid(I)),
			new FactoryImpl<type>(ctor)
		);
	}
};


template<class T>
class provider
{
public:

	typedef typename returns<T>::type type;

	provider()
		: provider(context())
	{}

	template<class I>
	provider(Constructor<I>* ctor)
		: provider(ctor,context())
	{}

	provider(Context& ctx)
    {
		ctx.registerFactory(
			std::type_index(typeid(type)),
			new Provider<type>(
				new ConstructorImpl<T>
			)
		);
    }

	template<class I>
	provider(Constructor<I>* ctor, Context& ctx)
	{
		ctx.registerFactory(
			std::type_index(typeid(I)),
			new Provider<type>(ctor)
		);
	}
};


template<class T>
class ctx_value
{
public:

	typedef T type;
	typedef std::shared_ptr<T> Ptr;

	ctx_value(T* t)
		: ctx_value(t,context())
	{}

	ctx_value(Ptr t)
		: ctx_value(t, context())
	{}

	ctx_value(T* t, Context& ctx )
    {
		register_value(ctx,std::type_index(typeid(type)),Ptr(t));
    }

	ctx_value(Ptr t, Context& ctx )
	{
		register_value(ctx,std::type_index(typeid(type)),t);
    }

private:

    void register_value(Context& ctx, const std::type_index& idx, Ptr ptr)
    {
    	ctx.registerFactory(
    		idx,
			new FactoryImpl<type>(ptr)
		);
    }
};


// syntactic sugar

class Application
{
public:

	template<class ... Args>
	Application(Args&& ... args)
	{}

private:

	Application(const Application& rhs) = delete;
};

typedef Application components;


}

#define DIY_DEFINE_CONTEXT() \
namespace diy {            \
                           \
Context& context()         \
{                          \
    static Context ctx(0); \
    return ctx;            \
}                          \
}


#endif /* INCLUDE_PROMISE_WEB_CTX_H_ */
