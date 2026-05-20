# CRTP — Curiously Recurring Template Pattern

## 概述

CRTP (奇异递归模板模式) 是一种 C++ 模板技术：一个类继承自一个模板基类，并将**自己**作为模板参数传给基类。

```cpp
template <typename Derived>
struct Base {
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }
};

class MyClass : public Base<MyClass> { ... };
//                         ^^^^^^^^ 把自己传给基类
```

## 核心技巧

基类通过 `static_cast<Derived&>(*this)` 把自己**向下转型**为派生类，从而调用派生类的接口。这等价于 Rust 的派生宏 —— "为继承我的人提供默认方法实现"。

## 在 iceoryx 中的应用

`expected<T, E>` 继承 `FunctionalInterface<expected<T, E>, T, E>`，免费获得以下方法：

```cpp
expected<T, E>  ←继承了→  FunctionalInterface<expected<T, E>, T, E>
                              ├── .and_then(callback)    有值时调用 callback(value)
                              ├── .or_else(callback)     有错误时调用 callback(error)
                              ├── .expect("msg")         无值则 fatal error
                              └── .value_or(default)     有值返回值，无值返回 default
```

`optional<T>` 也继承同一个 `FunctionalInterface`，因此也有相同的方法集。

## 运行时多态 vs CRTP 静态多态

| 特性 | virtual 函数 | CRTP |
|------|:---:|:---:|
| 绑定时机 | 运行时 (vtable) | 编译时 (模板实例化) |
| 性能 | 有虚函数调用开销 | 零开销（编译时确定） |
| 灵活性 | 可存放不同类型到同一个容器 | 每个派生类是独立类型 |

## 运行

```bash
cmake --build build --target 03_crtp
./build/bin/03_crtp
```
