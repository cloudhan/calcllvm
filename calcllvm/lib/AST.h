#pragma once

#include <llvm/ADT/StringRef.h>

/**
 * Builtin functions:
 *  - abs
 *  - exp, log2, lg, ln,
 *  - sin, cos, tan, cot, arcsin, arccos, arctan, arccot,
 *
 * Grammar:
 *
 *  expr := term | '+' term | '-' term | term '+' term | term '-' term
 *
 *  term := factor
 *        | factpr '!'
 *        | factor '^' factor
 *        | factor '*' factor
 *        | factor '/' factor
 *        | factor '%' factor
 *
 *  factor := '(' expr ')'
 *          | func_call
 *          | IDENT
 *          | number
 *
 *  func_call := IDENT '(' expr ')'
 *
 *  number := FP_LITERAL | INT_LITERAL
 */

class AST;
class Expr;
class Term;
class Factor;
class UnaryOp;
class BinaryOp;
class FuncCall;

struct ASTVisitor {
    virtual ~ASTVisitor() {}

    virtual void visit(AST&) = 0;
    virtual void visit(Expr&) = 0;
    virtual void visit(Term&) = 0;
    virtual void visit(Factor&) = 0;
    virtual void visit(UnaryOp&) = 0;
    virtual void visit(BinaryOp&) = 0;
    virtual void visit(FuncCall&) = 0;
};

class AST {
public:
    virtual ~AST() {}
    virtual void accept(ASTVisitor& v) {
        v.visit(*this);
    };
};

class Expr : public AST {};

class Term : public Expr {};

class Factor : public Term {
public:
    enum Kind {
        IDENT,
        NUMBER,
        UnaryOp,
        BinaryOp,
    };

private:
    Kind kind;
    llvm::StringRef value;

public:
    Factor(Kind factor_kind, llvm::StringRef value)
        : kind(factor_kind)
        , value(value) {}

    Factor(Kind factor_kind)
        : Factor(factor_kind, "") {}

    Kind getKind() const {
        return kind;
    }

    llvm::StringRef getValueLiteralStr() {
        return value;
    }
};

class UnaryOp : public Factor {
public:
    enum Op {
        POS,
        NEG,
        FACT,
    };

private:
    Op op;
    Expr* e;

public:
    UnaryOp(Op op, Expr* e)
        : Factor(Factor::UnaryOp)
        , op(op)
        , e(e) {}

    Op getOp() {
        return op;
    }

    Expr* getExpr() {
        return e;
    }
};

class BinaryOp : public Factor {
public:
    enum Op {
        PLUS,
        MINUS,
        MUL,
        DIV,
        POW,
        MOD,
    };

private:
    BinaryOp::Op op;
    Expr* lhs;
    Expr* rhs;

public:
    BinaryOp(Op op, Expr* lhs, Expr* rhs)
        : Factor(Factor::BinaryOp)
        , op(op)
        , lhs(lhs)
        , rhs(rhs) {}

    Op getOp() const {
        return op;
    }

    Expr* getLeft() {
        return lhs;
    }

    Expr* getRight() {
        return rhs;
    }
};

class FuncCall : public Expr {
    llvm::StringRef name;
    Expr* param;

public:
    FuncCall(llvm::StringRef ident, Expr* e)
        : name(ident)
        , param(e) {}
    llvm::StringRef getName() const {
        return name;
    }
    Expr* getParam() const {
        return param;
    }
};

class Number : public Factor {
public:
    enum Type {
        INT,
        FLOAT,
    };

private:
    Type t;

public:
    Number(Type t, llvm::StringRef v)
        : Factor(Factor::NUMBER, v)
        , t(t) {}

    Type getType() {
        return t;
    }
};
