set(CALCLLVM_SRCS
    "Lexer.cpp"
    "Parser.cpp"
)

add_library(libcalcllvm ${CALCLLVM_SRCS})

llvm_map_components_to_libnames(llvm_libs support core)

target_link_libraries(libcalcllvm PUBLIC ${llvm_libs})
