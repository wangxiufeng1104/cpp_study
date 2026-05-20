#pragma once

#include <new>
#include <type_traits>
#include <utility>

// ============================================================================
// 共享 expected<T, E> 实现（供 04_expected 和 05_monadic 共用）
// ============================================================================
//
// 为什么需要 expected？
// iceoryx 禁用 C++ 异常（-fno-exceptions），用 expected 强制调用方
// 处理"成功"和"失败"两种可能性，避免遗漏错误检查。
//
// 一个 expected 对象只处于两种状态之一：
//   状态 A：包含值（成功） → has_value() == true
//   状态 B：包含错误（失败） → has_value() == false
//
// 模仿 Rust 的 Result<T, E>。

// ============================================================================
// 错误码枚举
// ============================================================================
enum class LoanError {
    RUNNING_OUT_OF_CHUNKS,  // 共享内存块耗尽
    SEMAPHORE_FULL,         // 信号量满
    PORT_NOT_AVAILABLE,     // 端口不可用
};

// 错误码转字符串（便于打印调试）
inline const char* to_string(LoanError e)
{
    switch (e) {
    case LoanError::RUNNING_OUT_OF_CHUNKS:
        return "RUNNING_OUT_OF_CHUNKS";
    case LoanError::SEMAPHORE_FULL:
        return "SEMAPHORE_FULL";
    case LoanError::PORT_NOT_AVAILABLE:
        return "PORT_NOT_AVAILABLE";
    }
    return "UNKNOWN";
}

// ============================================================================
// 前置声明
// ============================================================================
// FunctionalInterface 的 and_then() 中需要用 expected<void, E> 作为
// void 返回值的载体，所以必须在 FunctionalInterface 之前声明 expected。
//
// 为什么声明两个？
//   - 第一行：声明主模板 expected<T, E>（两个参数都可变）
//   - 第二行：声明偏特化 expected<void, E>（T 固定为 void，只 E 可变）
//
// 偏特化比主模板"更特化"，当 T=void 时编译器优先匹配偏特化版本。

template <typename T, typename E>
class expected;

template <typename E>
class expected<void, E>;

// ============================================================================
// FunctionalInterface — CRTP 基类
// ============================================================================
//
// CRTP（奇异递归模板模式）：基类通过 static_cast<Derived&>(*this) 向下转型
// 为派生类，从而调用派生类的 has_value() / value() / error() 方法。
//
// 这样 expected<T,E> 只需继承 FunctionalInterface<expected<T,E>, T, E>，
// 就免费获得下面所有方法，不需要每个派生类自己重写。
//
// 模板参数说明：
//   Derived    — 派生类类型（如 expected<T,E>）
//   ValueType  — 成功时携带的值类型（如 RadarObject、int、void）
//   ErrorType  — 失败时携带的错误类型（如 LoanError）

template <typename Derived, typename ValueType, typename ErrorType>
struct FunctionalInterface {
    // 将 this 向下转型为派生类引用（CRTP 核心操作）
    Derived& derived() { return static_cast<Derived&>(*this); }

    // ========================================================================
    // and_then : 有值时执行 callback，无值时错误短路
    // ========================================================================
    //
    // 语义：
    //   有值  → 调用 callback(value)，callback 的返回值成为新的 expected
    //   无值  → 不调用 callback，把错误"转发"到返回的 expected 中（短路）
    //
    // callback 可能返回两种东西：
    //   A) 返回 void     → and_then 返回 expected<void, ErrorType>
    //      （只在有值时执行副作用：赋值、publish 等）
    //   B) 返回某种值 X  → and_then 返回 expected<X, ErrorType>
    //      （串联下一个可能失败的操作）
    //
    // 为什么有两个重载（& 和 &&）？
    //   &  — 对象是左值（有名字的变量）时调用，返回 what callback returns
    //   && — 对象是右值（临时对象）时调用
    //        如 publisher.loan().and_then(...)  — loan() 返回临时对象

    // & 重载：用于左值对象
    template <typename Functor>
    auto and_then(Functor&& callable) &
    {
        using CallReturn = decltype(callable(std::declval<ValueType&>()));
        if constexpr (std::is_void_v<CallReturn>) {
            // callback 返回 void → 只做副作用，返回 expected<void, E>
            if (derived().has_value()) {
                callable(derived().value());
                return expected<void, ErrorType>();      // 成功（无值）
            }
            return expected<void, ErrorType>(derived().error());  // 失败（转发错误）
        } else {
            // callback 返回非 void → 返回 callback 的返回值本身
            if (derived().has_value()) {
                return callable(derived().value());
            }
            return CallReturn(derived().error());  // 失败时用错误构造 callback 的返回类型
        }
    }

    // && 重载：用于右值（临时）对象
    template <typename Functor>
    auto and_then(Functor&& callable) &&
    {
        using CallReturn = decltype(callable(std::declval<ValueType&>()));
        if constexpr (std::is_void_v<CallReturn>) {
            if (derived().has_value()) {
                callable(derived().value());
                return expected<void, ErrorType>();
            }
            return expected<void, ErrorType>(derived().error());
        } else {
            if (derived().has_value()) {
                return callable(derived().value());
            }
            return CallReturn(derived().error());
        }
    }

    // ========================================================================
    // or_else : 有错误时执行 error_handler，无错误时跳过
    // ========================================================================
    //
    // 语义：
    //   有错误 → 调用 error_handler(error)，返回自身引用（用于继续链式调用）
    //   无错误 → 什么都不做，返回自身引用
    //
    // 为什么 & 返回引用，&& 返回值？
    //   &  — 左值还在作用域内，返回引用安全
    //   && — 右值即将销毁，必须把值移出来（return std::move(derived())）
    //         这样链式调用在右值上也能工作

    template <typename Functor>
    Derived& or_else(Functor&& callable) &
    {
        if (!derived().has_value()) {
            callable(derived().error());
        }
        return derived();
    }

    template <typename Functor>
    Derived or_else(Functor&& callable) &&
    {
        if (!derived().has_value()) {
            callable(derived().error());
        }
        return std::move(derived());
    }

    // ========================================================================
    // value_or : 有值返回值，无值返回默认值
    // ========================================================================
    //
    // 语义：
    //   有值  → 返回持有的值
    //   无值  → 返回提供的默认值（不修改 expected 本身）
    //
    // 用法：int v = result.value_or(42);
    //       成功 → v = result 的值
    //       失败 → v = 42

    ValueType value_or(ValueType&& default_value) &
    {
        if (derived().has_value()) {
            return std::move(derived().value());  // 移交所有权（避免拷贝）
        }
        return std::move(default_value);
    }

    ValueType value_or(const ValueType& default_value) const&
    {
        if (derived().has_value()) {
            return derived().value();   // const 下只能拷贝
        }
        return default_value;
    }
};

// ============================================================================
// expected<T, E> — 主模板（T ≠ void）
// ============================================================================
//
// 内部用 union 存储：要么存 T（值），要么存 E（错误），二者共用同一块内存。
// union 不能自动区分当前存储的是什么，所以用 m_hasValue 标记。
//
// [[nodiscard]]：如果调用方忽略返回值，编译器生成警告。
// 防止忘了检查 loan() 是否成功。

template <typename T, typename E>
class [[nodiscard]] expected : public FunctionalInterface<expected<T, E>, T, E> {
public:
    // --- 构造：成功路径 ---
    // placement new 在 union 的 m_value 地址上构造 T 对象
    expected(const T& value) : m_hasValue(true), m_error{} { new (&m_value) T(value); }
    expected(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_hasValue(true), m_error{} { new (&m_value) T(std::move(value)); }

    // --- 构造：失败路径 ---
    // placement new 在 union 的 m_error 地址上构造 E 对象
    expected(E error) : m_hasValue(false), m_value{} { new (&m_error) E(error); }

    // --- 拷贝构造 ---
    // 根据源对象的状态，选择拷贝 m_value 或 m_error
    expected(const expected& other) : m_hasValue(other.m_hasValue)
    {
        if (m_hasValue) new (&m_value) T(other.m_value);
        else new (&m_error) E(other.m_error);
    }

    // --- 移动构造 ---
    expected(expected&& other) noexcept : m_hasValue(other.m_hasValue)
    {
        if (m_hasValue) new (&m_value) T(std::move(other.m_value));
        else new (&m_error) E(std::move(other.m_error));
    }

    // --- 析构 ---
    // union 成员需要手动析构（编译器不知道哪一个活跃）
    ~expected()
    {
        if (m_hasValue) m_value.~T();   // 销毁值
        else m_error.~E();              // 销毁错误
    }

    // --- 状态查询 ---
    bool has_value() const { return m_hasValue; }

    // --- 访问值（调用前应先检查 has_value()）---
    T& value() { return m_value; }
    const T& value() const { return m_value; }

    // --- 访问错误（调用前应先检查 !has_value()）---
    E& error() { return m_error; }
    const E& error() const { return m_error; }

private:
    bool m_hasValue;          // true = 当前存的是值，false = 当前存的是错误
    union {
        T m_value;            // 成功时使用
        E m_error;            // 失败时使用
        // T 和 E 共用同一块内存，节省空间
    };
};

// ============================================================================
// expected<void, E> — 偏特化（T = void）
// ============================================================================
//
// 当 T = void 时，没有值可存储，只关心"成功 or 失败"。
//
// 与主模板的区别：
//   1. 不继承 FunctionalInterface（and_then 和 value_or 对 void 无意义）
//   2. 没有 value() 方法（无值可返回）
//   3. 没有 m_value 成员（union 只有 m_error，不需要区分状态时的内存共享）
//   4. 手动提供 or_else（因为不继承 FunctionalInterface）
//
// 使用场景：ok() 不带参数 → 返回 expected<void, LoanError>（只表示成功）
//          and_then 返回 void  → 返回 expected<void, LoanError>

template <typename E>
class [[nodiscard]] expected<void, E> {
public:
    // 默认构造 = 成功（无值）
    expected() : m_hasValue(true), m_error{} {}

    // 从错误构造 = 失败
    expected(E error) : m_hasValue(false) { new (&m_error) E(error); }

    // 拷贝构造
    expected(const expected& other) : m_hasValue(other.m_hasValue)
    { if (!m_hasValue) new (&m_error) E(other.m_error); }

    // 移动构造
    expected(expected&& other) noexcept : m_hasValue(other.m_hasValue)
    { if (!m_hasValue) new (&m_error) E(std::move(other.m_error)); }

    // 析构
    ~expected() { if (!m_hasValue) m_error.~E(); }

    // 状态查询
    bool has_value() const { return m_hasValue; }

    // 访问错误
    E& error() { return m_error; }
    const E& error() const { return m_error; }

    // or_else: 为链式调用 .and_then(void_functor).or_else(error_handler) 提供
    // & 重载：左值调用
    template <typename Functor>
    expected& or_else(Functor&& callable) &
    {
        if (!m_hasValue) callable(m_error);
        return *this;
    }
    // && 重载：右值调用
    template <typename Functor>
    expected or_else(Functor&& callable) &&
    {
        if (!m_hasValue) callable(m_error);
        return std::move(*this);
    }

private:
    bool m_hasValue;
    union { E m_error; };
    // 只有 m_error，不需要 m_value — 无值可存
};

// ============================================================================
// 工厂函数
// ============================================================================
// 用户不直接写 expected<T, LoanError>(value)，而是用 ok(value) / err(code)，
// 让编译器自动推导 T，代码更简洁。

// ok(value)：构造一个成功的 expected
//   ok(42)        → expected<int, LoanError>
//   ok("hello"s)  → expected<std::string, LoanError>
template <typename T>
auto ok(T&& value)
{
    return expected<std::decay_t<T>, LoanError>(std::forward<T>(value));
}

// ok()：构造一个成功的 expected<void, LoanError>（只携带"成功"信息）
inline auto ok()
{
    return expected<void, LoanError>();
}

// err(code)：构造一个失败的 expected
// 注意：返回的是 LoanError 值本身，依赖 expected 的隐式构造（expected(E error)）
// 当用在 return expected<T, LoanError> 上下文中时，编译器自动调用 expected(E) 构造
inline auto err(LoanError e)
{
    return e;
}
