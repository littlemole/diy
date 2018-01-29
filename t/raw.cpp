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

DIY_DEFINE_CONTEXT()

/*
diy::singleton<Logger()> LoggerComponent;

diy::singleton<TestController(Logger)> TestControllerComponent(
	diy::constructor<TestController(Logger)>()
);

diy::singleton<MyApp(TestController)> MyAppComponent;
*/

TEST_F(BasicTest, SimpleDI) 
{
    diy::context().registerFactory(
        std::type_index(typeid(Logger)),
        new diy::FactoryImpl<Logger>(new diy::ConstructorImpl<Logger()>())
    );

    diy::context().registerFactory(
        std::type_index(typeid(TestController)),
        new diy::FactoryImpl<TestController>(new diy::ConstructorImpl<TestController(Logger)>())
    );

    diy::context().registerFactory(
        std::type_index(typeid(MyApp)),
        new diy::FactoryImpl<MyApp>(new diy::ConstructorImpl<MyApp(TestController)>())
    );

	auto myApp = diy::inject<MyApp>();
	myApp->run(42);

	auto tc = diy::inject<TestController>();
	auto l = diy::inject<Logger>();

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
