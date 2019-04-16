#ifndef INCLUDE_PROMISE_WEB_CTX_INJECTOR_H_
#define INCLUDE_PROMISE_WEB_CTX_INJECTOR_H_

#include "diycpp/ctx.h"
 
namespace diy  {

template<int I>
struct reg
{
	template<class P>
	static void register_ctx( P& p,diy::Context* ctx)
	{
		reg<I-1>::register_ctx(p,ctx);

		auto& t = std::get<I>(p);
		t.ctx_register(ctx);
	}
};

template<>
struct reg<0>
{
	template<class P>
	static void register_ctx( P& p,diy::Context* ctx)
	{			
		auto& t = std::get<0>(p);
		t.ctx_register(ctx);
	}
};


class Injector
{
friend class inject_modules;

public:

	typedef Injector type;

	Injector(const Injector& i)
	{
		registrator_ = i.registrator_;
		injectors().insert(this);
	}

	Injector(Injector&& i)
	{
		registrator_ = i.registrator_;
		injectors().insert(this);
	}

	template<class ... Args>
	Injector( Args&& ... args)
	{
		auto t = std::make_tuple(std::forward<Args>(args)...);

		registrator_ = [t](diy::Context* ctx) mutable
		{
			reg<std::tuple_size<decltype(t)>::value-1>::register_ctx(t,ctx);
		};
		injectors().insert(this);
	}

	~Injector()
	{
		injectors().erase(this);
	}

	void ctx_register(diy::Context* ctx)
	{
		registrator_(ctx);
	}

private:


	std::function<void(diy::Context* ctx)> registrator_;

	static std::set<Injector*>& injectors()
	{
		static std::set<Injector*> i;
		return i;
	};
};


struct inject_modules
{
	void ctx_register(diy::Context* ctx)
	{
		for( auto i : Injector::injectors())
		{
			i->ctx_register(ctx);
		}
	}
};


}


#endif
