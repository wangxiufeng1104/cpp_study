#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// 用户表结构
typedef struct {
    int id;
    char name[64];
    int age;
    char email[128];
    double score;
} User;

// 打印错误信息
void print_error(sqlite3 *db, const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    if (db) {
        fprintf(stderr, "SQLite Error: %s\n", sqlite3_errmsg(db));
    }
}

// 分析 SQL 语句的参数
void analyze_sql_parameters(sqlite3 *db, const char *sql) {
    printf("\n=== SQL Parameter Analysis ===\n");
    printf("SQL: %s\n\n", sql);
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare SQL: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    // 获取参数总数
    int param_count = sqlite3_bind_parameter_count(stmt);
    printf("Total parameters: %d\n\n", param_count);
    
    if (param_count == 0) {
        printf("No parameters found in SQL statement.\n");
    } else {
        printf("%-10s %-15s %-20s\n", "Index", "Type", "Name");
        printf("-------------------------------------------------\n");
        
        for (int i = 1; i <= param_count; i++) {
            const char *param_name = sqlite3_bind_parameter_name(stmt, i);
            const char *param_type;
            
            // 判断参数类型
            if (param_name == NULL) {
                param_type = "Positional (?)";
            } else if (strncmp(param_name, ":", 1) == 0) {
                param_type = "Named (:name)";
            } else if (strncmp(param_name, "@", 1) == 0) {
                param_type = "Named (@name)";
            } else if (strncmp(param_name, "$", 1) == 0) {
                param_type = "Named ($name)";
            } else {
                param_type = "Unknown";
            }
            
            printf("%-10d %-15s %-20s\n", i, param_type, 
                   param_name ? param_name : "(unnamed)");
        }
    }
    
    printf("\n");
    sqlite3_finalize(stmt);
}

// 动态执行 SQL 语句 - 使用位置参数
int execute_dynamic_sql_positional(sqlite3 *db, const char *sql, User *user) {
    printf("\n=== Executing SQL with Positional Parameters ===\n");
    printf("SQL: %s\n\n", sql);
    
    // 先分析参数
    analyze_sql_parameters(db, sql);
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    // 根据参数索引绑定值
    // 参数索引从 1 开始
    printf("Binding parameters:\n");
    printf("  [1] id: %d (int)\n", user->id);
    sqlite3_bind_int(stmt, 1, user->id);
    
    printf("  [2] name: %s (text)\n", user->name);
    sqlite3_bind_text(stmt, 2, user->name, -1, SQLITE_TRANSIENT);
    
    printf("  [3] age: %d (int)\n", user->age);
    sqlite3_bind_int(stmt, 3, user->age);
    
    printf("  [4] email: %s (text)\n", user->email);
    sqlite3_bind_text(stmt, 4, user->email, -1, SQLITE_TRANSIENT);
    
    printf("  [5] score: %.2f (double)\n", user->score);
    sqlite3_bind_double(stmt, 5, user->score);
    
    printf("\nExecuting...\n");
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    printf("Success! Row inserted with ID: %lld\n", (long long)sqlite3_last_insert_rowid(db));
    
    sqlite3_finalize(stmt);
    return 0;
}

// 动态执行 SQL 语句 - 使用命名参数
int execute_dynamic_sql_named(sqlite3 *db, const char *sql, User *user) {
    printf("\n=== Executing SQL with Named Parameters ===\n");
    printf("SQL: %s\n\n", sql);
    
    // 先分析参数
    analyze_sql_parameters(db, sql);
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return -1;
    }
    
    // 根据参数名称绑定值
    printf("Binding parameters:\n");
    
    int idx;
    
    idx = sqlite3_bind_parameter_index(stmt, ":id");
    if (idx > 0) {
        printf("  [:%s -> %d] id: %d (int)\n", 
               "id", idx, user->id);
        sqlite3_bind_int(stmt, idx, user->id);
    }
    
    idx = sqlite3_bind_parameter_index(stmt, ":name");
    if (idx > 0) {
        printf("  [:%s -> %d] name: %s (text)\n", 
               "name", idx, user->name);
        sqlite3_bind_text(stmt, idx, user->name, -1, SQLITE_TRANSIENT);
    }
    
    idx = sqlite3_bind_parameter_index(stmt, ":age");
    if (idx > 0) {
        printf("  [:%s -> %d] age: %d (int)\n", 
               "age", idx, user->age);
        sqlite3_bind_int(stmt, idx, user->age);
    }
    
    idx = sqlite3_bind_parameter_index(stmt, ":email");
    if (idx > 0) {
        printf("  [:%s -> %d] email: %s (text)\n", 
               "email", idx, user->email);
        sqlite3_bind_text(stmt, idx, user->email, -1, SQLITE_TRANSIENT);
    }
    
    idx = sqlite3_bind_parameter_index(stmt, ":score");
    if (idx > 0) {
        printf("  [:%s -> %d] score: %.2f (double)\n", 
               "score", idx, user->score);
        sqlite3_bind_double(stmt, idx, user->score);
    }
    
    printf("\nExecuting...\n");
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute statement");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    printf("Success! Row inserted with ID: %lld\n", (long long)sqlite3_last_insert_rowid(db));
    
    sqlite3_finalize(stmt);
    return 0;
}

// 动态查询
void dynamic_query(sqlite3 *db, const char *sql, const char *where_name) {
    printf("\n=== Dynamic Query ===\n");
    printf("SQL Template: %s\n", sql);
    printf("Parameter: %s\n\n", where_name);
    
    // 分析参数
    analyze_sql_parameters(db, sql);
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return;
    }
    
    // 绑定查询参数
    printf("Binding parameter:\n");
    printf("  [1] name: %s (text)\n", where_name);
    sqlite3_bind_text(stmt, 1, where_name, -1, SQLITE_TRANSIENT);
    
    printf("\nExecuting...\n");
    printf("\n%-10s %-20s %-10s %-30s %-10s\n", 
           "ID", "Name", "Age", "Email", "Score");
    printf("-----------------------------------------------------------------\n");
    
    int count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const unsigned char *email = sqlite3_column_text(stmt, 3);
        double score = sqlite3_column_double(stmt, 4);
        
        printf("%-10d %-20s %-10d %-30s %-10.2f\n", 
               id, name, age, email, score);
        count++;
    }
    
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute query");
    }
    
    printf("\nTotal rows: %d\n", count);
    sqlite3_finalize(stmt);
}

// 显示所有用户
void display_all_users(sqlite3 *db) {
    printf("\n=== Display All Users ===\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name, age, email, score FROM users ORDER BY id";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        print_error(db, "Failed to prepare statement");
        return;
    }
    
    printf("\n%-10s %-20s %-10s %-30s %-10s\n", 
           "ID", "Name", "Age", "Email", "Score");
    printf("-----------------------------------------------------------------\n");
    
    int count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const unsigned char *email = sqlite3_column_text(stmt, 3);
        double score = sqlite3_column_double(stmt, 4);
        
        printf("%-10d %-20s %-10d %-30s %-10.2f\n", 
               id, name, age, email, score);
        count++;
    }
    
    if (rc != SQLITE_DONE) {
        print_error(db, "Failed to execute query");
    }
    
    printf("\nTotal users: %d\n", count);
    sqlite3_finalize(stmt);
}

// 创建用户表
int create_users_table(sqlite3 *db) {
    const char *sql = "DROP TABLE IF EXISTS users;"
                      "CREATE TABLE users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "age INTEGER,"
                      "email TEXT,"
                      "score REAL"
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
    
    printf("Dynamic SQL Demo\n");
    printf("================\n");
    printf("SQLite Version: %s\n\n", sqlite3_libversion());
    
    // 创建内存数据库
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    // 创建用户表
    if (create_users_table(db) != 0) {
        sqlite3_close(db);
        return 1;
    }
    printf("Users table created successfully.\n");
    
    // 测试数据
    User users[] = {
        {101, "Alice", 28, "alice@example.com", 95.5},
        {102, "Bob", 32, "bob@example.com", 87.3},
        {103, "Charlie", 25, "charlie@example.com", 92.1},
        {104, "David", 35, "david@example.com", 88.7},
        {105, "Eve", 29, "eve@example.com", 91.2}
    };
    
    // 演示1: 使用位置参数插入数据
    printf("\n========================================\n");
    printf("Demo 1: Positional Parameters (?)\n");
    printf("========================================\n");
    
    for (int i = 0; i < 3; i++) {
        char sql[256];
        sprintf(sql, "INSERT INTO users (id, name, age, email, score) VALUES (?, ?, ?, ?, ?)");
        execute_dynamic_sql_positional(db, sql, &users[i]);
    }
    
    // 演示2: 使用命名参数插入数据
    printf("\n========================================\n");
    printf("Demo 2: Named Parameters (:name)\n");
    printf("========================================\n");
    
    for (int i = 3; i < 5; i++) {
        char sql[256];
        sprintf(sql, "INSERT INTO users (id, name, age, email, score) VALUES (:id, :name, :age, :email, :score)");
        execute_dynamic_sql_named(db, sql, &users[i]);
    }
    
    // 显示所有用户
    display_all_users(db);
    
    // 演示3: 动态查询
    printf("\n========================================\n");
    printf("Demo 3: Dynamic Query\n");
    printf("========================================\n");
    
    const char *query_sql = "SELECT id, name, age, email, score FROM users WHERE name = ?";
    dynamic_query(db, query_sql, "Alice");
    
    printf("\n");
    dynamic_query(db, query_sql, "Bob");
    
    printf("\n");
    dynamic_query(db, query_sql, "NonExistentUser");
    
    // 演示4: 混合参数
    printf("\n========================================\n");
    printf("Demo 4: Mixed Parameters\n");
    printf("========================================\n");
    
    const char *mixed_sql = "SELECT * FROM users WHERE age > ? AND score > :min_score";
    analyze_sql_parameters(db, mixed_sql);
    
    printf("\nThis demonstrates that SQLite supports both positional (?) and named (:name) parameters in same statement.\n");
    
    // 关闭数据库
    sqlite3_close(db);
    
    printf("\n========================================\n");
    printf("All demos completed!\n");
    printf("========================================\n");
    return 0;
}