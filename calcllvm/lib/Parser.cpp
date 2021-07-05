#include "Parser.h"

inline void Parser::error() const {
    throw std::runtime_error("parser error");
}

inline void Parser::advance() {
    token = lexer.next();
}

inline void Parser::consume(TokenKind kind) {
    if (!expect(kind)) {
        error();
    }
    advance();
}

inline bool Parser::expect(TokenKind kind) const {
    if (!token.is(kind)) {
        error();
        return false;
    }
    return true;
}

bool Parser::isBuiltinFuncName(llvm::StringRef ident) const {
    std::vector<std::string> builtins = {"abs", "exp", "log2",   "lg",     "ln",     "sin",    "cos",
                                         "tan", "cot", "arcsin", "arccos", "arctan", "arccot", "sqrt"};
    for (const auto& name : builtins) {
        if (ident.equals(name)) {
            return true;
        }
    }
    return false;
}

Expr* Parser::parseExpr() {
    auto expr = parseTerm(0);
    return expr;
}

bool isPostfixOp(Token token) {
    return token.is(TokenKind::OP_FACT);
}

bool isBinaryOp(Token token) {
    return token.isOneOf(TokenKind::OP_PLUS, TokenKind::OP_MINUS, TokenKind::OP_MUL, TokenKind::OP_DIV,
                         TokenKind::OP_POW, TokenKind::OP_MOD);
}

int getPrecedence(Token token, bool binary = true) {
    if (binary) {
        switch (token.kind) {
        case TokenKind::OP_POW:
            return 4;
        case TokenKind::OP_MOD:
            return 2;
        case TokenKind::OP_MUL:
        case TokenKind::OP_DIV:
            return 1;
        case TokenKind::OP_PLUS:
        case TokenKind::OP_MINUS:
            return 0;
        default:
            return -1;
        }
    } else {
        switch (token.kind) {
        case TokenKind::OP_PLUS:
        case TokenKind::OP_MINUS:
            return 5;
        case TokenKind::OP_FACT:
            return 3;
        default:
            return -1;
        }
    }
}

bool isRightAssociative(Token token) {
    return token.is(TokenKind::OP_POW);
}

Expr* Parser::parseTerm(int precedence) {
    Expr* ret = parseFactor();
    while ((isBinaryOp(token) || isPostfixOp(token)) && getPrecedence(token, isBinaryOp(token)) >= precedence) {
        if (isBinaryOp(token)) {
            BinaryOp::Op op_kind{};
            switch (token.kind) {
#define CASE(token_kind, op)                                                                                           \
    case (token_kind):                                                                                                 \
        op_kind = op;                                                                                                  \
        break
                CASE(TokenKind::OP_PLUS, BinaryOp::PLUS);
                CASE(TokenKind::OP_MINUS, BinaryOp::MINUS);
                CASE(TokenKind::OP_MUL, BinaryOp::MUL);
                CASE(TokenKind::OP_DIV, BinaryOp::DIV);
                CASE(TokenKind::OP_POW, BinaryOp::POW);
                CASE(TokenKind::OP_MOD, BinaryOp::MOD);
#undef CASE
            }
            int q = isRightAssociative(token) ? getPrecedence(token) : 1 + getPrecedence(token);
            advance();
            auto rhs = parseTerm(q);
            ret = new BinaryOp(op_kind, ret, rhs);
        } else {
            consume(TokenKind::OP_FACT);
            ret = new UnaryOp(UnaryOp::FACT, ret);
        }
    }
    return ret;
}

Expr* Parser::parseFactor() {
    if (token.isOneOf(TokenKind::OP_PLUS, TokenKind::OP_MINUS)) {
        auto t = token;
        advance();
        auto e = parseTerm(getPrecedence(t, /*binary=*/false));
        return new UnaryOp(t.is(TokenKind::OP_PLUS) ? UnaryOp::POS : UnaryOp::NEG, e);
    }

    if (token.is(TokenKind::L_PARAN)) {
        advance();
        auto e = parseExpr();
        consume(TokenKind::R_PARAN);
        return e;
    }

    if (token.is(TokenKind::IDENT)) {
        if (isBuiltinFuncName(token.text)) {
            return parseFuncCall();
        } else {
            auto t = token;
            advance();
            return new Factor(Factor::IDENT, t.text);
        }
    }

    if (token.isOneOf(TokenKind::FP_LITERAL, TokenKind::INT_LITERAL)) {
        return parseNumber();
    }

    error();
    return nullptr;
}

Expr* Parser::parseFuncCall() {
    auto func_name = token.text;
    consume(TokenKind::IDENT);
    consume(TokenKind::L_PARAN);
    auto e = parseExpr();
    consume(TokenKind::R_PARAN);
    return new FuncCall(func_name, e);
}

Expr* Parser::parseNumber() {
    Expr* ret{};
    if (token.is(TokenKind::FP_LITERAL)) {
        ret = new Number(Number::FLOAT, token.text);
        advance();
    } else if (token.is(TokenKind::INT_LITERAL)) {
        ret = new Number(Number::INT, token.text);
        advance();
    } else {
        error();
        ret = nullptr;
    }
    return ret;
}

Parser::Parser(Lexer& lexer)
    : lexer(lexer)
    , token{TokenKind::UNKNOWN} {
    advance();
}

AST* Parser::parse() {
    auto e = parseExpr();
    expect(TokenKind::EOI);
    return e;
}
