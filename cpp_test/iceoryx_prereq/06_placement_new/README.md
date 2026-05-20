# placement new — 在已有内存上构造对象

## 概述

placement new 是 C++ 的原地构造机制：**在指定的内存地址上构造 C++ 对象，不分配新内存**。

iceoryx 在共享内存上分配原始字节，然后用 placement new 在指定地址构造 C++ 对象。

## 语法

```cpp
// 语法：new (address) Type(constructor_args...);

// 普通 new：分配内存 + 构造对象（两步合一）
RadarObject* obj = new RadarObject(1.0, 2.0, 3.0);
//   1. 在堆上分配 sizeof(RadarObject) 字节
//   2. 在该地址上调用 RadarObject 构造函数

// placement new：在指定地址上构造（只构造，不分配）
RadarObject* obj = new (userPayload) RadarObject(1.0, 2.0, 3.0);
//                                    ^^^^^^^^^^^
//                                    指定构造地址 (共享内存中的一块)
```

## placement new 的对象必须手动析构

用 placement new 构造的对象**不能**用 `delete` 释放（内存不归堆管）。必须**手动调用析构函数**：

```cpp
obj->~RadarObject();  // 手动析构
// 注意：不能 delete obj，因为内存来自共享内存而非堆
```

## iceoryx loan() 内部流程

```
publisher.loan() 内部：
  1) 从共享内存分配一块原始内存 (userPayload)
  2) placement new 在该地址构造用户类型
  3) 返回 expected<Sample<T>, AllocationError>

用户调用 publish() 后：
  4) 对象被析构，内存归还共享内存池
```

## 常用场景

| 场景 | 说明 |
|------|------|
| 共享内存 | 在已有共享内存段上构造 C++ 对象 |
| 内存池 | 预分配大块内存，按需 placement new 构造 |
| 嵌入式 | 在特定物理地址上构造对象 |
| arena allocator | 在预分配的 arena 中分配和构造 |

## 运行

```bash
cmake --build build --target 06_placement_new
./build/bin/06_placement_new
```
