#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "diycpp/ctx.h"
  
class ProviderTest : public ::testing::Test {
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
        std::cout << s << std::endl;
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

	std::shared_ptr<TestController> controller_;
};

//DIY_DEFINE_CONTEXT()


diy::provider<TestController(Logger)> TestControllerComponent;

diy::provider<MyApp(TestController)> MyAppComponent;


TEST_F(ProviderTest, ValueMaintainsOwnLifetime) 
{

    // define a shared_ptr to Logger elsewhere:
    auto theLogger = std::make_shared<Logger>();

    // add the shared_ptr to context:
    diy::value<Logger> loggerComponent(theLogger);

    diy::ApplicationContext ctx{
        loggerComponent,
        TestControllerComponent,
        MyAppComponent
    };
    
    // run test
    {
        auto myApp = diy::inject<MyApp>(ctx);
        myApp->run(42);

        // check results
        auto tc = myApp->controller_;
        auto l = tc->logger_;

        EXPECT_EQ(1,tc->invocation_count);
        EXPECT_EQ(1,l->invocation_count);
        EXPECT_STREQ("value:42",l->buffer.c_str());
    }
    // run test again
    {
        auto myApp = diy::inject<MyApp>(ctx);
        myApp->run(43);

        // check results
        auto tc = myApp->controller_;
        auto l = tc->logger_;

        EXPECT_EQ(1,tc->invocation_count);
        EXPECT_EQ(2,l->invocation_count);
        EXPECT_STREQ("value:42value:43",l->buffer.c_str());
    }
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}
