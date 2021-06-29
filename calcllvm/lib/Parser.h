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

    AST* parseLang();
    Expr* parseExpr();
    Expr* parseTerm();
    Expr* parseFactor();
    Expr* parseFuncCall();
    Expr* parseNumber();

public:
    Parser(Lexer& lexer);
    AST* parse();
};
