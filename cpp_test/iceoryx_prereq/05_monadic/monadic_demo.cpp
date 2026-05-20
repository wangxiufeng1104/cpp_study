#include <iostream>
#include <string>

#include "../expected.hpp"

// ============================================================================
// and_then / or_else — Monadic 链式调用
// ============================================================================
// 核心思想：将"if 成功 { ... } else { ... }" 变成链式调用，
// 每一步返回 expected，错误自动短路。

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
    // ================================================================
    // 示例 1：monadic 链式写法（icedelivery 风格）
    // ================================================================
    std::cout << "\n========== and_then/or_else 链式写法 ==========" << std::endl;

    double sampleValue = 7.0;

    // 使用 .and_then() / .or_else() 替代 if-else
    // 短路语义：如果 loan() 失败，.and_then 跳过，直接执行 .or_else
    {
        SimulatedPublisher publisher(2);

        (void)(publisher.loan()
            .and_then([&](auto& sample) {
                sample.x = sampleValue;
                sample.y = sampleValue;
                sample.z = sampleValue;
                sample.publish();
            })
            .or_else([](auto& error) {
                std::cout << "  无法 loan sample, 错误: " << to_string(error) << std::endl;
            }));

        (void)(publisher.loan()
            .and_then([&](auto& sample) {
                sample.x = sampleValue + 1;
                sample.y = sampleValue + 1;
                sample.z = sampleValue + 1;
                sample.publish();
            })
            .or_else([](auto& error) {
                std::cout << "  无法 loan sample, 错误: " << to_string(error) << std::endl;
            }));

        // 第 3 次 — 预期失败
        (void)(publisher.loan()
            .and_then([&](auto& sample) {
                sample.x = 99.0;
                sample.publish();
            })
            .or_else([](auto& error) {
                std::cout << "  无法 loan sample, 错误: " << to_string(error) << " (预期)" << std::endl;
            }));
    }

    // ================================================================
    // 示例 2：短路语义
    // ================================================================
    std::cout << "\n========== 短路语义 ==========" << std::endl;

    auto always_fail = []() -> expected<int, LoanError> {
        return err(LoanError::SEMAPHORE_FULL);
    };

    // and_then 不会执行，直接跳到 or_else
    (void)(always_fail()
        .and_then([](auto& value) {
            std::cout << "  [这条不会打印]" << std::endl;
        })
        .or_else([](auto& error) {
            std::cout << "  短路生效: and_then 被跳过，直接 or_else，错误=" << to_string(error) << std::endl;
        }));

    // value_or 用法
    {
        auto result = always_fail();
        int val = result.value_or(100);
        std::cout << "  result.value_or(100) = " << val << " (失败时返回默认值)" << std::endl;
    }

    {
        auto success = ok(42);
        std::cout << "  success.value_or(100) = " << success.value_or(100) << " (成功时返回实际值)" << std::endl;
    }

    // ================================================================
    // 示例 3：if-else 与 and_then/or_else 等价转换
    // ================================================================
    std::cout << "\n========== if-else vs and_then/or_else 等价转换 ==========" << std::endl;

    double ct = 99.0;

    // if-else 写法（icehello 风格）
    std::cout << "  --- if-else 写法 ---" << std::endl;
    {
        SimulatedPublisher pub(1);
        auto result = pub.loan();
        if (result.has_value()) {
            auto& sample = result.value();
            sample.x = ct;
            sample.z = ct;
            sample.publish();
        } else {
            auto error = result.error();
            std::cout << "  错误: " << to_string(error) << std::endl;
        }
    }

    // monadic 写法（icedelivery 风格）
    std::cout << "  --- and_then/or_else 写法 ---" << std::endl;
    {
        SimulatedPublisher pub(1);
        (void)(pub.loan()
            .and_then([&](auto& sample) {
                sample.x = ct;
                sample.z = ct;
                sample.publish();
            })
            .or_else([](auto& error) {
                std::cout << "  错误: " << to_string(error) << std::endl;
            }));
    }

    std::cout << "\n  二者完全等价，monadic 写法避免了 result 临时变量和两层分支嵌套。" << std::endl;

    // ================================================================
    // 示例 4：短路流程图示
    // ================================================================
    std::cout << "\n  短路流程图:" << std::endl;
    std::cout << "    publisher.loan()           ← 返回 expected<RadarObject, LoanError>" << std::endl;
    std::cout << "      .and_then(callback)      ← 成功时执行 (RadarObject&) -> void" << std::endl;
    std::cout << "      .or_else(error_handler)  ← 失败时执行 (LoanError&) -> void" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "    如果 loan() 失败: .and_then 被跳过 → .or_else 直接执行" << std::endl;
    std::cout << "    如果 loan() 成功: .and_then 执行 → .or_else 被跳过" << std::endl;

    return 0;
}
