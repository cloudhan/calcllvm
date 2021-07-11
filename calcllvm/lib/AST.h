#pragma once

#include <llvm/ADT/StringRef.h>
#include <stdexcept>

/**
 * Builtin functions:
 *  - abs
 *  - exp, log2, lg, ln,
 *  - sin, cos, tan, cot, arcsin, arccos, arctan, arccot,
 *  - sqrt,
 *
 * Grammar:
 *
 *  expr := term(0)
 *
 *  term(p) := factpr { binary_op term(q) }
 *
 *  factor := unary_op term(q)
 *          | '(' expr ')'
 *          | ident
 *          | func_call
 *          | number
 *
 *  func_call := ident '(' expr ')'
 *
 *  number := fp_literal | int_literal
 */

class AST;
class Expr;
class Term;
class Factor;
class UnaryOp;
class BinaryOp;
class FuncCall;
class Ident;
class Number;

struct ASTVisitor {
    virtual ~ASTVisitor() {}

    virtual void visit(AST&) {}
    virtual void visit(Expr&) {}
    virtual void visit(Term&) {}
    virtual void visit(Factor&) {}
    virtual void visit(UnaryOp&) = 0;
    virtual void visit(BinaryOp&) = 0;
    virtual void visit(FuncCall&) = 0;
    virtual void visit(Ident&) = 0;
    virtual void visit(Number&) = 0;
};

class AST {
public:
    enum class Kind {
        AST,
        Expr,
        Term,
        Factor,
        UnaryOp,
        BinaryOp,
        FuncCall,
        Number,
        Ident,
    } class_kind;

private:
    const Kind kind;

public:
    AST(Kind kind)
        : kind(kind) {}

    virtual ~AST() {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::AST;
    }

    Kind getKind() const {
        return kind;
    }

    virtual void accept(ASTVisitor& v) = 0;
};

class Expr : public AST {
public:
    Expr(Kind kind)
        : AST(kind) {}

    void accept(ASTVisitor&) override {
        throw std::runtime_error("visitor should properly implement all required visit functions");
    };
};

class Term : public Expr {
public:
    Term(Kind kind)
        : Expr(kind) {}
};

class Factor : public Term {
public:
    Factor(Kind kind)
        : Term(kind) {}
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
        : Factor(Kind::UnaryOp)
        , op(op)
        , e(e) {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::UnaryOp;
    }

    Op getOp() {
        return op;
    }

    Expr* getExpr() {
        return e;
    }

    void accept(ASTVisitor& v) override {
        v.visit(*this);
    };
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
        : Factor(Kind::BinaryOp)
        , op(op)
        , lhs(lhs)
        , rhs(rhs) {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::BinaryOp;
    }

    Op getOp() const {
        return op;
    }

    Expr* getLeft() {
        return lhs;
    }

    Expr* getRight() {
        return rhs;
    }

    void accept(ASTVisitor& v) override {
        v.visit(*this);
    };
};

class FuncCall : public Expr {
    llvm::StringRef name;
    Expr* param;

public:
    FuncCall(llvm::StringRef ident, Expr* e)
        : Expr(Kind::FuncCall)
        , name(ident)
        , param(e) {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::FuncCall;
    }

    llvm::StringRef getName() const {
        return name;
    }

    Expr* getParam() const {
        return param;
    }

    void accept(ASTVisitor& v) override {
        v.visit(*this);
    };
};

class Number : public Factor {
public:
    enum Type {
        INT,
        FLOAT,
    };

private:
    Type t;
    llvm::StringRef value;

public:
    Number(Type t, llvm::StringRef v)
        : Factor(Kind::Number)
        , t(t)
        , value(v) {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::Number;
    }

    Type getType() {
        return t;
    }

    llvm::StringRef getValue() {
        return value;
    }

    void accept(ASTVisitor& v) override {
        v.visit(*this);
    };
};

class Ident : public Factor {
    llvm::StringRef name;

public:
    Ident(llvm::StringRef name)
        : Factor(Kind::Ident)
        , name(name) {}

    static bool classof(const AST* node) {
        return node->getKind() == Kind::Ident;
    }

    llvm::StringRef getName() {
        return name;
    }

    void accept(ASTVisitor& v) override {
        v.visit(*this);
    };
};
