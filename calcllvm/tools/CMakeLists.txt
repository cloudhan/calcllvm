include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib)
link_libraries(libcalcllvm)

add_executable(calcc Compiler.cpp)
add_executable(calci Interpreter.cpp)
