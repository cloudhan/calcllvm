#pragma once

#include "AST.h"
#include <sstream>

class ToSExprVisitor : public ASTVisitor {
    std::stringstream result;

    void error() {
        throw std::runtime_error("sexpr convert error");
    }

public:
    std::string convert(AST* tree) {
        tree->accept(*this);
        return result.str();
    }

    void visit(Factor& e) override {
        if (e.getKind() == Factor::IDENT) {
            result << e.getValueLiteralStr().str();
        }
    }

    void visit(Number& e) override {
        result << e.getValueLiteralStr().str();
    }

    void visit(UnaryOp& e) override {
        switch (e.getOp()) {
        case UnaryOp::POS:
            e.getExpr()->accept(*this);
            return;
        case UnaryOp::NEG:
            result << "(- ";
            e.getExpr()->accept(*this);
            result << ")";
            return;
        case UnaryOp::FACT:
            result << "(! ";
            e.getExpr()->accept(*this);
            result << ")";
            return;
        default:
            error();
        }
    }

    void visit(BinaryOp& e) override {
        char c;
        switch (e.getOp()) {
#define CASE(v1, v2)                                                                                                   \
    case (v1):                                                                                                         \
        c = v2;                                                                                                        \
        break

            CASE(BinaryOp::PLUS, '+');
            CASE(BinaryOp::MINUS, '-');
            CASE(BinaryOp::MUL, '*');
            CASE(BinaryOp::DIV, '/');
            CASE(BinaryOp::POW, '^');
            CASE(BinaryOp::MOD, '%');

#undef CASE
        }
        result << "(" << c << " ";
        e.getLeft()->accept(*this);
        result << " ";
        e.getRight()->accept(*this);
        result << ")";
    }

    void visit(FuncCall& e) override {
        result << "(" << e.getName().str() << " ";
        e.getParam()->accept(*this);
        result << ")";
    }
};
