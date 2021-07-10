#include "AST.h"
#include "Lexer.h"
#include "Parser.h"
#include "ToIRVisitor.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/raw_ostream.h>

namespace cl = llvm::cl;

static cl::opt<std::string> input(cl::Positional, cl::desc("expr"));

class Compiler {
    llvm::LLVMContext& ctx;

public:
    Compiler(llvm::LLVMContext& ctx)
        : ctx(ctx) {}

    std::shared_ptr<llvm::Module> compile(AST* ast) {
        auto mod = std::make_shared<llvm::Module>("expr", ctx);
        ToIRVisitor toIR(mod);
        toIR.create_main_function(ast);
        mod->print(llvm::outs(), nullptr);
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
    compiler.compile(expr);
    return 0;
}
