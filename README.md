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

    // assume class Dependency is defined

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

just declare context with dependencies, then use it

```cpp
    int main()
    {
        ApplicationContext  ctx {
            diy::singleton<TestComponent(Dependency)>,
            diy::singleton<Dependency()> 
        };
        
        auto testComponent = inject<TestComponent>(ctx);
        testComponent->call_some_method();
        
        return 0;
    }
```


components can be registered as static globals,
within main or even in some init() function called from main.

### as singleton

```cpp
        // singleton<T> will return always the same instance for this context
        ApplicationContext  ctx {
            diy::singleton<TestComponent(Dependency)>,
            diy::singleton<Dependency()> 
        };
```
### as provider

```cpp
        // provider will hand out a new obj for every injection
        ApplicationContext  ctx {
            diy::provider<TestComponent(Dependency)>,
            ...
        };
```
### as value
```cpp
        // use any std::shared_ptr<T> and register with context
        auto tc = std::make_shared<TestComponent>();
        
        ApplicationContext  ctx {
            diy::ctx_value<TestComponent>(&tc),
            ---
        }
```        


# context inheritance and the default context

contexts can inherit, for example a local context inheriting from a default context:

```cpp
    using namespace diy;

    // dependency declared in default context
    ApplicationContext ctx {
        singleton<Dependency()> singletonComponent;
    };

    // inherit context
    Context childCtx(&ctx);
    childCtx->registerFactory<TestComponent(Dependency)>();

    // this will resolve:
    auto tc = inject<TestComponent>(childCtx);
    tc->some_method_using_dependencies();
```


# design goals
- build clean obj tree
- constructor injection only
- no dependencies
- non invasive - use plain testable c++ objects
- context inheritance
- no runtime parsing of xml files

