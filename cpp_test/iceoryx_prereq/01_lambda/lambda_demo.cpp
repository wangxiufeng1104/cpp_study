#include <iostream>
#include <string>

// ============================================================================
// Lambda 表达式 — [](){ } 匿名函数
// ============================================================================
// Lambda 是 iceoryx 中最常见的语法结构。你看到的 []() { ... } 本质上就是一个匿名函数。
//
// 完整语法：[capture](parameters) -> return_type { body }
// 简化版：  []() { body }                     — 无捕获，无参数
//           [&](auto& x) { body }             — 引用捕获所有外部变量
//           [=](auto x) { body }              — 拷贝捕获所有外部变量

// 函数对象写法（等价的旧式写法，用于对比理解）
struct GreeterOld {
    void operator()() const
    {
        std::cout << "  旧式函数对象写法（等价的 lambda）" << std::endl;
    }
};

// [this] 捕获示例
class Counter {
public:
    Counter() : m_count(0) {}

    void increment()
    {
        // [&] 引用捕获，隐式捕获 this，m_count 会被修改
        auto inc = [&]() { m_count++; };
        inc();
    }

    void incrementWithThis()
    {
        // [this] 显式捕获 this 指针，可直接访问成员
        auto inc = [this]() { m_count++; };
        inc();
    }

    void incrementByCopy()
    {
        // [=] 拷贝捕获，隐式拷贝 this，修改不影响原成员
        auto inc = [=]() mutable {
            m_count++; // 只修改拷贝的成员
        };
        inc();
    }

    int value() const { return m_count; }

private:
    int m_count;
};

int main()
{
    std::cout << "\n========== Lambda 表达式 ==========" << std::endl;

    // 1. 最简单的 lambda：[] 不捕获外部变量
    auto greet = []() { std::cout << "  1. 最简单的 lambda：不捕获外部变量" << std::endl; };
    greet();

    // 等价的旧式写法
    GreeterOld oldGreet;
    oldGreet();

    // 2. [&] 以引用捕获所有外部变量 —— 可以修改外部变量
    int ct = 42;
    auto byRef = [&]() {
        ct += 1; // 修改外部变量
    };
    byRef();
    std::cout << "  2. [&] 引用捕获后 ct = " << ct << " (原值被修改)" << std::endl;

    // 2b. 编译器将上述 lambda 展开为等价的函数对象（functor）：
    //
    // struct __lambda_byRef {
    //     int& ct;                    // [&] → 以引用方式捕获 ct
    //
    //     __lambda_byRef(int& _ct)    // 构造时传入 ct 的引用
    //         : ct(_ct) {}
    //
    //     void operator()() const {   // 默认 const（除非加 mutable）
    //         ct += 1;                // 通过引用修改外部变量
    //     }
    // };
    //
    // __lambda_byRef byRef{ct};      // 等价于 auto byRef = [&]() { ct += 1; };
    // byRef();                       // 等价于 byRef.operator()();
    //
    // 关键点：
    //   [&] → 捕获列表里每个外部变量都变成引用成员
    //   operator() 默认 const，但引用成员的 const 只限定引用本身不可
    //   重绑定，通过引用修改对象不受影响
    //   [=] → 成员是值拷贝，operator() 为 const 时不能修改（需加 mutable）

    // 等价的函数对象验证
    {
        int ct2 = 42;
        struct ByRefFunctor {
            int& ct;
            ByRefFunctor(int& _ct) : ct(_ct) {}
            void operator()() const { ct += 1; }
        };
        ByRefFunctor functor{ct2};
        functor();
        std::cout << "  2b. 函数对象写法验证: ct2 = " << ct2 << " (应与 ct 结果一致)" << std::endl;
    }

    // 3. [=] 以拷贝捕获所有外部变量 —— 不修改外部变量
    int num = 10;
    auto byCopy = [=]() {
        return num + 1; // 只读访问 num 的拷贝
    };
    std::cout << "  3. [=] 拷贝捕获: num + 1 = " << byCopy() << ", 原 num = " << num << " (不变)" << std::endl;

    // 4. [&x] 只以引用捕获 x
    std::string msg = "hello";
    auto append = [&msg]() { msg += " world"; };
    append();
    std::cout << "  4. [&msg] 只引用捕获 msg: " << msg << std::endl;

    // 5. [this] 捕获当前对象的 this 指针
    {
        Counter counter;

        // [&] 隐式捕获 this（在成员函数内等价于 [&, this]）
        counter.increment();
        std::cout << "  5. [&] 隐式捕获 this: " << counter.value() << std::endl;

        // [this] 显式捕获 — 只捕获 this，不捕获其他变量
        counter.incrementWithThis();
        std::cout << "  5. [this] 显式捕获: " << counter.value() << std::endl;
    }

    // 6. auto 参数：类型由编译器推导
    auto printAny = [](const auto& value) {
        std::cout << "  6. auto 推导: " << value << std::endl;
    };
    printAny(3.14);
    printAny(std::string("iceoryx"));

    // 7. Lambda 与捕获表对照
    std::cout << "\n  捕获符对照表:" << std::endl;
    std::cout << "    []    — 不捕获任何外部变量" << std::endl;
    std::cout << "    [&]   — 以引用方式捕获所有外部变量" << std::endl;
    std::cout << "    [=]   — 以拷贝方式捕获所有外部变量" << std::endl;
    std::cout << "    [&x]  — 只以引用捕获 x" << std::endl;
    std::cout << "    [this] — 捕获当前对象的 this 指针" << std::endl;

    return 0;
}
