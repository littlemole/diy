#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "diycpp/invoke.h"

class StaticTest : public ::testing::Test {
protected:

	static void SetUpTestCase() {

	}

	virtual void SetUp() {
	}

	virtual void TearDown() {
	}
}; // end test setup




class Logger
{
public:

	int invocation_count = 0;
	std::string buffer;

	void log(const std::string& s)
	{
		buffer.append(s);
		invocation_count++;
	}
};


class TestController
{
public:

	int invocation_count = 0;

	TestController(std::shared_ptr<Logger> logger)
		: logger_(logger)
	{}

	void handler(int val)
	{
		std::ostringstream oss;
		oss << "value:" << val;
		logger_->log(oss.str());
		invocation_count++;
	}

    static std::shared_ptr<TestController> create_instance(std::shared_ptr<Logger> logger)
    {
        return std::make_shared<TestController>(logger);
    }

  
private:

	std::shared_ptr<Logger> logger_;
};



class MyApp
{
public:
	MyApp(std::shared_ptr<TestController> tc)
		: controller_(tc)
	{}

	void run(int value)
	{
		controller_->handler(value);
	}

    static std::shared_ptr<MyApp> create_instance(std::shared_ptr<TestController> controller)
    {
        return std::make_shared<MyApp>(controller);
    }

private:
	std::shared_ptr<TestController> controller_;
};


TEST_F(StaticTest, SimpleDI)
{
	diy::ApplicationContext ctx{
//		LoggerComponent,
//		TestControllerComponent,
//		MyAppComponent
	};

	{
		auto myApp = diy::inject<MyApp>(ctx);
		myApp->run(42);
	}
	{
		auto myApp = diy::inject<MyApp>(ctx);
		myApp->run(43);
	}

	// assert results
	auto tc = diy::inject<TestController>(ctx);
    
	auto l = diy::inject<Logger>(ctx);

	EXPECT_EQ(2, tc->invocation_count);
	EXPECT_EQ(2, l->invocation_count);
	EXPECT_STREQ("value:42value:43", l->buffer.c_str());
}

class DependencyA
{
public:
	DependencyA() : x("its an A!") {}
	std::string x;
};

class DependencyB
{
public:
	DependencyB() : x(42) {}
	int x;
};

void testFunc(std::shared_ptr<DependencyA> a, std::shared_ptr<DependencyB> b) {
	std::cout << a->x << " with " << b->x << std::endl;
}

std::string testFunc2(std::shared_ptr<DependencyA> a, std::shared_ptr<DependencyB> b) {
	std::ostringstream oss;
	oss << "with value: " << a->x << " with " << b->x << std::endl;
	return oss.str();
}


std::string testFunc3(DependencyA& a, DependencyB& b) {
	std::ostringstream oss;
	oss << "with refs: " << a.x << " with " << b.x << std::endl;
	return oss.str();
}


class TestObj
{
public:

	std::string testFunc(std::shared_ptr<DependencyA> a, std::shared_ptr<DependencyB> b)
	{
		std::ostringstream oss;
		oss << "with value: " << a->x << " with " << b->x << std::endl;
		return oss.str();
	}

	void testFunc2(std::shared_ptr<DependencyA> a, std::shared_ptr<DependencyB> b)
	{
		std::cout << "with void: " << a->x << " with " << b->x << std::endl;
	}

	void testFunc3(DependencyA& a, DependencyB& b)
	{
		std::cout << "with ref: " << a.x << " with " << b.x << std::endl;
	}
};


TEST_F(StaticTest, SimpleInvoke)
{
	diy::ApplicationContext ctx{
//		diy::singleton<DependencyA()>(),
//		diy::singleton<DependencyB()>()
	};

	call(ctx, testFunc);

	std::string s = call(ctx, testFunc2);

	std::cout << s;
}



TEST_F(StaticTest, ObjSimpleInvoke)
{
	diy::ApplicationContext ctx{
//		diy::singleton<DependencyA()>(),
//		diy::singleton<DependencyB()>()
	};

	TestObj obj;

	std::string s = call(ctx, obj, &TestObj::testFunc);

	std::cout << s;

	call(ctx, obj, &TestObj::testFunc2);

}

TEST_F(StaticTest, SimpleInvokeRef)
{
	diy::ApplicationContext ctx{
//		diy::singleton<DependencyA()>(),
//		diy::singleton<DependencyB()>()
	};

	std::string s = call(ctx, testFunc3);

	std::cout << s;
}

TEST_F(StaticTest, ObjSimpleInvokeRef)
{
	diy::ApplicationContext ctx{
//		diy::singleton<DependencyA()>(),
//		diy::singleton<DependencyB()>()
	};

	TestObj obj;


	call(ctx, obj, &TestObj::testFunc3);

}


TEST_F(StaticTest, ObjSimpleAutoFactory)
{
	diy::ApplicationContext ctx{};

	auto logger = diy::inject<Logger>(ctx);

}



int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int r = RUN_ALL_TESTS();

	return r;
}

