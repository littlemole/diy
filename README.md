# diy
do it yourself dependency injection for cpp

# dependencies
c++14

# install

# basics

all dependencies are managed as std::shared_ptr&lt;T&gt;. only constructor dependency resolution is supported, a constructor
with all dependencies as std::shared_ptr&lt;P&gt; has to be provided. Components with no dependencies can use a default constructor.

dependency injection is otherwise non-invasive, that is components are plain std=c++ objects.

```cpp
    class TestComponent
    {
        public:
            TestComponent( std::shared_ptr<Dependency> dep)
                : dep_(dep)
            {}

            ... whatever methods using dep_ ...

        private:
            std::shared_ptr<Dependency> dep_;
    }
```

# usage

1. define app context

```cpp
    DIY_DEFINE_CONTEXT()
```

2. register components

components can be registered as static globals,
within main or even in some init() function called from main.

### as singleton

```cpp
        // declare as global statics, in main or in some init() function called from main
        diy::singleton<TestComponent(Dependency)> testComponent;
        diy::singleton<Dependency()> dependencyComponent;
```
### as provider

```cpp
        // provider will hand out a new obj for every injection
        diy::provider<TestComponent(Dependency)> testComponent;
```
### as value
```cpp
        // use any std::shared_ptr<T> and register with context
        auto tc = std::make_shared<TestComponent>();
        diy::ctx_value<TestComponent> testComponent(&tc);
```        
3. bootstrap application context and enter the matrix
```cpp
    auto tc = inject<TestComponent>();
    tc->some_method_using_dependencies();
```

# sugar to build application context
```cpp
    using namespace diy;

    Application depends {
        singleton<TestComponent(Dependency)>(),
        singleton<Dependency()>()
    }
```

# context inheritance and the default context

while the api provides convienience shortcuts that use context() as
default context, everything can also be called on a specific context.

```cpp
    using namespace diy;

    Context myContext(0);
    singleton<TestComponent(Dependency)> singletonComponent(myContext) ;
    singleton<Dependency()> testComponent(myContext) ;

    auto tc = inject<TestComponent>(myContext);
    tc->some_method_using_dependencies();
```
contexts can inherit, for example a local context inheriting from default context:

```cpp
    using namespace diy;

    // dependency declared in default context
    singleton<Dependency()>() singletonComponent;

    // inherit context
    Context myContext(&context());
    singleton<TestComponent(Dependency)> testComponent(myContext) ;

    // this will resolve:
    auto tc = inject<TestComponent>(myContext);
    tc->some_method_using_dependencies();
```


# design goals
- build clean obj tree
- constructor injection only
- no dependencies
- non invasive use plain testable c++ objects
- context inheritance
- no runtime parsing of xml files

