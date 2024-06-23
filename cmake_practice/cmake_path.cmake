cmake_minimum_required(VERSION 3.20)
# 创建路径变量
message(STATUS "创建路径变量")
cmake_path(SET x NORMALIZE "a/../b")
message("${x}") # 输出b

# 追加路径
message(STATUS "追加路径变量")
cmake_path(APPEND res a b c)
message("${res}") # 输出：a/b/c
cmake_path(APPEND_STRING res a b c)
message("${res}") # 输出：a/b/cabc
cmake_path(APPEND res d e)
message("${res}") # 输出：a/b/cabc/d/e
if(WIN32)
    cmake_path(APPEND res a c:b c:c OUTPUT_VARIABLE res2)
else()
    cmake_path(APPEND res a /b c OUTPUT_VARIABLE res2)
endif()
message("${res2}")
# 在Windows操作系统中输出：c:b/c
# 在类UNIX操作系统中输出：/b/c
#[[ 在追加过程中，若追加部分的路径是一个绝对路径，
或追加部分的路径存在根名且根名与当前被追加的路径根名不同，
则该追加部分的路径会直接替换掉当前被追加的路径。]]
# 分解路径
message(STATUS "分解路径")
cmake_path(SET CURRENT_FILE_PATH  ${CMAKE_CURRENT_LIST_DIR})
cmake_path(APPEND CURRENT_FILE_PATH  ${CMAKE_CURRENT_LIST_FILE})
message(STATUS "The absolute path of the current CMake file is: ${CURRENT_FILE_PATH}")

cmake_path(GET CURRENT_FILE_PATH ROOT_NAME res)
message("res ROOT_NAME:${res}")
cmake_path(GET CURRENT_FILE_PATH ROOT_DIRECTORY res) 
message("res ROOT_DIRECTORY:${res}")
cmake_path(GET CURRENT_FILE_PATH ROOT_PATH res)
message("res ROOT_PATH:${res}")
cmake_path(GET CURRENT_FILE_PATH FILENAME res)
message("res FILENAME:${res}")
cmake_path(GET CURRENT_FILE_PATH EXTENSION LAST_ONLY res)
message("res EXTENSION:${res}")
cmake_path(GET CURRENT_FILE_PATH STEM LAST_ONLY res)
message("res STEM:${res}")
cmake_path(GET CURRENT_FILE_PATH RELATIVE_PART res)
message("res RELATIVE_PART:${res}")
cmake_path(GET CURRENT_FILE_PATH PARENT_PATH res)
message("res PARENT_PATH:${res}")





