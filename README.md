# C++ Code Validation

C++ 学习与验证项目，涵盖基础语法、STL、设计模式、开源库集成等。

## 构建

项目使用 CMake (>= 3.16)，要求 C++17。

```bash
# 配置（在项目根目录执行）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 构建全部目标
cmake --build build

# 构建指定目标
cmake --build build --target <target_name>

# 例如：构建 iceoryx_prereq 所有子目标
cmake --build build --target 01_lambda --target 02_function_ref --target 03_crtp --target 04_expected --target 05_monadic --target 06_placement_new
```

编译产物输出到 `build/bin/`。

## 运行

```bash
./build/bin/<target_name>
```

## 清理

```bash
# 清理构建产物（保留 CMake 缓存）
cmake --build build --target clean

# 完全清理（删除整个 build 目录）
rm -rf build
```

重新构建只需再次执行配置 + 构建即可：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
