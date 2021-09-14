#include "AST.h"
#include "Lexer.h"
#include "Parser.h"
#include "ToIRVisitor.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/raw_ostream.h>

namespace cl = llvm::cl;

static cl::opt<std::string> input("input", cl::desc("expr"), cl::Positional, cl::Required);
static cl::opt<std::string> output("o", cl::desc("Specify output filename"), cl::value_desc("filename"), cl::Optional);

class Compiler {
    llvm::LLVMContext& ctx;

public:
    Compiler(llvm::LLVMContext& ctx)
        : ctx(ctx) {}

    std::shared_ptr<llvm::Module> compile(AST* ast) {
        auto mod = doCompile(ast);
        mod->print(llvm::outs(), nullptr);
        return mod;
    }

    std::shared_ptr<llvm::Module> compile(AST* ast, const std::string& filename) {
        auto mod = doCompile(ast);
        std::error_code ec;
        llvm::raw_fd_ostream f(filename, ec);
        mod->print(f, nullptr);
        return mod;
    }

private:
    std::shared_ptr<llvm::Module> doCompile(AST* ast) {
        auto mod = std::make_shared<llvm::Module>("expr", ctx);
        ToIRVisitor toIR(mod);
        toIR.create_main_function(ast);
        return mod;
    }
};

int main(int argc, char* argv[]) {
    llvm::InitLLVM initLLVM(argc, argv);
    cl::ParseCommandLineOptions(argc, argv, "A calculator based on LLVM.");

    llvm::LLVMContext ctx{};

    Lexer lexer(input);
    Parser parser(lexer);
    AST* expr = parser.parse();
    Compiler compiler(ctx);
    if (output.empty()) {
        compiler.compile(expr);
    }
    else {
        compiler.compile(expr, output);
    }
    return 0;
}
