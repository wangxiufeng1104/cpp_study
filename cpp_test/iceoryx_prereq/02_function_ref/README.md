# function_ref — 从 C 语言角度理解

## 先看问题：C 语言怎么传递"回调函数 + 数据"？

C 语言里，如果你想让一个函数接受"回调 + 数据"，会这样写：

```c
// C 语言：回调 = 函数指针 + void* 用户数据
void for_each(int* arr, int len, void (*callback)(int, void*), void* userdata) {
    for (int i = 0; i < len; i++) {
        callback(arr[i], userdata);   // ← 调用时把 userdata 传回去
    }
}

// 使用者：
int threshold = 10;
for_each(arr, 5, my_print, &threshold);  // 把 threshold 地址塞进 void*
```

`void*` 是 C 语言里"丢掉类型信息"的手段 —— 不管原来是什么指针，塞进 `void*` 后编译器就忘掉它的类型。回调函数里再 `(int*)userdata` 强制转回来。

**`function_ref` 就是 C++ 对上面这个模式的封装。** 只是不拆分"函数指针 + void*"两个参数，而是打包成一个对象。

## function_ref 是什么

`function_ref` 是一个**轻量级的回调包装器**。它只存两个东西：

```
function_ref 对象（16 字节）
┌─────────────────────────────┐
│  void* obj_          (8B)   │  ← 存"回调数据"的地址（对应 C 的 userdata）
│  R (*invoke_)(...)   (8B)   │  ← 存"怎么调用"的函数指针（对应 C 的 callback）
└─────────────────────────────┘
```

和 C 的区别只是：C 里回调函数和 userdata 分离传递，`function_ref` 把它们绑在一起。

**不拷贝、不拥有**被包装的对象 —— 只是记住它的地址。所以被包装的对象必须在 `function_ref` 使用期间活着。

## 对比 `std::function`

| | `std::function` | `function_ref` |
|---|---|---|
| 拥有回调对象吗？ | 是（拷贝/移动一份） | 否（只记地址） |
| sizeof | 32 字节 | 16 字节 |
| 堆分配？ | 可能 | 绝不 |
| C 等价 | 像 `malloc` 了一块存回调上下文 | 像只存了 {函数指针, void*} |

## 类型擦除是什么（从 C 的角度）

C 语言里，`void*` 就是最原始的类型擦除：

```c
int x = 42;
void* p = &x;        // 类型信息丢了——编译器不知道 p 指向 int
int y = *(int*)p;    // 程序员手动把类型"恢复"回来
```

问题：`void*` 丢了类型后，**恢复类型的责任落在程序员身上**。一旦转错，编译期不报错，运行期崩溃。

C++ 的"类型擦除"做的是同一件事，但把类型恢复的责任从**程序员**转移到了**代码自动生成**上：

```
C 语言：
  存：程序员手动 &obj → (void*)
  取：程序员手动 (ObjType*)p → *p

C++ function_ref：
  存：模板自动生成代码 → 类型信息"烧进"了 invoke_ 函数里
  取：invoke_ 里已经硬编码了正确的类型转换 → 调用方不需要知道类型
```

所以 **"`void*` 丢了类型，`invoke_` 把类型恢复回来"** 的意思是：

- `obj_`（`void*`）保存数据地址，但类型丢了
- `invoke_`（函数指针）的函数体内**写死了正确的强制类型转换** —— 因为它是模板生成的，生成时就知道原始类型是什么

## 实现逐行拆解

### 第 1 步：声明一个能匹配"函数签名"的类名

```cpp
template <typename Signature>
class function_ref;  // 主模板，不定义（只是占位的壳）

template <typename R, typename... Args>
class function_ref<R(Args...)>;  // 偏特化，真正实现
```

**C 语言类比**：C 里函数指针写法是 `void (*)(int, double)`。C++ 想写得更像函数声明：`function_ref<void(int, double)>`。

`template<typename R, typename... Args>` 定义了两个占位符 —— `R` 填返回值类型，`Args...` 填参数类型列表。使用时：

```cpp
function_ref<void(int, double)>   // R=void,    Args={int, double}
function_ref<int(float, char*)>   // R=int,     Args={float, char*}
```

编译器会给每种组合生成一份独立的类代码（类似带类型的宏展开）。

### 第 2 步：两个成员变量

```cpp
private:
    void* obj_;                     // ← C 的 userdata：存数据地址
    R (*invoke_)(void*, Args...);   // ← C 的 callback：存调用函数指针
```

`invoke_` 的类型展开看：如果 `R=void, Args={int, double}`，它就是 `void (*)(void*, int, double)`。就是一个普通 C 函数指针，第一个参数固定为 `void*`（用来接收 `obj_`）。

### 第 3 步：构造函数 —— 接受任何可调用对象

```cpp
template <typename F>
function_ref(F&& f) noexcept
```

`template<typename F>` 告诉编译器："不管传进来的是什么类型，都帮我生成一个对应的构造函数"。

`F&&` 叫转发引用（forwarding reference）：

- 传入左值（有名字的变量）→ `F = T&`，`F&&` 展开为 `T&`（普通引用）
- 传入右值（临时对象）→ `F = T`，`F&&` 展开为 `T&&`（右值引用）

然后用 `using Fn = std::remove_reference_t<F>;` 去掉引用，得到纯类型（后面叫 `Fn`）。

### 第 4 步：分两路 —— 普通函数 vs 可调用对象

**为什么需要分两路？**

```cpp
void plainFunc(int);         // 普通函数：代码在代码段里，没有"对象"
auto lambda = [](int){};     // Lambda：在栈上有一个匿名对象
```

| | 普通函数 | Lambda / functor |
|---|---|---|
| 有对象实例吗？ | 没有，只有代码地址 | 有，存在栈上 |
| 能取 `&obj` 地址吗？ | 不能（没有 obj） | 能 |
| `sizeof` 有意义吗？ | 无意义 | 有意义 |

所以 `function_ref` 必须用 `if constexpr` 区分处理：

```cpp
using Fn = std::remove_reference_t<F>;

if constexpr (std::is_function_v<Fn>)   // "Fn 是函数类型吗？"
{
    // ========== 路径 A：传入的是普通函数 ==========
    // 例如 function_ref<void(int)> ref(plainFunction);
    //    F  = void (&)(int)     (引用到函数)
    //    Fn = void (int)        (函数类型)
    // 普通函数没有"对象"可以取地址 → 把函数地址本身塞进 obj_

    Fn* fnPtr = &f;                              // 取函数地址 → 函数指针值
    obj_ = reinterpret_cast<void*>(fnPtr);       // 存进 obj_
    // 此时 obj_ == 0x401000（函数在代码段的地址）

    invoke_ = +[](void* obj, Args... args) -> R {
        return reinterpret_cast<Fn*>(obj)(args...);
        //     ^^^^^^^^^^^^^^^^^^^^^^^^
        //     把 void* 当函数指针直接调用（不访问内存）
    };
}
else
{
    // ========== 路径 B：传入的是对象（Lambda/functor/std::function） ==========
    // 例如 function_ref<void(int)> ref(lambda);
    // 存对象的地址（不是对象本身）

    obj_ = ... std::addressof(f) ... ;   // &f → void*
    // 此时 obj_ == 0x7fff1234（lambda 对象在栈上的地址）

    invoke_ = +[](void* obj, Args... args) -> R {
        return (*static_cast<Fn*>(obj))(args...);
        //     ^^^^^^^^^^^^^^^^^^^^^^
        //     第1步：void* → Fn*（对象指针）
        //     第2步：* 解引用，拿到对象
        //     第3步：调用对象的 operator()
    };
}
```

两条路径的核心区别：

```
路径 A（函数）：
  obj_ 存的是"函数地址"本身（如 0x401000）
  调用时：reinterpret_cast<Fn*>(obj_) → 把这个值当作函数指针直接调用
  不访问 obj_ 指向的内存 —— obj_ 就是函数地址

路径 B（对象）：
  obj_ 存的是"对象的地址"（如 0x7fff1234）
  调用时：static_cast<Fn*>(obj_) → 得到对象指针 → *解引用 → 调用 operator()
  需要访问 obj_ 指向的内存
```

### 第 5 步：`+` 号 —— Lambda 转函数指针

```cpp
invoke_ = +[](void* obj, Args... args) -> R { ... };
//        ^
```

`invoke_` 的类型是 `R (*)(void*, Args...)` —— 一个普通 C 函数指针。
但 `[](...){...}` 创造的是一个 Lambda **对象**，不是函数指针。

**`+` 号强制把无捕获的 Lambda 转为函数指针**。等价于：

```cpp
// 不用 + 号的啰嗦写法：
using ThunkType = R (*)(void*, Args...);   // 定义函数指针类型
ThunkType thunk = [](void* obj, Args... args) -> R { ... };  // Lambda → 函数指针
invoke_ = thunk;
```

**为什么 Lambda 能自动变成函数指针？** `[]` 里为空（无捕获）的 Lambda 不含任何数据成员，编译器可以把它编译成一个普通函数，然后生成一个指向这个函数的指针。这是 C++ 标准规定的隐式转换。

### 第 6 步：调用 —— `operator()`

```cpp
R operator()(Args... args) const
{
    return invoke_(obj_, std::forward<Args>(args)...);
}
```

就是把保存的 `obj_` 作为第一个参数，加上用户传的参数，一起转发给 `invoke_`。

## 完整调用流程（C 语言等价代码）

以一个具体的 Lambda 为例：

```cpp
auto lambda = [](int x) { cout << x; };
function_ref<void(int)> ref(lambda);
ref(42);
```

编译器生成的大致等价代码（用 C 语言概念表示）：

```c
// —— 第 1 步：构造 function_ref ——
// 编译器分析 lambda 的类型，生成：
typedef /*匿名类型*/ LambdaType;  // LambdaType 有一个 void operator()(int)

// 构造时：
ref.obj_   = &lambda;              // 存 Lambda 对象的地址
ref.invoke_ = &thunk_for_lambda;   // 存编译时生成的 thunk 函数

// thunk_for_lambda 是编译器为这个 Lambda 类型特化生成的：
void thunk_for_lambda(void* obj, int arg) {
    LambdaType* ptr = (LambdaType*)obj;  // 把 void* 转回 LambdaType*
    (*ptr)(arg);                         // 调 operator()
}

// —— 第 2 步：调用 ref(42) ——
ref.operator()(42)
  → ref.invoke_(ref.obj_, 42)              // 调 thunk，把 obj_ 传回去
  → thunk_for_lambda(&lambda, 42)          // thunk 拿到原始 Lambda 的地址
  → (*(LambdaType*)obj)(42)                // 转回类型，调用
  → cout << 42                             // 最终效果
```

整个链路中，**类型信息 `LambdaType` 只在编译时存在于模板生成的代码中，运行时不携带**。这就是"类型擦除"的本质。

## ⚠️ 注意事项

`function_ref` 不拥有被引用的 callable。**被引用的 callable 必须在 `function_ref` 使用期间保持存活**，否则是悬空指针。

## 在 iceoryx 中

iceoryx 内部用 `function_ref` 传递 lambda 回调，对调用方透明。你不需要显式构造它，只需传递 lambda 即可。

## 运行

```bash
cmake --build build --target 02_function_ref
./build/bin/02_function_ref
```
