#pragma once

#include "AST.h"
#include "Lexer.h"

#include "llvm/IR/IRBuilder.h"

class ToIRVisitor : public ASTVisitor {
    std::shared_ptr<llvm::Module> mod;
    llvm::IRBuilder<> irBuilder;

    enum class ResultType {
        INT,
        FLOAT,
    } result_type;

    llvm::Value* result;
    llvm::Type* i64;
    llvm::Type* f64;

public:
    ToIRVisitor(std::shared_ptr<llvm::Module> mod)
        : mod(mod)
        , irBuilder(mod->getContext()) {
        auto& ctx = mod->getContext();
        i64 = llvm::Type::getInt64Ty(ctx);
        f64 = llvm::Type::getDoubleTy(ctx);
    }

    void create_main_function(AST* expr) {
        auto& ctx = mod->getContext();
        auto i32 = llvm::Type::getInt32Ty(ctx);
        auto i8 = llvm::Type::getInt8Ty(ctx);

        auto i8PtrPtr = i8->getPointerTo()->getPointerTo();
        auto mainFuncType = llvm::FunctionType::get(i32, {i32, i8PtrPtr}, /*isVarArg=*/false);
        auto mainFunc = llvm::Function::Create(mainFuncType, llvm::GlobalValue::ExternalLinkage, "main", mod.get());
        auto basicBlock = llvm::BasicBlock::Create(ctx, "entry", mainFunc);
        irBuilder.SetInsertPoint(basicBlock);

        expr->accept(*this);

        decltype(f64) inputType;
        std::string funcName;
        if (result_type == ResultType::FLOAT) {
            inputType = f64;
            funcName = "outputf";
        } else {
            inputType = i64;
            funcName = "outputi";
        }
        auto outputFuncType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {inputType}, false);
        auto outputFunc =
            llvm::Function::Create(outputFuncType, llvm::GlobalValue::ExternalLinkage, funcName, mod.get());
        irBuilder.CreateCall(outputFuncType, outputFunc, result);

        irBuilder.CreateRet(llvm::ConstantInt::get(i32, 0, true));
    }

    void visit(UnaryOp& e) override {
        e.getExpr()->accept(*this);

        if (e.getOp() == UnaryOp::POS) {
            // do nothing
        } else if (e.getOp() == UnaryOp::NEG) {
            if (result_type == ResultType::FLOAT) {
                result = irBuilder.CreateFNeg(result);
            } else {
                result = irBuilder.CreateNeg(result);
            }
        } else if (e.getOp() == UnaryOp::FACT) {
            if (result_type == ResultType::FLOAT) {
                throw std::runtime_error("ToIR: factorial of float is not defined");
            } else {
                throw std::runtime_error("ToIR: NotImplemented");
            }
        } else {
            throw std::runtime_error("ToIR: unknown unary op");
        }
    }

    void visit(BinaryOp& e) override {
        e.getLeft()->accept(*this);
        llvm::Value* lhs = result;
        auto lhsType = result_type;
        e.getRight()->accept(*this);
        llvm::Value* rhs = result;
        auto rhsType = result_type;

        // prompt to f64, only i64 to f64 is allowed
        if (lhsType != rhsType) {
            if (lhsType == ResultType::INT) {
                lhsType = ResultType::FLOAT;
                lhs = irBuilder.CreateSIToFP(lhs, f64);
            }

            if (rhsType == ResultType::INT) {
                rhsType = ResultType::FLOAT;
                rhs = irBuilder.CreateSIToFP(rhs, f64);
            }
        }

        auto op = e.getOp();

#define IF_OP_THEN(bop, float_func, int_func)                                                                          \
    if (op == bop) {                                                                                                   \
        if (lhsType == ResultType::FLOAT) {                                                                            \
            result = irBuilder.float_func(lhs, rhs);                                                                   \
        } else {                                                                                                       \
            result = irBuilder.int_func(lhs, rhs);                                                                     \
        }                                                                                                              \
        return;                                                                                                        \
    }

        IF_OP_THEN(BinaryOp::PLUS, CreateFAdd, CreateAdd);
        IF_OP_THEN(BinaryOp::MINUS, CreateFSub, CreateSub);
        IF_OP_THEN(BinaryOp::MUL, CreateFMul, CreateMul);
        IF_OP_THEN(BinaryOp::DIV, CreateFDiv, CreateSDiv);

        if (op == BinaryOp::MOD) {
            if (lhsType == ResultType::FLOAT) {
                throw std::runtime_error("mod only works on integer");
            } else {
                result = irBuilder.CreateSRem(lhs, rhs);
            }
        }

        if (op == BinaryOp::POW) {
            if (lhsType == ResultType::FLOAT) {
                auto powFuncType = llvm::FunctionType::get(f64, {f64, f64}, false);
                auto powFunc =
                    llvm::Function::Create(powFuncType, llvm::GlobalValue::ExternalLinkage, "powi", mod.get());
                result = irBuilder.CreateCall(powFuncType, powFunc, {lhs, rhs});
                result_type = ResultType::FLOAT;
            } else {
                throw std::runtime_error("NotImplemented");
            }
        }
    }

    void visit(FuncCall& e) override {}

    void visit(Ident& e) override {}

    void visit(Number& e) override {
        if (e.getType() == Number::INT) {
            int64_t v;
            e.getValue().getAsInteger(10, v);
            result = llvm::ConstantInt::get(i64, v);
            result_type = ResultType::INT;
        } else if (e.getType() == Number::FLOAT) {
            double v;
            e.getValue().getAsDouble(v);
            result = llvm::ConstantFP::get(f64, v);
            result_type = ResultType::FLOAT;
        }
    }
};
