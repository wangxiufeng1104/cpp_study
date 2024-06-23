
# 3.4 命令参数
# 3.4.1 引号参数
# 引号参数
message("CMake 
你好！")
# 引号参数（避免换行）
message("\
CMake\
你好！\
")
# 3.4.2 非引号参数
#[[非引号参数自然指未被引号包裹的参数。
    这种参数中不能包含任何空白符，也不能包含圆括号、#符号、双引号和反斜杠，除非经过转义。
    非引号参数也支持变量引用和转义字符。]]
message("x;y;z") # 引号参数
message(x y z) #  多个非引号参数
message(x;y;z) # 非引号参数，实际当作列表语法传入
#[[打印结果
x;y;z
xyz
xyz]]
# 3.4.3 变量引用
set(var_a 你好)
set(var_b a)
message(${var_${var_b}})

# 3.4.4 转义字符
cmake_minimum_required(VERSION 3.20)
set("a?b" "变量a?b")
# \?转义为？
message(${a\?b})
set("a;b" "变量a;b")
# 变量引用中\;转义为;
message("${a\;b}")





