# Lambda 表达式

## 概述

Lambda 是 C++11 引入的匿名函数语法，也是 **iceoryx 中最常见的语法结构**。在阅读 iceoryx 代码时，`[]() { ... }` 的身影几乎无处不在。

## 语法

```
[capture](parameters) -> return_type { body }
```

| 部分 | 说明 |
|------|------|
| `[capture]` | **捕获列表**：决定 lambda 体内可以访问哪些外部变量 |
| `(parameters)` | **参数列表**：与普通函数相同 |
| `-> return_type` | **返回类型**：可省略，编译器自动推导 |
| `{ body }` | **函数体** |

## 捕获符对照表

| 捕获符 | 含义 | 能修改外部变量? |
|--------|------|:---:|
| `[]` | 不捕获任何外部变量 | — |
| `[&]` | 以**引用**方式捕获所有外部变量 | ✅ |
| `[=]` | 以**拷贝**方式捕获所有外部变量 | ❌ (除非加 `mutable`) |
| `[&x]` | 只以引用捕获 x | ✅ x |
| `[=, &x]` | 默认拷贝捕获，但 x 以引用捕获 | ✅ x |
| `[this]` | 捕获当前对象的 this 指针 | ✅ 成员变量 |

## 在 iceoryx 中

iceoryx 的 `and_then` / `or_else` 的参数就是 lambda：

```cpp
publisher.loan()
    .and_then([&](auto& sample) {   // [&] 引用捕获外部变量 ct
        sample->x = ct;             // ct 来自外部作用域
        sample.publish();
    })
    .or_else([](auto& error) {      // [] 不捕获任何外部变量
        std::cerr << error;
    });
```

## 运行

```bash
cmake --build build --target 01_lambda
./build/bin/01_lambda
```
