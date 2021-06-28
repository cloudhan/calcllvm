#include "Lexer.h"

namespace {
LLVM_READNONE inline bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

LLVM_READNONE inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

LLVM_READNONE inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

LLVM_READONLY const char* lexInt(const char* p) {
    while (isDigit(*p)) {
        p += 1;
    }
    return p;
}

LLVM_READONLY const char* lexIdent(const char* p) {
    if (!isAlpha(*p) && *p != '_') {
        return p;
    }
    p += 1;
    while (isAlpha(*p) || isDigit(*p) || *p == '_') {
        p += 1;
    }
    return p;
}
} // namespace

Token Lexer::next() {
    while (*bufferCurr && isWhitespace(*bufferCurr)) {
        ++bufferCurr;
    }

    if (*bufferCurr == '\0') {
        return Token(TokenKind::EOI);
    }

    // is a number: FP_LITERAL or INT_LITERAL
    if (isDigit(*bufferCurr)) {
        auto p = lexInt(bufferCurr);
        if (*p == '.') {
            p = lexInt(p + 1);
            return formToken(p, TokenKind::FP_LITERAL);
        }
        return formToken(p, TokenKind::INT_LITERAL);
    }

    // is an identifier
    if (isAlpha(*bufferCurr) || *bufferBase == '_') {
        auto p = lexIdent(bufferCurr);
        return formToken(p, TokenKind::IDENT);
    }

    switch (*bufferCurr) {
#define CASE(c, kind)                                                                                                  \
    case (c):                                                                                                          \
        return formToken(bufferCurr + 1, (kind));

        CASE('+', TokenKind::OP_PLUS);
        CASE('-', TokenKind::OP_MINUS);
        CASE('*', TokenKind::OP_MUL);
        CASE('/', TokenKind::OP_DIV);
        CASE('^', TokenKind::OP_POW);
        CASE('%', TokenKind::OP_MOD);
        CASE('!', TokenKind::OP_FACT);
        CASE('(', TokenKind::L_PARAN);
        CASE(')', TokenKind::R_PARAN);
#undef CASE
    default:
        return Token(TokenKind::UNKNOWN);
    }
}
