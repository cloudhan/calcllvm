find_package(GTest REQUIRED)

add_executable(calctest
    LexerTest.cpp
    ParserTest.cpp
)

target_include_directories(calctest PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../lib)
target_link_libraries(calctest PRIVATE libcalcllvm GTest::gtest GTest::gtest_main)
