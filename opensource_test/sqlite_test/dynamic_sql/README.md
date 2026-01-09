# Dynamic SQL Demo

本目录演示如何在 SQLite 中使用动态 SQL 和参数化查询。

## 概述

动态 SQL 是指在运行时构建和执行的 SQL 语句。通过使用参数占位符，可以安全地构建动态 SQL，避免 SQL 注入攻击。

## 主要功能

本程序演示了以下动态 SQL 技术：

### 1. 位置参数 (?)

使用问号 `?` 作为参数占位符，参数按位置顺序绑定。

```c
const char *sql = "INSERT INTO users (id, name, age, email, score) VALUES (?, ?, ?, ?, ?)";
sqlite3_bind_int(stmt, 1, user->id);
sqlite3_bind_text(stmt, 2, user->name, -1, SQLITE_TRANSIENT);
// ... 等等
```

**优点：**
- 语法简单
- 适合参数较少的情况
- 在大多数数据库中通用

**缺点：**
- 参数顺序必须与占位符顺序一致
- 代码可读性较差，需要记住每个位置的含义

### 2. 命名参数 (:name, @name, $name)

使用带前缀的名称作为参数占位符，参数按名称绑定。

```c
const char *sql = "INSERT INTO users (id, name, age, email, score) VALUES (:id, :name, :age, :email, :score)";
int idx = sqlite3_bind_parameter_index(stmt, ":id");
sqlite3_bind_int(stmt, idx, user->id);
idx = sqlite3_bind_parameter_index(stmt, ":name");
sqlite3_bind_text(stmt, idx, user->name, -1, SQLITE_TRANSIENT);
// ... 等等
```

**支持的命名参数前缀：**
- `:` - 冒号（最常用）
- `@` - @ 符号（T-SQL 风格）
- `$` - 美元符号（MySQL/PostgreSQL 风格）

**优点：**
- 代码可读性高，参数名称清晰
- 参数顺序不重要
- 适合复杂 SQL 和大量参数的情况

**缺点：**
- 需要额外的步骤获取参数索引
- 语法稍显冗长

### 3. 混合参数

SQLite 允许在同一个 SQL 语句中混合使用位置参数和命名参数。

```c
const char *sql = "SELECT * FROM users WHERE age > ? AND score > :min_score";
```

### 4. SQL 参数分析器

提供 `analyze_sql_parameters()` 函数，可以在运行时分析 SQL 语句中的参数：

```c
void analyze_sql_parameters(sqlite3 *db, const char *sql);
```

**功能：**
- 识别所有参数占位符
- 显示参数总数、索引、类型和名称
- 自动区分位置参数和命名参数

**输出示例：**
```
=== SQL Parameter Analysis ===
SQL: INSERT INTO users (id, name, age, email, score) VALUES (:id, :name, :age, :email, :score)

Total parameters: 5

Index      Type            Name
-------------------------------------------------
1          Named (:name)   :id
2          Named (:name)   :name
3          Named (:name)   :age
4          Named (:name)   :email
5          Named (:name)   :score
```

## 数据库结构

本程序使用内存数据库和以下表结构：

```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    age INTEGER,
    email TEXT,
    score REAL
);
```

## 使用到的 SQLite3 API

### 参数分析

- **sqlite3_bind_parameter_count(sqlite3_stmt *pStmt)**
  - 获取 SQL 语句中的参数总数
  - 返回参数个数（从 0 开始，如果没有参数则为 0）

- **sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, int i)**
  - 获取指定索引参数的名称
  - `i`: 参数索引（从 1 开始）
  - 返回参数名字符串，如果没有名称则返回 NULL

- **sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName)**
  - 根据参数名称获取参数索引
  - `zName`: 参数名称（如 `:name`）
  - 返回参数索引（从 1 开始），如果未找到则返回 0

### 参数绑定

- **sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue)**
  - 绑定整数值
  - `i`: 参数索引（从 1 开始）

- **sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void (*xDel)(void *))**
  - 绑定字符串
  - `zData`: 字符串指针
  - `nData`: 字符串长度，-1 表示自动计算
  - `xDel`: 内存管理函数（SQLITE_STATIC 或 SQLITE_TRANSIENT）

- **sqlite3_bind_double(sqlite3_stmt *pStmt, int i, double rValue)**
  - 绑定浮点数值

### 执行和结果

- **sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail)**
  - 准备 SQL 语句
  - 返回准备好的语句对象

- **sqlite3_step(sqlite3_stmt *pStmt)**
  - 执行准备语句
  - 返回 SQLITE_ROW（有结果行）或 SQLITE_DONE（执行完成）

- **sqlite3_column_int(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的整数值

- **sqlite3_column_text(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的文本值

- **sqlite3_column_double(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的浮点数值

- **sqlite3_last_insert_rowid(sqlite3 *db)**
  - 获取最后一次插入的行 ID

## 演示内容

程序包含以下演示：

1. **位置参数插入** - 使用 `?` 占位符插入数据
2. **命名参数插入** - 使用 `:name` 占位符插入数据
3. **动态查询** - 使用参数化查询查找特定用户
4. **混合参数** - 演示同时使用位置和命名参数

## 运行程序

```bash
cd build
make dynamic_sql
cd Release/bin
./dynamic_sql
```

## 示例输出

```
Dynamic SQL Demo
================
SQLite Version: 3.46.1

Users table created successfully.

========================================
Demo 1: Positional Parameters (?)
========================================

=== Executing SQL with Positional Parameters ===
SQL: INSERT INTO users (id, name, age, email, score) VALUES (?, ?, ?, ?, ?)

=== SQL Parameter Analysis ===
SQL: INSERT INTO users (id, name, age, email, score) VALUES (?, ?, ?, ?, ?)

Total parameters: 5

Index      Type            Name
-------------------------------------------------
1          Positional (?)  (unnamed)
2          Positional (?)  (unnamed)
3          Positional (?)  (unnamed)
4          Positional (?)  (unnamed)
5          Positional (?)  (unnamed)

Binding parameters:
  [1] id: 101 (int)
  [2] name: Alice (text)
  [3] age: 28 (int)
  [4] email: alice@example.com (text)
  [5] score: 95.50 (double)

Executing...
Success! Row inserted with ID: 101

[... 更多输出 ...]
```

## 最佳实践

### 安全性

- **永远不要使用字符串拼接构建 SQL**
  ```c
  // 危险！容易受到 SQL 注入攻击
  char sql[256];
  sprintf(sql, "SELECT * FROM users WHERE name = '%s'", user_input);
  ```

- **始终使用参数化查询**
  ```c
  // 安全！
  const char *sql = "SELECT * FROM users WHERE name = ?";
  sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_TRANSIENT);
  ```

### 性能

- 重用准备语句（prepared statement）以提高性能
- 对于重复执行的语句，准备一次，执行多次

### 代码可读性

- 对于复杂 SQL，优先使用命名参数
- 使用参数名称时，保持命名一致性

## 应用场景

动态 SQL 适用于：

1. **用户输入处理** - 表单提交、搜索查询
2. **条件查询** - 根据用户选择动态构建 WHERE 子句
3. **批量操作** - 循环插入或更新数据
4. **动态报表** - 根据用户选择生成不同的查询
5. **ORM 框架** - 对象关系映射库通常使用参数化查询

## 注意事项

1. **参数索引从 1 开始**，不是 0
2. **命名参数前缀** 必须一致，`:name` 和 `:NAME` 是不同的参数
3. **内存管理** - 使用 SQLITE_TRANSIENT 或自定义析构函数正确管理内存
4. **参数类型** - 确保 bind 函数的数据类型与列类型匹配
5. **错误处理** - 始终检查 SQLite API 的返回值

## 参考资料

- [SQLite Parameter Binding](https://www.sqlite.org/c3ref/bind_blob.html)
- [SQLite C Interface Reference](https://www.sqlite.org/c3ref/intro.html)
- [SQLite Query Planning](https://www.sqlite.org/queryplanner.html)