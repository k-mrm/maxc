#include"maxc.h"

void Parser::run(Token token) {
    Ast ast;
    ast.make(token);
    std::cout << ast.node->type << std::endl;
}

void Parser::eval() {
}
