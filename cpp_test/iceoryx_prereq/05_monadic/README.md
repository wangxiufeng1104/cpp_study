# and_then / or_else — Monadic 链式调用

## 概述

核心思想：将"if 成功 { ... } else { ... }" 变成**链式调用**，每一步返回 `expected`，**错误自动短路**。

## 对照表

| if-else 写法 | monadic 写法 |
|---|---|
| `if (result.has_value()) { result.value()... }` | `.and_then([](auto& value) { ... })` |
| `else { result.error()... }` | `.or_else([](auto& error) { ... })` |

## 示例

```cpp
// icedelivery 风格 (monadic)
publisher.loan()
    .and_then([&](auto& sample) {
        sample->x = sampleValue;
        sample.publish();       // lambda 返回 void
    })
    .or_else([](auto& error) {
        std::cerr << "error: " << error;
    });
```

## 短路语义

```
publisher.loan()         ← 返回 expected
    .and_then(callback)  ← loan() 成功时执行
    .or_else(handler)    ← loan() 失败时执行（.and_then 被跳过）
```

如果 `publisher.loan()` 失败，`.and_then` 跳过，直接执行 `.or_else`。  
如果 `publisher.loan()` 成功，`.and_then` 执行，`.or_else` 被跳过。

## value_or

有值返回值，无值返回默认值：

```cpp
auto result = always_fail();
int val = result.value_or(100);  // result 失败 → val = 100
```

## 等价转换

```
icehello (if-else)              icedelivery (monadic)
─────────────────────────────────────────────────────
auto result = pub.loan();       pub.loan()
if (result.has_value()) {           .and_then([&](auto& s) {
    auto& s = result.value();           s->x = ct;
    s->x = ct;                          s->z = ct;
    s->z = ct;                          s.publish();
    s.publish();                    })
} else {                           .or_else([](auto& e) {
    auto e = result.error();           cerr << e;
    cerr << e;                     });
}
```

二者完全等价，monadic 写法避免了 `result` 临时变量和两层分支嵌套。

## 运行

```bash
cmake --build build --target 05_monadic
./build/bin/05_monadic
```
