#pragma once

#include "AST.h"
#include "Lexer.h"

#include <cstdio>
#include <limits>
#include <unordered_map>

int64_t pow_(int64_t b, int64_t e) {
    if (e == 1)
        return b;

    if ((e % 2) == 0) {
        auto r = pow_(b, e / 2);
        return r * r;
    } else {
        auto r = pow_(b, e / 2);
        return r * r * b;
    }
}

int64_t pow(int64_t b, int64_t e) {
    if (b == 0 && e == 0)
        throw std::runtime_error("0^0 is undefined");

    if (e < 0)
        throw std::runtime_error("exponent < 0 for int value is not allowed");

    if (b == 0)
        return 1;

    return pow_(b, e);
}

class Value {
    union {
        int64_t v_i;
        double v_f;
    };
    bool isInt_;

    void error() const {
        throw std::runtime_error("value getting error");
    }

    double promoteToFloat() const {
        if (!isInt_)
            return v_f;
        return static_cast<double>(v_i);
    }

public:
    Value(int64_t v)
        : v_i(v)
        , isInt_(true) {}
    Value(double v)
        : v_f(v)
        , isInt_(false) {}

    Value()
        : Value(std::numeric_limits<double>::quiet_NaN()) {}

    bool isInt() const {
        return isInt_;
    }

    int64_t getInt() const {
        if (!isInt_) {
            error();
        }
        return v_i;
    }

    double getFloat() const {
        if (isInt_)
            return promoteToFloat();

        return v_f;
    }

    Value operator+(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return getInt() + rhs.getInt();
        }
        return getFloat() + rhs.getFloat();
    }

    Value operator-(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return getInt() - rhs.getInt();
        }
        return getFloat() - rhs.getFloat();
    }

    Value operator*(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return getInt() * rhs.getInt();
        }
        return getFloat() * rhs.getFloat();
    }

    Value operator/(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return getInt() / rhs.getInt();
        }
        return getFloat() / rhs.getFloat();
    }

    Value operator^(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return pow(getInt(), rhs.getInt());
        }
        return std::pow(getFloat(), rhs.getFloat());
    }

    Value operator%(const Value& rhs) const {
        if (isInt() && rhs.isInt()) {
            return getInt() % rhs.getInt();
        }
        throw std::runtime_error("mod for float value is not allowed");
    }
};

Value negate(Value v) {
    if (v.isInt()) {
        return Value(-v.getInt());
    } else {
        return Value(-v.getFloat());
    }
}

Value factorial(Value v) {
    int64_t i = v.getInt();
    if (i < 0) {
        throw std::runtime_error("factorial value error");
    }
    if (i == 0) {
        return Value(1LL);
    }
    int64_t acc = 1;
    for (int64_t j = 1; j <= i; j++) {
        acc *= j;
    }
    return Value(acc);
}

class InterpretVisitor : public ASTVisitor {
    std::unordered_map<std::string, Value> env;

public:
    InterpretVisitor()
        : eval_result(std::numeric_limits<double>::quiet_NaN()) {}

    void visit(Ident& e) override {
        auto ident = e.getName().str();
        auto it = env.find(ident);
        if (it == env.end()) {
            std::array<char, 256> buffer{};
            std::fill(buffer.begin(), buffer.end(), 0);
            printf("Input value %s: ", ident.c_str());
            scanf("%256s", buffer.data());
            auto it = std::find(buffer.begin(), buffer.end(), '.');
            Value v;
            if (it == buffer.end()) {
                v = std::atoll(buffer.data());
            } else {
                v = std::stod(buffer.data());
            }

            env[ident] = v;
            eval_result = v;
            return;
        }

        eval_result = it->second;
        return;
    }

    void visit(UnaryOp& e) override {
        e.getExpr()->accept(*this);
        // result is the eval_result for UnaryOp::POS;
        if (e.getOp() == UnaryOp::NEG) {
            eval_result = negate(eval_result);
        } else if (e.getOp() == UnaryOp::FACT) {
            eval_result = factorial(eval_result);
        }
    }

    void visit(BinaryOp& e) override {
        e.getLeft()->accept(*this);
        auto lhs = eval_result;
        e.getRight()->accept(*this);
        auto rhs = eval_result;

        switch (e.getOp()) {
#define CASE(p, op)                                                                                                    \
    case (p):                                                                                                          \
        eval_result = (lhs op rhs);                                                                                    \
        break
            CASE(BinaryOp::PLUS, +);
            CASE(BinaryOp::MINUS, -);
            CASE(BinaryOp::MUL, *);
            CASE(BinaryOp::DIV, /);
            CASE(BinaryOp::POW, ^);
            CASE(BinaryOp::MOD, %);
#undef CASE
        }
    }

    void visit(Number& e) override {
        if (e.getType() == Number::INT) {
            int64_t v;
            e.getValue().getAsInteger(10, v);
            eval_result = Value(v);
        } else if (e.getType() == Number::FLOAT) {
            double v;
            e.getValue().getAsDouble(v);
            eval_result = Value(v);
        }
    }

    void visit(FuncCall& e) override {
        e.getParam()->accept(*this);
        Value v = eval_result;
#define IF_THEN_CALL(func_name, callable)                                                                              \
    if (e.getName().equals(func_name)) {                                                                               \
        eval_result = Value((callable)(v.getFloat()));                                                                 \
        return;                                                                                                        \
    }

        IF_THEN_CALL("abs", std::abs);
        IF_THEN_CALL("exp", std::exp);
        IF_THEN_CALL("log2", std::log2);
        IF_THEN_CALL("ln", std::log);
        IF_THEN_CALL("lg", std::log10);
        IF_THEN_CALL("sin", std::sin);
        IF_THEN_CALL("cos", std::cos);
        IF_THEN_CALL("tan", std::tan);
        IF_THEN_CALL("cot", [&](Value v) { return 1.0 / std::tan(v.getFloat()); });
        IF_THEN_CALL("arcsin", std::asin);
        IF_THEN_CALL("arccos", std::acos);
        IF_THEN_CALL("arctan", std::atan);
        IF_THEN_CALL("arccot", [&](Value v) { return std::atan(1.0 / v.getFloat()); });
        IF_THEN_CALL("sqrt", std::sqrt);

#undef IF_THEN_CALL
    }

    Value eval_result;
};
