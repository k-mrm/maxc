#include"maxc.h"

void Parser::run(Token token) {
    Ast ast;
    ast.make(token);
    std::cout << ast.node->left->value << std::endl;
    std::cout << ast.node->right->value << std::endl;
}

void Parser::eval() {
}
