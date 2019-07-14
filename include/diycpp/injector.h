#ifndef INCLUDE_PROMISE_WEB_CTX_INJECTOR_H_
#define INCLUDE_PROMISE_WEB_CTX_INJECTOR_H_

//! \file injector.h
#include "diycpp/ctx.h"
 
namespace diy  {

//! \private

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

//! \private

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

//! Module level Injector
class Injector
{
friend class inject_modules;

public:

	//! \private
	typedef Injector type;

	//! \private
	Injector(const Injector& i)
	{
		registrator_ = i.registrator_;
		injectors().insert(this);
	}

	//! \private
	Injector(Injector&& i)
	{
		registrator_ = i.registrator_;
		injectors().insert(this);
	}

	//! constrcut Injector by passing Injection candidates.
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

	//! \private

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

//! pass inject_modules in your Application Context constructor
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
