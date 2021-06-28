
/**
 * INT_LITERAL: [0-9]+
 * FP_LITERAL: [0-9]+(.[0-9]*)?
 * OP_PLUS, OP_MINUS, OP_DIV, OP_MUL: '+', '-', '*', '/'
 * OP_POW, OP_MOD, OP_FACT: '^', '%', '!'
 * L_PARAN, R_PARAN: '(', ')'
 * IDENT: [a-zA-Z_][a-zA-Z_0-9]*
 */

#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>

enum class TokenKind : uint8_t {
    EOI, // end of input
    UNKNOWN,
    INT_LITERAL,
    FP_LITERAL,
    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_MOD,
    OP_FACT,
    L_PARAN,
    R_PARAN,
    IDENT,
};

struct Token {
    TokenKind kind;
    llvm::StringRef text;

    Token(TokenKind kind, llvm::StringRef text)
        : kind(kind)
        , text(text){};
    Token(TokenKind kind)
        : Token(kind, llvm::StringRef{}) {}

    bool operator==(const Token& other) const {
        return kind == other.kind && text.equals(other.text);
    }

    inline bool is(TokenKind kind) const {
        return kind == this->kind;
    }

    inline bool isOneOf(TokenKind kind1, TokenKind kind2) {
        return is(kind1) || is(kind2);
    }

    template <typename... Ts>
    bool isOneOf(TokenKind kind1, Ts... kinds) {
        return is(kind1) || isOneOf(kinds...);
    }
};

class Lexer {
    const char* const bufferBase;
    const char* bufferCurr;

public:
    Lexer() = delete;
    Lexer(const llvm::StringRef& src)
        : bufferBase(src.begin())
        , bufferCurr(bufferBase) {}

    Token next();

private:
    Token formToken(const char* end, TokenKind kind) {
        llvm::StringRef text(bufferCurr, end - bufferCurr);
        bufferCurr = end;
        return Token(kind, text);
    }
};
