cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set (LIB ${CURRENT_FOLDER})

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
add_library(${LIB} STATIC ${SRC})
target_include_directories(${LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(${LIB} PRIVATE -Wno-unused-result)
