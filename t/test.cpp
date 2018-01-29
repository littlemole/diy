#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "diycpp/ctx.h"
  
class BasicTest : public ::testing::Test {
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

diy::singleton<Logger()> LoggerComponent;


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


diy::singleton<TestController(Logger)> TestControllerComponent(
	diy::constructor<TestController(Logger)>()
);

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

DIY_DEFINE_CONTEXT()


diy::singleton<MyApp(TestController)> MyAppComponent;

TEST_F(BasicTest, SimpleDI) 
{
	{
		auto myApp = diy::inject<MyApp>();
		myApp->run(42);
	}
	{
		auto myApp = diy::inject<MyApp>();
		myApp->run(43);
	}

	// assert results

	auto tc = diy::inject<TestController>();
	auto l = diy::inject<Logger>();

	EXPECT_EQ(2,tc->invocation_count);
	EXPECT_EQ(2,l->invocation_count);
	EXPECT_STREQ("value:42value:43",l->buffer.c_str());
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}
