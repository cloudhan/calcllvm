cmake_minimum_required(VERSION 3.18)
project(calcllvm)

option(ENABLE_TESTS "Enable tests" OFF)

set(CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})
if(MSVC)
    add_compile_options(/W4)
    # add_compile_options(/WX) # as error
else()
    add_compile_options(-Wall -Wextra -pedantic)
    # add_compile_options(-Werror) # as error
endif()

if(NOT BUILD_SHARED_LIBS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(calcllvm)
