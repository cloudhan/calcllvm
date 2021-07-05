#include "InterpretVisitor.h"
#include "Lexer.h"
#include "Parser.h"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage:\n\t" << argv[0] << " <expr>" << std::endl;
        return -1;
    }

    Lexer lexer(argv[1]);
    Parser parser(lexer);
    auto ast = parser.parse();
    InterpretVisitor eval;
    ast->accept(eval);

    if (eval.eval_result.isInt()) {
        std::cout << eval.eval_result.getInt() << std::endl;
    } else {
        std::cout << eval.eval_result.getFloat() << std::endl;
    }
}
