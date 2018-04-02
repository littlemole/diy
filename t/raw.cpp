#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "diycpp/ctx.h"
  
class RawTest : public ::testing::Test {
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

	TestController() {}

	TestController(std::shared_ptr<Logger> logger)
	: logger_(logger)
	{}

	void handler( int val)
	{
		std::ostringstream oss;
		oss << "value:" << val;
		logger_->log(oss.str());
		invocation_count++;
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

	void run( int value )
	{
		controller_->handler(value);
	}

private:
	std::shared_ptr<TestController> controller_;
};

//DIY_DEFINE_CONTEXT()



TEST_F(RawTest, rawApiTest) 
{
	// low level context setup interface

	diy::ApplicationContext ctx;

    ctx.registerFactory<Logger()>(
        new diy::FactoryImpl<Logger>(
			new diy::ConstructorImpl<Logger()>()
		)
    );

    ctx.registerFactory<TestController(Logger)>();

    ctx.registerFactory<MyApp(TestController)>();

	// use context after setup

	auto myApp = diy::inject<MyApp>(ctx);
	myApp->run(42);

	auto tc = diy::inject<TestController>(ctx);
	auto l = diy::inject<Logger>(ctx);

	EXPECT_EQ(1,tc->invocation_count);
	EXPECT_EQ(1,l->invocation_count);
	EXPECT_STREQ("value:42",l->buffer.c_str());
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}
