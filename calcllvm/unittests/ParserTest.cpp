#include "Parser.h"
#include "gtest/gtest.h"

TEST(ParserTest, func_call) {
#define DO_TEST(text, name, param_t)                                                                                   \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        auto fc = static_cast<FuncCall*>(e);                                                                           \
        EXPECT_NE(fc, nullptr);                                                                                        \
        EXPECT_TRUE(fc->getName().equals(name));                                                                       \
    }()

    DO_TEST("sin(x)", "sin", Factor);
    DO_TEST("cos(2.0)", "cos", Factor);
    DO_TEST("tan(3.0+4)", "tan", BinaryOp);
    DO_TEST("exp(8.0)", "exp", Factor);
}
