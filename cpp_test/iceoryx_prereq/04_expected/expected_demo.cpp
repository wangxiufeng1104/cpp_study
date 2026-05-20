#include <iostream>
#include <string>

#include "../expected.hpp"

// ============================================================================
// expected<T, E> — 无异常的"要么值要么错误"
// ============================================================================
// expected 是 iceoryx 最核心的类型，模仿 Rust 的 Result<T, E>。
// 一个 expected 对象只处于两种状态之一：
//   状态 A：包含值（成功）→ has_value() == true
//   状态 B：包含错误（失败）→ has_value() == false

struct RadarObject {
    double x;
    double y;
    double z;

    void publish() const
    {
        std::cout << "  [publish] RadarObject{" << x << ", " << y << ", " << z << "}" << std::endl;
    }
};

class SimulatedPublisher {
public:
    explicit SimulatedPublisher(int maxLoans)
        : m_maxLoans(maxLoans)
        , m_loanCount(0)
    {
    }

    expected<RadarObject, LoanError> loan()
    {
        if (m_loanCount >= m_maxLoans) {
            return err(LoanError::RUNNING_OUT_OF_CHUNKS);
        }
        m_loanCount++;
        return ok(RadarObject{0.0, 0.0, 0.0});
    }

private:
    int m_maxLoans;
    int m_loanCount;
};

int main()
{
    std::cout << "\n========== expected<T,E> — if-else 写法 (icehello 风格) ==========" << std::endl;

    // 最多 loan 2 次，第 3 次会失败
    SimulatedPublisher publisher(2);

    // 第 1 次 loan — 成功
    std::cout << "\n  第 1 次 loan:" << std::endl;
    {
        auto result = publisher.loan();
        if (result.has_value()) {
            // 状态 A：获取成功值
            auto& sample = result.value();
            sample.x = 1.0;
            sample.y = 2.0;
            sample.z = 3.0;
            sample.publish();
        } else {
            // 状态 B：获取错误码
            auto error = result.error();
            std::cout << "  错误: " << to_string(error) << std::endl;
        }
    }

    // 第 2 次 loan — 成功
    std::cout << "\n  第 2 次 loan:" << std::endl;
    {
        auto result = publisher.loan();
        if (result.has_value()) {
            auto& sample = result.value();
            sample.x = 4.0;
            sample.y = 5.0;
            sample.z = 6.0;
            sample.publish();
        } else {
            auto error = result.error();
            std::cout << "  错误: " << to_string(error) << std::endl;
        }
    }

    // 第 3 次 loan — 失败
    std::cout << "\n  第 3 次 loan (预期失败):" << std::endl;
    {
        auto result = publisher.loan();
        if (result.has_value()) {
            auto& sample = result.value();
            sample.publish();
        } else {
            auto error = result.error();
            std::cout << "  错误: " << to_string(error) << " (符合预期，已超过 loan 次数限制)" << std::endl;
        }
    }

    // value_or 用法
    std::cout << "\n  value_or 用法:" << std::endl;
    {
        expected<std::string, LoanError> failResult = err(LoanError::SEMAPHORE_FULL);
        std::cout << "    failResult.value_or(\"fallback\") = \""
                  << failResult.value_or(std::string("fallback")) << "\"" << std::endl;
    }

    // ok() 工厂函数（返回成功值）
    std::cout << "\n  工厂函数:" << std::endl;
    std::cout << "    ok(value)  → 构造成功的 expected" << std::endl;
    std::cout << "    ok()       → 构造成功的 expected<void,E>" << std::endl;
    std::cout << "    err(code)  → 构造失败的 expected" << std::endl;

    // [[nodiscard]] 属性
    std::cout << "\n  [[nodiscard]] 属性:" << std::endl;
    std::cout << "    expected 被标记为 [[nodiscard]]。" << std::endl;
    std::cout << "    如果忽略返回值，编译器会生成警告，防止遗漏错误检查。" << std::endl;

    return 0;
}
