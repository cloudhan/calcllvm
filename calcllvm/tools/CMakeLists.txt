include_directories(${CMAKE_CURRENT_LIST_DIR}/../lib)
link_libraries(libcalcllvm)

set(LLVM_CLANG ${LLVM_TOOLS_BINARY_DIR}/clang)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/calcc_runtime.ll
    COMMAND ${LLVM_CLANG} ARGS "-S" "-emit-llvm" "${CMAKE_CURRENT_LIST_DIR}/../runtime/runtime.c" -o ${CMAKE_CURRENT_BINARY_DIR}/calcc_runtime.ll
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/../runtime/runtime.c
)
add_custom_target(calcc_rt DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/calcc_runtime.ll)

add_executable(calcc Compiler.cpp)
add_dependencies(calcc calcc_rt)

add_executable(calci Interpreter.cpp)
