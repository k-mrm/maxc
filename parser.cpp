#include"maxc.h"

ast_t *Parser::run(Token token) {
    Ast ast;
    ast.make(token);

    return ast.node;
}

