#ifndef INCLUDE_PROMISE_WEB_CTX_INVOKE_H_
#define INCLUDE_PROMISE_WEB_CTX_INVOKE_H_

#include "diycpp/ctx.h"
 
namespace diy  {

	template<class T>
	struct Invoker
	{
		typedef T type;
	};


	template<class T>
	struct Invoker<std::shared_ptr<T>>
	{
		typedef T type;

		static std::shared_ptr<T> inject(diy::Context& ctx)
		{
			return diy::inject<type>(ctx);
		}
	};

	template<class T>
	struct Invoker<T&>
	{
		typedef T type;

		static T& inject(diy::Context& ctx)
		{
			return *(diy::inject<type>(ctx));
		}
	};

	template<class T>
	class Caller;

	template<class T>
	class ObjCaller;

	template<class F, class T>
	class Caller<void(F, T)>
	{
	public:

		template<class ... VArgs>
		static void invoke(diy::Context& ctx, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename RemoveSharedPtr<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			fun(vargs..., t);
		}
	};


	template<class O, class F, class T>
	class ObjCaller<void(O, F, T)>
	{
	public:

		template<class ... VArgs>
		static void invoke(diy::Context& ctx, O& o, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			(o.*fun)(vargs..., t);
		}
	};


	template<class R, class F, class T>
	class Caller<R(F, T)>
	{
	public:

		template<class ... VArgs>
		static R invoke(diy::Context& ctx, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			return fun(vargs..., t);
		}
	};


	template<class R, class O, class F, class T>
	class ObjCaller<R(O, F, T)>
	{
	public:

		template<class ... VArgs>
		static R invoke(diy::Context& ctx, O& o, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			return (o.*fun)(vargs..., t);
		}
	};


	template<class F, class T, class ... Args>
	class Caller<void(F, T, Args...)>
	{
	public:

		template<class ... VArgs>
		static void invoke(diy::Context& ctx, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			Caller<void(F, Args...)>::invoke(ctx, fun, vargs..., t);
		}
	};


	template<class O, class F, class T, class ... Args>
	class ObjCaller<void(O, F, T, Args...)>
	{
	public:

		template<class ... VArgs>
		static void invoke(diy::Context& ctx, O& o, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			ObjCaller<void(O, F, Args...)>::invoke(ctx, o, fun, vargs..., t);
		}
	};

	template<class R, class F, class T, class ... Args>
	class Caller<R(F, T, Args...)>
	{
	public:

		template<class ... VArgs>
		static R invoke(diy::Context& ctx, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			//auto t = Invoker<T>::inject(ctx);
			auto t = Invoker<T>::inject(ctx);
			return Caller<R(F, Args...)>::invoke(ctx, fun, vargs..., t);
		}
	};


	template<class R, class O, class F, class T, class ... Args>
	class ObjCaller<R(O, F, T, Args...)>
	{
	public:

		template<class ... VArgs>
		static R invoke(diy::Context& ctx, O& o, F fun, VArgs ... vargs)
		{
			//auto t = diy::inject<typename Invoker<T>::type>(ctx);
			auto t = Invoker<T>::inject(ctx);
			return ObjCaller<R(O, F, Args...)>::invoke(ctx, o, fun, vargs..., t);
		}
	};


	template<class ... Args>
	void call(diy::Context& ctx, void(*fun)(Args...))
	{
		Caller<void(decltype(fun), Args...)>::invoke(ctx, fun);
	}

	template<class R, class ... Args>
	auto call(diy::Context& ctx, R(*fun)(Args...)) -> R
	{
		return Caller<R(decltype(fun), Args...)>::invoke(ctx, fun);
	}


	template<class O, class ... Args>
	void call(diy::Context& ctx, O& o, void(O::*fun)(Args...))
	{
		ObjCaller<void(O, decltype(fun), Args...)>::invoke(ctx, o, fun);
	}


	template<class R, class O, class ... Args>
	auto call(diy::Context& ctx, O& o, R(O::*fun)(Args...)) -> R
	{
		return ObjCaller<R(O, decltype(fun), Args...)>::invoke(ctx, o, fun);
	}


} // end namespace

#endif /* INCLUDE_PROMISE_WEB_CTX_INVOKE_H_ */
