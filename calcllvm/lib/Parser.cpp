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
    std::vector<std::string> builtins = {"abs", "exp", "log2",   "lg",     "ln",     "sin",   "cos",
                                         "tan", "cot", "arcsin", "arccos", "arctan", "arccot"};
    for (const auto& name : builtins) {
        if (ident.equals(name)) {
            return true;
        }
    }
    return false;
}

AST* Parser::parseLang() {
    auto expr = parseExpr();
    expect(TokenKind::EOI);
    return expr;
}

Expr* Parser::parseExpr() {
    Expr* ret;
    if (token.isOneOf(TokenKind::OP_PLUS, TokenKind::OP_MINUS)) {
        auto op_kind = token.is(TokenKind::OP_PLUS) ? UnaryOp::POS : UnaryOp::NEG;
        advance();
        auto e = parseTerm();
        ret = new UnaryOp(op_kind, e);
    } else {
        ret = parseTerm();
        if (token.isOneOf(TokenKind::OP_PLUS, TokenKind::OP_MINUS)) {
            auto op_kind = token.is(TokenKind::OP_PLUS) ? BinaryOp::PLUS : BinaryOp::MINUS;
            advance();
            auto term2 = parseTerm();
            ret = new BinaryOp(op_kind, ret, term2);
        }
    }
    return ret;
}

Expr* Parser::parseTerm() {
    Expr* ret = parseFactor();
    if (token.is(TokenKind::OP_FACT)) {
        ret = new UnaryOp(UnaryOp::FACT, ret);
    } else if (token.isOneOf(TokenKind::OP_POW, TokenKind::OP_MUL, TokenKind::OP_DIV, TokenKind::OP_MOD)) {
        BinaryOp::Op op_kind{};
        switch (token.kind) {
#define CASE(token_kind, op)                                                                                           \
    op_kind = op;                                                                                                      \
    break
            CASE(TokenKind::OP_POW, BinaryOp::POW);
            CASE(TokenKind::OP_MUL, BinaryOp::MUL);
            CASE(TokenKind::OP_DIV, BinaryOp::DIV);
            CASE(TokenKind::OP_MOD, BinaryOp::MOD);
#undef CASE
        default:
            error();
        }
        advance();

        auto rhs = parseFactor();
        ret = new BinaryOp(op_kind, ret, rhs);
    }
    return ret;
}

Expr* Parser::parseFactor() {
    Expr* ret;
    if (token.is(TokenKind::L_PARAN)) {
        advance();
        ret = parseExpr();
        consume(TokenKind::R_PARAN);
    } else if (token.is(TokenKind::IDENT)) {
        if (isBuiltinFuncName(token.text)) {
            ret = parseFuncCall();
        } else {
            ret = new Factor(Factor::IDENT, token.text);
            advance();
        }
    } else if (token.isOneOf(TokenKind::FP_LITERAL, TokenKind::INT_LITERAL)) {
        ret = parseNumber();
    } else {
        ret = nullptr;
        error();
    }
    return ret;
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
    Expr* ret;
    if (token.is(TokenKind::FP_LITERAL)) {
        ret = new Number(Number::FLOAT, token.text);
    } else if (token.is(TokenKind::INT_LITERAL)) {
        ret = new Number(Number::INT, token.text);
    } else {
        error();
        ret = nullptr;
    }
    advance();
    return ret;
}

Parser::Parser(Lexer& lexer)
    : lexer(lexer)
    , token{TokenKind::UNKNOWN} {
    advance();
}

AST* Parser::parse() {
    return parseLang();
}
