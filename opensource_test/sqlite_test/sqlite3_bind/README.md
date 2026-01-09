# SQLite3 Bind Functions Test

本目录包含 SQLite3 参数绑定（bind）系列函数的测试代码。

## 概述

SQLite3 提供了多种参数绑定函数，用于将应用程序中的数据安全地绑定到 SQL 语句的参数占位符（?）上。这些函数可以防止 SQL 注入，提高代码安全性。

## 测试的函数

本测试程序验证以下 `sqlite3_bind` 系列函数：

### 基本类型
- **sqlite3_bind_null** - 绑定 NULL 值
- **sqlite3_bind_int** - 绑定 32 位整数
- **sqlite3_bind_int64** - 绑定 64 位整数
- **sqlite3_bind_double** - 绑定浮点数

### 字符串和二进制
- **sqlite3_bind_text** - 绑定字符串
- **sqlite3_bind_blob** - 绑定二进制数据
- **sqlite3_bind_zeroblob** - 绑定零填充的 BLOB（占位符）

### 高级类型
- **sqlite3_bind_value** - 绑定 sqlite3_value 对象
- **sqlite3_bind_pointer** - 绑定指针（SQLite 3.20.0+）

### 错误处理
- 测试无效参数索引（SQLITE_RANGE）
- 测试 NULL 语句对象（SQLITE_MISUSE）

## 测试输出示例

```
SQLite3 Bind Functions Test
============================
SQLite Version: 3.46.0

=== Testing sqlite3_bind_null ===
sqlite3_bind_null test passed

=== Testing sqlite3_bind_int ===
sqlite3_bind_int test passed (value: 12345)

=== Testing sqlite3_bind_int64 ===
sqlite3_bind_int64 test passed (value: 9876543210)

=== Testing sqlite3_bind_double ===
sqlite3_bind_double test passed (value: 3.141593)

=== Testing sqlite3_bind_text ===
sqlite3_bind_text test passed (value: "Hello, SQLite3!")

=== Testing sqlite3_bind_blob ===
sqlite3_bind_blob test passed (size: 8 bytes)

=== Testing sqlite3_bind_zeroblob ===
sqlite3_bind_zeroblob test passed (size: 1024 bytes)

=== Testing sqlite3_bind_value ===
sqlite3_bind_value test passed

=== Testing sqlite3_bind_pointer ===
sqlite3_bind_pointer test passed

=== Testing bind error handling ===
Error handling tests passed

=== Query Results ===
INT         INT64           DOUBLE     TEXT                 NULL     
------------------------------------------------------------
100         0               0.0000     test null            NULL     
12345       0               0.0000     test int             NOT NULL 
0           9876543210      0.0000     test int64           NOT NULL 
0           0               3.1416     test double          NOT NULL 
200         0               0.0000     Hello, SQLite3!      NOT NULL 
300         0               0.0000     NULL                 NOT NULL 
400         0               0.0000     NULL                 NOT NULL 
500         0               0.0000     copied value         NOT NULL 

=== All tests completed ===
```

## 技术细节

### 内存管理参数

`sqlite3_bind_*` 函数的第 5 个参数（对于 text 和 blob）控制内存管理：

- **SQLITE_STATIC**: 告诉 SQLite 该指针在语句执行期间保持有效，SQLite 不会复制数据
- **SQLITE_TRANSIENT**: 告诉 SQLite 复制数据，原始数据可以立即释放
- **自定义析构函数指针**: 提供自定义的内存释放函数

### 参数索引

- 参数索引从 1 开始（不是 0）
- 第一个问号 (?) 的索引为 1
- 使用 `sqlite3_bind_parameter_index()` 可以通过名称获取命名参数的索引

### 错误码

- **SQLITE_OK** (0) - 成功
- **SQLITE_RANGE** - 参数索引超出范围
- **SQLITE_NOMEM** - 内存分配失败
- **SQLITE_MISUSE** - 错误使用 API（如传递 NULL 指针）

## 数据库

本程序使用内存数据库（`:memory:`），不会创建持久化文件。测试完成后所有数据自动销毁。

## 使用到的 SQLite3 API 接口

### 数据库连接
- **sqlite3_open(const char *filename, sqlite3 **ppDb)**
  - 打开数据库连接
  - 使用 `:memory:` 创建内存数据库
  - 返回 SQLITE_OK 表示成功

- **sqlite3_close(sqlite3 *db)**
  - 关闭数据库连接
  - 释放相关资源

### SQL 语句准备与执行
- **sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail)**
  - 编译 SQL 语句为准备语句（prepared statement）
  - `zSql`: SQL 语句字符串
  - `nByte`: SQL 语句长度，-1 表示自动计算到第一个 '\0'
  - `ppStmt`: 输出准备语句指针
  - 返回 SQLITE_OK 表示成功

- **sqlite3_step(sqlite3_stmt *pStmt)**
  - 执行准备语句
  - 返回 SQLITE_ROW 表示有数据行
  - 返回 SQLITE_DONE 表示执行完成

- **sqlite3_finalize(sqlite3_stmt *pStmt)**
  - 销毁准备语句
  - 释放相关资源

### 绑定参数接口
- **sqlite3_bind_null(sqlite3_stmt *pStmt, int i)**
  - 将参数绑定为 NULL
  - `i`: 参数索引（从 1 开始）

- **sqlite3_bind_int(sqlite3_stmt *pStmt, int i, int iValue)**
  - 绑定 32 位整数值
  - `iValue`: 要绑定的整数值

- **sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, sqlite3_int64 iValue)**
  - 绑定 64 位整数值
  - `iValue`: 要绑定的 64 位整数值

- **sqlite3_bind_double(sqlite3_stmt *pStmt, int i, double rValue)**
  - 绑定浮点数值
  - `rValue`: 要绑定的浮点数值

- **sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void (*xDel)(void *))**
  - 绑定字符串
  - `zData`: 字符串指针
  - `nData`: 字符串长度，-1 表示自动计算
  - `xDel`: 析构函数，SQLITE_STATIC 或 SQLITE_TRANSIENT

- **sqlite3_bind_blob(sqlite3_stmt *pStmt, int i, const void *zData, int nData, void (*xDel)(void *))**
  - 绑定二进制数据
  - `zData`: 二进制数据指针
  - `nData`: 数据长度
  - `xDel`: 析构函数

- **sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, int i, int n)**
  - 绑定零填充的 BLOB
  - `n`: BLOB 大小（字节数）
  - 适合预留空间，后续可以更新

- **sqlite3_bind_value(sqlite3_stmt *pStmt, int i, const sqlite3_value *pValue)**
  - 绑定 sqlite3_value 对象
  - `pValue`: 从查询结果中获取的值对象
  - 用于值的复制和传递

- **sqlite3_bind_pointer(sqlite3_stmt *pStmt, int i, void *pPtr, const char *zType, void (*xDestructor)(void *))**
  - 绑定指针（SQLite 3.20.0+）
  - `pPtr`: 指针值
  - `zType`: 类型标识字符串
  - 注意：不会实际存储到数据库，用于应用内部传递

### 查询结果获取
- **sqlite3_column_count(sqlite3_stmt *pStmt)**
  - 获取结果列数

- **sqlite3_column_type(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的数据类型
  - 返回 SQLITE_NULL, SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB 等

- **sqlite3_column_int(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的整数值

- **sqlite3_column_int64(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的 64 位整数值

- **sqlite3_column_double(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的浮点数值

- **sqlite3_column_text(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的文本值
  - 返回 const unsigned char* 指针

- **sqlite3_column_blob(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的 BLOB 值
  - 返回 const void* 指针

- **sqlite3_column_bytes(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的数据长度

- **sqlite3_column_value(sqlite3_stmt *pStmt, int iCol)**
  - 获取列的 sqlite3_value 对象
  - 用于 sqlite3_bind_value 传递

### 错误处理
- **sqlite3_errcode(sqlite3 *db)**
  - 获取最近的错误码

- **sqlite3_errmsg(sqlite3 *db)**
  - 获取最近的错误信息描述
  - 返回字符串指针，在下次 SQLite API 调用前有效

### 直接执行 SQL
- **sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg)**
  - 直接执行 SQL 语句
  - 适合不返回数据的操作（如 CREATE, DROP, INSERT）
  - `callback`: 回调函数，可为 NULL
  - `errmsg`: 输出错误信息，需要用 sqlite3_free() 释放

### 其他实用函数
- **sqlite3_libversion(void)**
  - 获取 SQLite 库版本号字符串

- **sqlite3_free(void *ptr)**
  - 释放 SQLite 分配的内存
  - 用于释放 sqlite3_exec 返回的错误信息

- **sqlite3_int64**
  - 64 位整数类型定义
  - 通常为 `long long` 或 `int64_t`

## 参考资料

- [SQLite 官方文档 - Binding Values To Prepared Statements](https://www.sqlite.org/c3ref/bind_blob.html)
- [SQLite Parameter Binding Guide](https://www.sqlite.org/cintro.html#binding_parameters)
- [SQLite C Interface Reference](https://www.sqlite.org/c3ref/intro.html)
