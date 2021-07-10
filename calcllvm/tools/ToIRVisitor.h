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

        llvm::FunctionType* mainFuncType =
            llvm::FunctionType::get(i32, {i32, i8->getPointerTo()->getPointerTo()}, /*isVarArg=*/false);
        llvm::Function* mainFunc =
            llvm::Function::Create(mainFuncType, llvm::GlobalValue::ExternalLinkage, "main", mod.get());
        llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(ctx, "entry", mainFunc);
        irBuilder.SetInsertPoint(basicBlock);

        expr->accept(*this);

        irBuilder.CreateRet(llvm::ConstantInt::get(i32, 0, true));
    }

    void visit(Factor& e) override {
        if (e.getKind() != Factor::IDENT) {
            throw std::runtime_error("compile error, visited factor should only be identifier");
        }
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

        // prompt to float
        if (lhsType != rhsType) {
            if (rhsType == ResultType::INT) {
                lhsType = ResultType::FLOAT;
                lhs = irBuilder.CreateFPCast(lhs, f64);
            }

            if (rhsType == ResultType::INT) {
                rhsType = ResultType::FLOAT;
                rhs = irBuilder.CreateFPCast(rhs, f64);
            }
        }

        auto op = e.getOp();
        if (op == BinaryOp::PLUS) {
            if (lhsType == ResultType::FLOAT) {
                result = irBuilder.CreateFAdd(lhs, rhs);
            }
            else {
                result = irBuilder.CreateAdd(lhs, rhs);
            }
        }


    }

    void visit(FuncCall& e) override {}

    void visit(Number& e) override {
        if (e.getType() == Number::INT) {
            int64_t v;
            e.getValueLiteralStr().getAsInteger(10, v);
            result = llvm::ConstantInt::get(i64, v);
            result_type = ResultType::INT;
        } else if (e.getType() == Number::FLOAT) {
            double v;
            e.getValueLiteralStr().getAsDouble(v);
            result = llvm::ConstantFP::get(f64, v);
            result_type = ResultType::FLOAT;
        }
    }
};
