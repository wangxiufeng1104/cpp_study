#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// 用于测试的自定义数据
typedef struct {
    int id;
    char name[32];
} TestPointer;

// 打印错误信息
void print_error(sqlite3 *db, const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    if (db) {
        fprintf(stderr, "SQLite Error: %s\n", sqlite3_errmsg(db));
    }
}

// 测试 sqlite3_bind_null
int test_bind_null(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_null ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_int, col_text, col_null) VALUES (?, ?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    // 绑定参数
    sqlite3_bind_int(stmt, 1, 100);
    sqlite3_bind_text(stmt, 2, "test null", -1, SQLITE_STATIC);
    sqlite3_bind_null(stmt, 3);  // 绑定 NULL
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_null test passed\n");
    return 0;
}

// 测试 sqlite3_bind_int
int test_bind_int(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_int ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_int, col_text) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    int test_value = 12345;
    sqlite3_bind_int(stmt, 1, test_value);
    sqlite3_bind_text(stmt, 2, "test int", -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_int test passed (value: %d)\n", test_value);
    return 0;
}

// 测试 sqlite3_bind_int64
int test_bind_int64(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_int64 ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_int64, col_text) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    sqlite3_int64 test_value = 9876543210LL;
    sqlite3_bind_int64(stmt, 1, test_value);
    sqlite3_bind_text(stmt, 2, "test int64", -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_int64 test passed (value: %lld)\n", (long long)test_value);
    return 0;
}

// 测试 sqlite3_bind_double
int test_bind_double(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_double ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_double, col_text) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    double test_value = 3.1415926;
    sqlite3_bind_double(stmt, 1, test_value);
    sqlite3_bind_text(stmt, 2, "test double", -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_double test passed (value: %f)\n", test_value);
    return 0;
}

// 测试 sqlite3_bind_text
int test_bind_text(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_text ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_text, col_int) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    const char *test_text = "Hello, SQLite3!";
    // SQLITE_TRANSIENT: SQLite 会复制字符串
    sqlite3_bind_text(stmt, 1, test_text, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, 200);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_text test passed (value: \"%s\")\n", test_text);
    return 0;
}

// 测试 sqlite3_bind_blob
int test_bind_blob(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_blob ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_blob, col_int) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    unsigned char blob_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFE, 0xFD};
    sqlite3_bind_blob(stmt, 1, blob_data, sizeof(blob_data), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, 300);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_blob test passed (size: %zu bytes)\n", sizeof(blob_data));
    return 0;
}

// 测试 sqlite3_bind_zeroblob
int test_bind_zeroblob(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_zeroblob ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_blob, col_int) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    // 创建一个 1024 字节的零填充 blob
    int blob_size = 1024;
    sqlite3_bind_zeroblob(stmt, 1, blob_size);
    sqlite3_bind_int(stmt, 2, 400);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_zeroblob test passed (size: %d bytes)\n", blob_size);
    return 0;
}

// 测试 sqlite3_bind_value
int test_bind_value(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_value ===\n");
    
    // 首先创建一个 sqlite3_value
    sqlite3_stmt *stmt1;
    const char *sql1 = "SELECT 'copied value'";
    
    int rc = sqlite3_prepare_v2(db, sql1, -1, &stmt1, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement 1");
        return -1;
    }
    
    rc = sqlite3_step(stmt1);
    if (rc != SQLITE_ROW) {
        print_error(db, "Failed to step statement 1");
        sqlite3_finalize(stmt1);
        return -1;
    }
    
    // 获取 sqlite3_value 对象
    sqlite3_value *value = sqlite3_column_value(stmt1, 0);
    
    // 使用 sqlite3_bind_value 绑定
    sqlite3_stmt *stmt2;
    const char *sql2 = "INSERT INTO bind_test (col_text, col_int) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql2, -1, &stmt2, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement 2");
        sqlite3_finalize(stmt1);
        return -1;
    }
    
    sqlite3_bind_value(stmt2, 1, value);
    sqlite3_bind_int(stmt2, 2, 500);
    
    rc = sqlite3_step(stmt2);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement 2");
        sqlite3_finalize(stmt1);
        sqlite3_finalize(stmt2);
        return -1;
    }
    
    sqlite3_finalize(stmt1);
    sqlite3_finalize(stmt2);
    printf("sqlite3_bind_value test passed\n");
    return 0;
}

// 测试 sqlite3_bind_pointer
int test_bind_pointer(sqlite3 *db) {
    printf("\n=== Testing sqlite3_bind_pointer ===\n");
    
    // SQLite 3.20.0+ 支持 pointer 类型
#if SQLITE_VERSION_NUMBER >= 3020000
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_int, col_text) VALUES (?, ?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    TestPointer ptr_data = {600, "pointer test"};
    
    // 注意：pointer 类型不会实际存储到数据库中
    // 它主要用于应用程序内部传递指针
    sqlite3_bind_pointer(stmt, 2, &ptr_data, "test_pointer", NULL);
    sqlite3_bind_int(stmt, 1, ptr_data.id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("sqlite3_bind_pointer test passed\n");
#else
    printf("sqlite3_bind_pointer requires SQLite 3.20.0 or higher (skipped)\n");
#endif
    return 0;
}

// 测试错误处理
int test_bind_error_handling(sqlite3 *db) {
    printf("\n=== Testing bind error handling ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO bind_test (col_int) VALUES (?)";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    // 测试无效的参数索引
    rc = sqlite3_bind_int(stmt, 999, 100);  // 无效的索引
    if (rc != SQLITE_RANGE) {
        printf("Error: Expected SQLITE_RANGE for invalid index\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    // 测试 NULL stmt
    rc = sqlite3_bind_int(NULL, 1, 100);
    if (rc != SQLITE_MISUSE) {
        printf("Error: Expected SQLITE_MISUSE for NULL stmt\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    printf("Error handling tests passed\n");
    return 0;
}

// 查询并显示数据
void query_and_display(sqlite3 *db) {
    printf("\n=== Query Results ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "SELECT col_int, col_int64, col_double, col_text, col_null FROM bind_test LIMIT 10";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare query");
        return;
    }
    
    printf("%-10s %-15s %-10s %-20s %-10s\n", "INT", "INT64", "DOUBLE", "TEXT", "NULL");
    printf("------------------------------------------------------------\n");
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int col_int = sqlite3_column_int(stmt, 0);
        sqlite3_int64 col_int64 = sqlite3_column_int64(stmt, 1);
        double col_double = sqlite3_column_double(stmt, 2);
        const unsigned char *col_text = sqlite3_column_text(stmt, 3);
        int col_null = sqlite3_column_type(stmt, 4);
        
        printf("%-10d %-15lld %-10.4f %-20s %-10s\n", 
               col_int, 
               (long long)col_int64, 
               col_double, 
               col_text ? (char*)col_text : "NULL",
               col_null == SQLITE_NULL ? "NULL" : "NOT NULL");
    }
    
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute query");
    }
    
    sqlite3_finalize(stmt);
}

// 创建测试表
int create_test_table(sqlite3 *db) {
    const char *sql = "DROP TABLE IF EXISTS bind_test;"
                      "CREATE TABLE bind_test ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "col_int INTEGER,"
                      "col_int64 INTEGER,"
                      "col_double REAL,"
                      "col_text TEXT,"
                      "col_blob BLOB,"
                      "col_null TEXT"
                      ");";
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    return 0;
}

int main(void) {
    sqlite3 *db;
    int rc;
    
    printf("SQLite3 Bind Functions Test\n");
    printf("============================\n");
    printf("SQLite Version: %s\n\n", sqlite3_libversion());
    
    // 创建内存数据库
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    // 创建测试表
    if (create_test_table(db) != 0) {
        sqlite3_close(db);
        return 1;
    }
    
    // 运行所有测试
    test_bind_null(db);
    test_bind_int(db);
    test_bind_int64(db);
    test_bind_double(db);
    test_bind_text(db);
    test_bind_blob(db);
    test_bind_zeroblob(db);
    test_bind_value(db);
    test_bind_pointer(db);
    test_bind_error_handling(db);
    
    // 查询并显示结果
    query_and_display(db);
    
    // 关闭数据库
    sqlite3_close(db);
    
    printf("\n=== All tests completed ===\n");
    return 0;
}