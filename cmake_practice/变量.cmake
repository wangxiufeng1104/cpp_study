# 3.5.1 预定义变量
message("CMake命令行：${CMAKE_COMMAND}")
message("OS:${CMAKE_HOST_SYSTEM_NAME}")
#[[ 执行结果如下：
CMake命令行：/usr/bin/cmake
OS:Linux]]


# 3.5.2 定义变量
function(f)
    set(a "我是修改后的a")
    set(b "我是b")
    set(c "我是c" PARENT_SCOPE)
endfunction(f)

set(a "我是a")
f()
message("a : ${a}")
message("b : ${b}")
message("c : ${c}")

# 3.11 缓存变量
set(a 缓存变量 CACHE STRING "")
set(a 普通变量)
message("\${a}:${a}")
message("\$CACHE{a}:$CACHE{a}")
# 3.15 列表
# include(print_list.cmake)
# set(a "a;b\;c")
# set(b "a[;]b;c")
# set(c "a[[[;]]]b;c")
# set(d "a[;b;c")
# set(e "a[];b")
# print_list(a) # 输出:a | b;c
# print_list(b) # 输出:a[;]b | c
# print_list(c) # 输出:a[[[;]]]b | c
# print_list(d) # 输出:a[;b;c 
# print_list(e) # 输出:a[] | b 

# 3.7 foreach
foreach(x A;B;C D E F)
    message("${x}")
endforeach(x A;B;C D E F)
set(list X;Y;Z)
foreach(x ${list})
    message("${x}")
endforeach()
# 打包遍历
set(a A;B;C)
set(b 0;1;2)
set(c X;Y)
foreach(x IN ZIP_LISTS a;b c)
    message("x_0: ${x_0}, x_1: ${x_1}, x_2: ${x_2}")
endforeach()
foreach(x y z IN ZIP_LISTS a b;c)
    message("x:   ${x}, y:   ${y}, z:   ${z}")
endforeach()
# foreach(x y IN ZIP_LISTS a b c) # 报错
# endforeach()
# 函数或宏参数列表
macro(my_macro p)
    message("ARGC: ${ARGC}")
    message("ARGV: ${ARGV}")
    message("ARGN: ${ARGN}")
    message("ARGV0: ${ARGV0}, ARGV1: ${ARGV1}")
endmacro()
function(my_func p)
    message("ARGC: ${ARGC}")
    message("ARGV: ${ARGV}")
    message("ARGN: ${ARGN}")
    message("ARGV0: ${ARGV0}, ARGV1: ${ARGV1}")
endfunction()
my_macro(x y z)
my_func(x y z)






