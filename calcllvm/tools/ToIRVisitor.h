#pragma once

#include "AST.h"
#include "Lexer.h"

#include "llvm/IR/IRBuilder.h"

#include <string>
#include <unordered_map>
#include <vector>

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

    std::unordered_map<std::string, int> env; // name to index
    std::unordered_map<std::string, llvm::Function*> functions;

    llvm::BasicBlock* mainFuncPrelude;
    llvm::BasicBlock* mainFuncBody;

    std::shared_ptr<llvm::GlobalVariable> globalNumValues;
    std::shared_ptr<llvm::GlobalVariable> globalNames;

public:
    ToIRVisitor(const std::shared_ptr<llvm::Module>& mod)
        : mod(mod)
        , irBuilder(mod->getContext())
        , env() {
        auto& ctx = mod->getContext();
        i64 = llvm::Type::getInt64Ty(ctx);
        f64 = llvm::Type::getDoubleTy(ctx);
    }

    void create_main_function(AST* expr) {
        auto& ctx = mod->getContext();
        auto i32 = llvm::Type::getInt32Ty(ctx);
        auto i8 = llvm::Type::getInt8Ty(ctx);

        globalNumValues = std::make_shared<llvm::GlobalVariable>(llvm::Type::getInt32Ty(ctx), /*isConstant=*/true,
                                                                 llvm::GlobalVariable::AvailableExternallyLinkage);
        globalNames = std::make_shared<llvm::GlobalVariable>(llvm::ArrayType::get(llvm::Type::getInt8PtrTy(ctx), 128),
                                                             /*isConstant=*/true,
                                                             llvm::GlobalVariable::AvailableExternallyLinkage);

        auto i8PtrPtr = i8->getPointerTo()->getPointerTo();
        auto mainFuncType = llvm::FunctionType::get(i32, {i32, i8PtrPtr}, /*isVarArg=*/false);
        auto mainFunc = llvm::Function::Create(mainFuncType, llvm::GlobalValue::ExternalLinkage, "main", mod.get());
        mainFuncPrelude = llvm::BasicBlock::Create(ctx, "prelude", mainFunc);
        mainFuncBody = llvm::BasicBlock::Create(ctx, "body", mainFunc);
        irBuilder.SetInsertPoint(mainFuncBody);



        expr->accept(*this);

        // print the value
        llvm::Type* inputType;
        std::string funcName;
        if (result_type == ResultType::FLOAT) {
            inputType = f64;
            funcName = "print_f";
        } else {
            inputType = i64;
            funcName = "print_i";
        }
        callExternal(funcName, llvm::Type::getVoidTy(ctx), {inputType}, {result});

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
    if (op == (bop)) {                                                                                                 \
        if (lhsType == ResultType::FLOAT) {                                                                            \
            result = irBuilder.float_func(lhs, rhs);                                                                   \
            result_type = ResultType::FLOAT;                                                                           \
        } else {                                                                                                       \
            result = irBuilder.int_func(lhs, rhs);                                                                     \
            result_type = ResultType::INT;                                                                             \
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
            if (lhsType == ResultType::INT && rhsType == ResultType::INT) {
                result = callExternal("powi", i64, {i64, i64}, {lhs, rhs});
                result_type = ResultType::INT;
            } else {
                result = callExternal("pow", f64, {f64, f64}, {lhs, rhs});
                result_type = ResultType::FLOAT;
            }
        }
    }

    void visit(FuncCall& e) override {
        e.getParam()->accept(*this);

        auto name = e.getName();

        // only abs supports int input
        if (result_type == ResultType::INT) {
            if (name.equals("abs")) {
                result = callExternal("llabs", i64, {i64}, {result});
                return;
            } else {
                result = irBuilder.CreateSIToFP(result, f64);
                result_type = ResultType::FLOAT;
            }
        }

        static std::unordered_map<std::string, std::string> calcc_func_to_math_func{
            {"abs", "fabs"},    //
            {"exp", "exp"},     //
            {"log2", "log2"},   //
            {"lg", "log10"},    //
            {"ln", "log"},      //
            {"sin", "sin"},     //
            {"cos", "cos"},     //
            {"tan", "tan"},     //
            {"arcsin", "asin"}, //
            {"arccos", "acos"}, //
            {"arctan", "atan"}, //
            {"sqrt", "sqrt"},   //
        };
        auto it = calcc_func_to_math_func.find(name.str());
        if (it != calcc_func_to_math_func.end()) {
            result = callExternal(it->second, f64, {f64}, {result});
            return;
        }

        auto one = llvm::ConstantFP::get(f64, 1.0);
        if (name.equals("cot")) {
            result = callExternal("tan", f64, {f64}, {result});
            result = irBuilder.CreateFDiv(one, result);
            return;
        }

        if (name.equals("arccot")) {
            result = irBuilder.CreateFDiv(one, result);
            result = callExternal("atan", f64, {f64}, {result});
            return;
        }

        throw std::runtime_error("never reach");
    }

    void visit(Ident& e) override {
        auto name = e.getName().str();
        auto it = env.find(name);
        if (it == env.end()) {
            int index = env.size();
            prependReads(name, index);
            env[name] = static_cast<int>(index);
        }
    }

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

private:
    llvm::Value* callExternal(const std::string& funcName, llvm::Type* retType, llvm::ArrayRef<llvm::Type*> inType,
                              llvm::ArrayRef<llvm::Value*> input) {
        auto funcType = llvm::FunctionType::get(retType, inType, false);
        auto it = functions.find(funcName);
        llvm::Function* func;
        if (it == functions.end()) {
            func = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, funcName, mod.get());
            functions[funcName] = func;
        } else {
            func = it->second;
        }

        return irBuilder.CreateCall(funcType, func, input);
    }

    void prependReads(const std::string& name, int index) {
        irBuilder.SetInsertPoint(mainFuncPrelude);

        irBuilder.SetInsertPoint(mainFuncBody);
    }
};
