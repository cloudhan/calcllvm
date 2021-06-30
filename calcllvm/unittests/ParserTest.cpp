#include "Parser.h"

#include "ToSExpr.h"
#include <gtest/gtest.h>

TEST(ParserTest, number) {
#define DO_TEST(text, type, literal)                                                                                   \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto n = dynamic_cast<Number*>(e);                                                                             \
        EXPECT_NE(n, nullptr);                                                                                         \
        EXPECT_EQ(n->getKind(), Factor::NUMBER);                                                                       \
        EXPECT_EQ(n->getType(), type);                                                                                 \
        EXPECT_EQ(n->getValueLiteralStr(), literal);                                                                   \
    }()

    DO_TEST("1", Number::INT, "1");
    DO_TEST("1.0", Number::FLOAT, "1.0");

#undef DO_TEST
}

TEST(ParserTest, unary_op) {
#define DO_TEST(text, op, sexpr)                                                                                       \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto uo = dynamic_cast<UnaryOp*>(e);                                                                           \
        EXPECT_NE(uo, nullptr);                                                                                        \
        EXPECT_EQ(uo->getKind(), Factor::UnaryOp);                                                                     \
        EXPECT_EQ(uo->getOp(), op);                                                                                    \
        ToSExprVisitor scvt;                                                                                           \
        auto my_sexpr = scvt.convert(e);                                                                               \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("+1", UnaryOp::POS, "1");
    DO_TEST("-1", UnaryOp::NEG, "(- 1)");
    DO_TEST("1!", UnaryOp::FACT, "(! 1)");

    DO_TEST("+x", UnaryOp::POS, "x");
    DO_TEST("-x", UnaryOp::NEG, "(- x)");
    DO_TEST("x!", UnaryOp::FACT, "(! x)");

#undef DO_TEST
}

TEST(ParserTest, func_call) {
#define DO_TEST(text, name, sexpr)                                                                                     \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto fc = dynamic_cast<FuncCall*>(e);                                                                          \
        EXPECT_NE(fc, nullptr);                                                                                        \
        EXPECT_TRUE(fc->getName().equals(name));                                                                       \
        ToSExprVisitor v;                                                                                              \
        auto my_sexpr = v.convert(e);                                                                                  \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("sin(x)", "sin", "(sin x)");
    DO_TEST("cos(2.0)", "cos", "(cos 2.0)");
    DO_TEST("tan(3.0+4)", "tan", "(tan (+ 3.0 4))");
    DO_TEST("exp(-8.0)", "exp", "(exp (- 8.0))");

#undef DO_TEST
}
