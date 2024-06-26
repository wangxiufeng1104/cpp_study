# Turn on warnings on the given target
function(spdlog_enable_warnings target_name)
    if(SPDLOG_BUILD_WARNINGS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            list(APPEND MSVC_OPTIONS "/W3")
            if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
                list(APPEND MSVC_OPTIONS "/WX")
            endif()
        endif()

        target_compile_options(
            ${target_name}
            PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                    -Wall
                    -Wextra
                    -Wconversion
                    -pedantic
                    -Werror
                    -Wfatal-errors>
                    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_OPTIONS}>)
    endif()
endfunction()
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set (APP ${CURRENT_FOLDER})
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR}/..)
aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
list(APPEND SRC ${PROTOBUF_SRC})
add_executable(${APP} ${SRC})

spdlog_enable_warnings(${APP})

target_link_libraries(${APP} pthread protobus_v2)
target_include_directories(${APP}
                            PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}/
                            ${CMAKE_CURRENT_SOURCE_DIR}/inc
                            )
