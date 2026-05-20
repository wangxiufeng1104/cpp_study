#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

// ============================================================================
// function_ref — 轻量回调引用（零开销替代 std::function）
// ============================================================================
// function_ref 是 iceoryx 对 std::function 的零开销替代，它不拷贝也不拥有被引用的 callable。
// 本质上只是一个 {void* + 函数指针}，完全在栈上存储，无堆分配。
//
// 什么是 callable（可调用对象）？
// callable 是 C++ 中所有能用 operator() 调用的类型的统称，包括：
//
//   1. 普通函数           void f(int);
//   2. 函数指针           void (*fp)(int) = &f;
//   3. 函数对象(functor)  struct F { void operator()(int); };
//   4. Lambda 表达式      [](int x) { ... }
//   5. std::function      std::function<void(int)>
//   6. 成员函数指针       (obj.*pmf)(args) 或 std::invoke(pmf, obj, args)
//
// 共同特征：都可以写成 callable(args...) 的形式来调用。

// 演示各种 callable 类型
void plainFunction(int x)
{
    std::cout << "    普通函数: " << x << std::endl;
}

struct Functor {
    void operator()(int x) const
    {
        std::cout << "    函数对象: " << x << std::endl;
    }
};

template <typename Signature>
class function_ref;

template <typename R, typename... Args>
class function_ref<R(Args...)> {
public:
    template <typename F>
    function_ref(F&& f) noexcept
    {
        using Fn = std::remove_reference_t<F>;

        if constexpr (std::is_function_v<Fn>) {
            // f 是函数引用（如 plainFunction），存储函数指针值本身
            Fn* fnPtr = &f;
            obj_ = reinterpret_cast<void*>(fnPtr);
            invoke_ = +[](void* obj, Args... args) -> R {
                return reinterpret_cast<Fn*>(obj)(std::forward<Args>(args)...);
            };
        } else {
            // f 是函数对象/Lambda/functor/std::function，存储对象地址
            obj_ = const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f)));
            invoke_ = +[](void* obj, Args... args) -> R {
                return (*static_cast<Fn*>(obj))(std::forward<Args>(args)...);
            };
        }
    }

    R operator()(Args... args) const
    {
        return invoke_(obj_, std::forward<Args>(args)...);
    }

private:
    void* obj_;
    R (*invoke_)(void*, Args...);
};

// 使用 function_ref 作为回调参数的函数
void callTwice(int x, const function_ref<void(int)>& callback)
{
    callback(x);
    callback(x);
}

int main()
{
    // ================================================================
    // 0. 什么是 callable？（演示 6 种类型）
    // ================================================================
    std::cout << "\n========== 什么是 callable？ ==========" << std::endl;
    std::cout << "  callable = 所有能用 () 调用的类型：普通函数、函数指针、" << std::endl;
    std::cout << "  函数对象(functor)、Lambda、std::function、成员函数指针。" << std::endl;
    std::cout << "  共同特征：都可以写成 callable(args...) 的形式来调用。" << std::endl;

    // 1. 普通函数
    std::cout << "\n  1. ";
    {
        function_ref<void(int)> r1 = plainFunction;
        r1(1);
    }

    // 2. 函数指针
    void (*fptr)(int) = plainFunction;
    std::cout << "  2. ";
    {
        function_ref<void(int)> r2 = fptr;
        r2(2);
    }

    // 3. 函数对象 (functor) — 注意：必须保持 functor 存活
    Functor functor{};
    std::cout << "  3. ";
    {
        function_ref<void(int)> r3 = functor;
        r3(3);
    }

    // 4. Lambda 表达式 — 注意：必须保持 lambda 存活
    auto lambda1 = [](int x) { std::cout << "    Lambda: " << x << std::endl; };
    std::cout << "  4. ";
    {
        function_ref<void(int)> r4 = lambda1;
        r4(4);
    }

    // 5. std::function → function_ref（注意：std::function 需保持存活）
    std::function<void(int)> sf = [](int x) { std::cout << "    std::function: " << x << std::endl; };
    std::cout << "  5. ";
    {
        function_ref<void(int)> r5 = sf;
        r5(5);
    }

    // 6. 带捕获的 Lambda（iceoryx and_then/or_else 中最常见的用法）
    int capturedValue = 42;
    auto lambda2 = [capturedValue](int x) { std::cout << "    带捕获 Lambda: " << x << ", 捕获值=" << capturedValue << std::endl; };
    std::cout << "  6. ";
    {
        function_ref<void(int)> r6 = lambda2;
        r6(6);
    }

    // ================================================================
    // function_ref 与 std::function 对比
    // ================================================================
    std::cout << "\n========== function_ref vs std::function ==========" << std::endl;

    // std::function — 拥有被包装的 callable，可能堆分配
    std::function<void(int)> stdFunc = [](int x) {
        std::cout << "  std::function 回调: " << x << std::endl;
    };
    stdFunc(100);

    // function_ref — 不拥有 callable，零开销
    auto lambdaRef = [](int x) {
        std::cout << "  function_ref 回调: " << x << std::endl;
    };
    function_ref<void(int)> refFunc = lambdaRef;
    refFunc(200);

    // 对比内存占用
    std::cout << "\n  内存占用对比:" << std::endl;
    std::cout << "    sizeof(std::function<void()>) = " << sizeof(std::function<void()>) << std::endl;
    std::cout << "    sizeof(function_ref<void()>) = " << sizeof(function_ref<void()>) << std::endl;

    // function_ref 作为参数传递
    std::cout << "\n  function_ref 作为回调参数:" << std::endl;
    callTwice(5, [](int val) { std::cout << "    回调被调用: " << val << std::endl; });

    // ⚠️ function_ref 不拥有 callable，注意生命周期
    std::cout << "\n  ⚠️注意事项:" << std::endl;
    std::cout << "    function_ref 不拥有被引用的 callable。" << std::endl;
    std::cout << "    被引用的 callable 必须在 function_ref 使用期间保持存活。" << std::endl;

    std::cout << "\n  对比总结:" << std::endl;
    std::cout << "    std::function  — 拥有 callable，类型擦除有开销，可能堆分配" << std::endl;
    std::cout << "    function_ref   — 不拥有 callable，{void* + 函数指针}，栈上零开销" << std::endl;

    return 0;
}
