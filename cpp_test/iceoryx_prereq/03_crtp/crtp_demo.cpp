#include <iostream>
#include <string>

// ============================================================================
// CRTP — Curiously Recurring Template Pattern（奇异递归模板模式）
// ============================================================================
// 基类把自己转换回派生类类型来调用派生类的方法。
// iceoryx 中 expected<T,E> 继承 FunctionalInterface<expected<T,E>, T, E>，
// 从而免费获得 and_then / or_else / expect / value_or 等所有方法。
// 等价于 Rust 的派生宏 —— "为继承我的人提供默认方法实现"。

// 经典 CRTP 示例：为派生类提供 operator==
template <typename Derived>
class Equatable {
public:
    friend bool operator==(const Derived& lhs, const Derived& rhs)
    {
        // 基类调用派生类的 equals() 方法
        return lhs.equals(rhs);
    }

    friend bool operator!=(const Derived& lhs, const Derived& rhs)
    {
        return !(lhs == rhs);
    }
};

class Point : public Equatable<Point> {
public:
    Point(int x, int y) : m_x(x), m_y(y) {}

    // 派生类必须实现 equals()
    bool equals(const Point& other) const
    {
        return m_x == other.m_x && m_y == other.m_y;
    }

    int x() const { return m_x; }
    int y() const { return m_y; }

private:
    int m_x;
    int m_y;
};

// CRTP 的另一种形式：接口类通过静态多态实现方法
template <typename Derived>
class Printable {
public:
    // 基类通过向下转型调用派生类的 toString()
    void print() const
    {
        // static_cast<const Derived&>(*this) 是 CRTP 的核心
        std::cout << static_cast<const Derived&>(*this).toString() << std::endl;
    }
};

class User : public Printable<User> {
public:
    User(const std::string& name) : m_name(name) {}
    std::string toString() const { return "User: " + m_name; }

private:
    std::string m_name;
};

int main()
{
    std::cout << "\n========== CRTP 模式 ==========" << std::endl;

    // 示例 1：Equatable
    std::cout << "\n  1. Equatable CRTP:" << std::endl;
    Point p1(1, 2);
    Point p2(1, 2);
    Point p3(3, 4);
    std::cout << "    p1 == p2: " << (p1 == p2 ? "true" : "false") << std::endl;
    std::cout << "    p1 == p3: " << (p1 == p3 ? "true" : "false") << std::endl;
    // Point 通过继承 Equatable<Point> 免费获得了 operator== 和 operator!=

    // 示例 2：Printable
    std::cout << "\n  2. Printable CRTP:" << std::endl;
    User u("alice");
    std::cout << "    ";
    u.print();
    // User 通过继承 Printable<User> 免费获得了 print()

    // CRTP 的本质
    std::cout << "\n  3. CRTP 关键技巧:" << std::endl;
    std::cout << "    template <typename Derived>" << std::endl;
    std::cout << "    struct Base {" << std::endl;
    std::cout << "      Derived& derived() {" << std::endl;
    std::cout << "        return static_cast<Derived&>(*this);" << std::endl;
    std::cout << "      }" << std::endl;
    std::cout << "    };" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "    class MyClass : public Base<MyClass> { ... };" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  Base 通过 static_cast<Derived&>(*this) 把自己" << std::endl;
    std::cout << "  向下转型为 Derived，从而调用派生类的接口。" << std::endl;

    std::cout << "\n  在 iceoryx 中的应用:" << std::endl;
    std::cout << "    expected<T,E> : FunctionalInterface<expected<T,E>, T, E>" << std::endl;
    std::cout << "      → 免费获得 .and_then() / .or_else() / .value_or()" << std::endl;
    std::cout << "    optional<T>   : FunctionalInterface<optional<T>, T, nullopt_t>" << std::endl;
    std::cout << "      → 免费获得 .and_then() / .or_else() / .value_or()" << std::endl;

    return 0;
}
