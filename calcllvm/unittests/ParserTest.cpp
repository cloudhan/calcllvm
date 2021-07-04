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

TEST(ParserTest, binary_op) {
#define DO_TEST(text, op, sexpr)                                                                                       \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto bo = dynamic_cast<BinaryOp*>(e);                                                                          \
        EXPECT_NE(bo, nullptr);                                                                                        \
        EXPECT_EQ(bo->getKind(), Factor::BinaryOp);                                                                    \
        EXPECT_EQ(bo->getOp(), op);                                                                                    \
        ToSExprVisitor scvt;                                                                                           \
        auto my_sexpr = scvt.convert(e);                                                                               \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("x+1", BinaryOp::PLUS, "(+ x 1)");
    DO_TEST("1-y", BinaryOp::MINUS, "(- 1 y)");
    DO_TEST("2.0 * 2.0", BinaryOp::MUL, "(* 2.0 2.0)");
    DO_TEST("x / -y", BinaryOp::DIV, "(/ x (- y))");
    DO_TEST("4 ^ 2 ", BinaryOp::POW, "(^ 4 2)");
    DO_TEST("4 % 2 ", BinaryOp::MOD, "(% 4 2)");

#undef DO_TEST
}

TEST(ParserTest, binary_op_compound) {
#define DO_TEST(text, sexpr)                                                                                           \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto bo = dynamic_cast<BinaryOp*>(e);                                                                          \
        EXPECT_NE(bo, nullptr);                                                                                        \
        ToSExprVisitor scvt;                                                                                           \
        auto my_sexpr = scvt.convert(e);                                                                               \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("(1+2+3)", "(+ (+ 1 2) 3)");
    DO_TEST("(1*2*3)", "(* (* 1 2) 3)");
    DO_TEST("(1*2+3)", "(+ (* 1 2) 3)");
    DO_TEST("(1+2*3)", "(+ 1 (* 2 3))");
    DO_TEST("(1+2/3)", "(+ 1 (/ 2 3))");
    DO_TEST("(1^2*3)", "(* (^ 1 2) 3)");
    DO_TEST("(1^2^3)", "(^ 1 (^ 2 3))");

    DO_TEST("(1*2+3/4)", "(+ (* 1 2) (/ 3 4))");
    DO_TEST("(1*2^3/4)", "(/ (* 1 (^ 2 3)) 4)");

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

TEST(ParserTest, simple) {
#define DO_TEST(text, sexpr)                                                                                           \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        ToSExprVisitor v;                                                                                              \
        auto my_sexpr = v.convert(e);                                                                                  \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("1+2-x", "(- (+ 1 2) x)");
    DO_TEST("1+2-3+4", "(+ (- (+ 1 2) 3) 4)");
    DO_TEST("1-2-3-4-5", "(- (- (- (- 1 2) 3) 4) 5)");
    DO_TEST("1*2-x", "(- (* 1 2) x)");
    DO_TEST("1+2*x", "(+ 1 (* 2 x))");
    DO_TEST("x^y^z", "(^ x (^ y z))");

#undef DO_TEST
}

TEST(ParserTest, complex) {
#define DO_TEST(text, sexpr)                                                                                           \
    [&]() {                                                                                                            \
        Lexer lexer(text);                                                                                             \
        Parser parser(lexer);                                                                                          \
        auto e = parser.parse();                                                                                       \
        EXPECT_NE(e, nullptr);                                                                                         \
        auto my_sexpr = ToSExprVisitor().convert(e);                                                                   \
        EXPECT_EQ(sexpr, my_sexpr);                                                                                    \
    }()

    DO_TEST("1+2-sqrt(4)", "(- (+ 1 2) (sqrt 4))");

    DO_TEST("(1+2-z!) %   4 + (6*b/7.0) ^ 8 - sqrt(9.0) ",
            "(- (+ (% (- (+ 1 2) (! z)) 4) (^ (/ (* 6 b) 7.0) 8)) (sqrt 9.0))");

#undef DO_TEST
}
