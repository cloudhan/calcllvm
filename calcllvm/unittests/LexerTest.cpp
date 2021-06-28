#include "Lexer.h"
#include "gtest/gtest.h"

#include <iostream>

TEST(LexerTest, one_token) {
#define EXPECT_EQ_3(src, kind, txt) [&]() { EXPECT_EQ(Lexer(src).next(), Token((kind), (txt))); }()

    EXPECT_EQ_3("123", TokenKind::INT_LITERAL, "123");
    EXPECT_EQ_3("1234567890", TokenKind::INT_LITERAL, "1234567890");
    EXPECT_EQ_3("1.23", TokenKind::FP_LITERAL, "1.23");
    EXPECT_EQ_3("9087654321.0", TokenKind::FP_LITERAL, "9087654321.0");
    EXPECT_EQ_3("1.1234567890", TokenKind::FP_LITERAL, "1.1234567890");
    EXPECT_EQ_3("a", TokenKind::IDENT, "a");
    EXPECT_EQ_3("a12", TokenKind::IDENT, "a12");
    EXPECT_EQ_3("_a12", TokenKind::IDENT, "_a12");
    EXPECT_EQ_3("_1", TokenKind::IDENT, "_1");
    EXPECT_EQ_3("+", TokenKind::OP_PLUS, "+");
    EXPECT_EQ_3("-", TokenKind::OP_MINUS, "-");
    EXPECT_EQ_3("*", TokenKind::OP_MUL, "*");
    EXPECT_EQ_3("/", TokenKind::OP_DIV, "/");
    EXPECT_EQ_3("%", TokenKind::OP_MOD, "%");
    EXPECT_EQ_3("^", TokenKind::OP_POW, "^");
    EXPECT_EQ_3("!", TokenKind::OP_FACT, "!");

#undef EXPECT_EQ_3
}

TEST(LexerTest, two_tokens) {
#define EXPECT_EQ_5(src, kind1, text1, kind2, text2)                                                                   \
    [&]() {                                                                                                            \
        auto lexer = Lexer(src);                                                                                       \
        EXPECT_EQ(lexer.next(), Token((kind1), (text1)));                                                              \
        EXPECT_EQ(lexer.next(), Token((kind2), (text2)));                                                              \
    }();

    EXPECT_EQ_5("123 1.23", TokenKind::INT_LITERAL, "123", TokenKind::FP_LITERAL, "1.23");
    EXPECT_EQ_5("1.23    123", TokenKind::FP_LITERAL, "1.23", TokenKind::INT_LITERAL, "123");
    EXPECT_EQ_5("a 1", TokenKind::IDENT, "a", TokenKind::INT_LITERAL, "1");
    EXPECT_EQ_5("1+", TokenKind::INT_LITERAL, "1", TokenKind::OP_PLUS, "+");
    EXPECT_EQ_5("-1", TokenKind::OP_MINUS, "-", TokenKind::INT_LITERAL, "1");
    EXPECT_EQ_5("*/", TokenKind::OP_MUL, "*", TokenKind::OP_DIV, "/");
    EXPECT_EQ_5("%^", TokenKind::OP_MOD, "%", TokenKind::OP_POW, "^");
    EXPECT_EQ_5("!!", TokenKind::OP_FACT, "!", TokenKind::OP_FACT, "!");

#undef EXPECT_EQ_5
}

TEST(LexerTest, complex) {
    auto text = "(1+2-z!) %   4 + (6*b/7.0) ^ 8 - sqrt(9.0) ";
    auto lexer = Lexer(text);

    EXPECT_EQ(lexer.next(), Token(TokenKind::L_PARAN, "("));
    EXPECT_EQ(lexer.next(), Token(TokenKind::INT_LITERAL, "1"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_PLUS, "+"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::INT_LITERAL, "2"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_MINUS, "-"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::IDENT, "z"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_FACT, "!"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::R_PARAN, ")"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_MOD, "%"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::INT_LITERAL, "4"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_PLUS, "+"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::L_PARAN, "("));
    EXPECT_EQ(lexer.next(), Token(TokenKind::INT_LITERAL, "6"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_MUL, "*"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::IDENT, "b"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_DIV, "/"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::FP_LITERAL, "7.0"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::R_PARAN, ")"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_POW, "^"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::INT_LITERAL, "8"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::OP_MINUS, "-"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::IDENT, "sqrt"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::L_PARAN, "("));
    EXPECT_EQ(lexer.next(), Token(TokenKind::FP_LITERAL, "9.0"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::R_PARAN, ")"));
    EXPECT_EQ(lexer.next(), Token(TokenKind::EOI));
}

TEST(LexerTest, endless) {
    auto text = "";
    auto lexer = Lexer(text);
    EXPECT_EQ(lexer.next(), Token(TokenKind::EOI));
    EXPECT_EQ(lexer.next(), Token(TokenKind::EOI));
    EXPECT_EQ(lexer.next(), Token(TokenKind::EOI));
}
