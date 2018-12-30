#include"maxc.h"

void Parser::run(Token token) {
    Ast ast;
    ast.node = ast.make(token);
}

void Parser::eval() {
}
