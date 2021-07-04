#pragma once

#include "AST.h"
#include "Lexer.h"

class Parser {
    Lexer& lexer;
    Token token; // the peaked token

    void error() const;
    void advance();
    void consume(TokenKind kind);
    bool expect(TokenKind kind) const;
    bool isBuiltinFuncName(llvm::StringRef) const;

    Expr* parseExpr();
    Expr* parseTerm(int precedence);
    Expr* parseFactor();
    Expr* parseFuncCall();
    Expr* parseNumber();

public:
    Parser(Lexer& lexer);
    AST* parse();
};
