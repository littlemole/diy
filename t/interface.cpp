#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "diycpp/invoke.h"

class InterfaceTest : public ::testing::Test {
protected:

	static void SetUpTestCase() {

	}

	virtual void SetUp() {
	}

	virtual void TearDown() {
	}
}; // end test setup

class ILogger
{
public:
    virtual ~ILogger() {}

    virtual void log(const std::string& s) = 0;
    virtual int ic() = 0;
    virtual std::string& buf() = 0;
};


class Logger : public ILogger
{
public:

	int invocation_count = 0;
	std::string buffer;

    int ic()
    {
        return invocation_count;
    }

	void log(const std::string& s)
	{
		buffer.append(s);
		invocation_count++;
	}

    std::string& buf()
    {
        return buffer;
    }
};

diy::singleton<Logger()>::as<ILogger> LoggerComponent;



class IController
{
public:

	virtual ~IController() {}

	virtual void handler(int val) = 0;
    virtual int ic() = 0;
};


class TestController : public IController
{
public:

	int invocation_count = 0;

	TestController() {}

	TestController(std::shared_ptr<ILogger> logger)
		: logger_(logger)
	{}

	void handler(int val)
	{
		std::ostringstream oss;
		oss << "value:" << val;
		logger_->log(oss.str());
		invocation_count++;
	}

    int ic()
    {
        return invocation_count;
    }

private:

	std::shared_ptr<ILogger> logger_;
};



diy::singleton<TestController(ILogger&)>::as<IController> TestControllerComponent;

class MyApp
{
public:
	MyApp(std::shared_ptr<IController> tc)
		: controller_(tc)
	{}

	void run(int value)
	{
		controller_->handler(value);
	}

private:
	std::shared_ptr<IController> controller_;
};

//DIY_DEFINE_CONTEXT()


diy::singleton<MyApp(IController&)> MyAppComponent;

TEST_F(InterfaceTest, InterfaceBasedDI)
{
	diy::ApplicationContext ctx{
		LoggerComponent,
		TestControllerComponent,
		MyAppComponent
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

	auto tc = diy::inject<IController>(ctx);
	auto l = diy::inject<ILogger>(ctx);

	EXPECT_EQ(2, tc->ic());
	EXPECT_EQ(2, l->ic());
	EXPECT_STREQ("value:42value:43", l->buf().c_str());
}


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int r = RUN_ALL_TESTS();

	return r;
}

