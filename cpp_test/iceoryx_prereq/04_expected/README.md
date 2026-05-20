# expected\<T, E\> — 无异常的"要么值要么错误"

## 概述

`expected<T, E>` 是 iceoryx 最核心的类型，模仿 Rust 的 `Result<T, E>`。

iceoryx 禁用异常（`-fno-exceptions`），用 `expected` **强制调用方处理两种可能性**，避免遗漏错误检查。

## 两种状态

```
expected<RadarObject, AllocationError> result;

// 状态 A：包含值（成功）
result.has_value() == true
result.value()      → RadarObject&

// 状态 B：包含错误（失败）
result.has_value() == false
result.error()      → AllocationError
```

## 构造方式

```cpp
// 返回成功 — 用 ok() 包装值
return ok(RadarObject{1.0, 2.0, 3.0});
return ok();                         // 值是 void 时

// 返回失败 — 用 err() 包装错误
return err(AllocationError::RUNNING_OUT_OF_CHUNKS);
```

## if-else 访问方式（icehello 风格）

```cpp
auto result = publisher.loan();
if (result.has_value()) {
    auto& sample = result.value();   // 获取成功值
    sample->x = 42.0;
} else {
    auto error = result.error();     // 获取错误码
}
```

## [[nodiscard]]

`expected` 被标记为 `[[nodiscard]]`，如果忽略返回值，编译器会生成警告。这确保不会遗漏错误检查。

## expected\<void, E\> 特化

只关心成功/失败，不携带值。`ok()` 不带参数时返回此类型。

## 运行

```bash
cmake --build build --target 04_expected
./build/bin/04_expected
```
