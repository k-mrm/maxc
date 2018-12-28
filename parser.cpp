#include"maxc.h"

void Parser::run(Token token) {
    Ast ast;
    for(token_t tk: token.token_v) {
        ast.make_node();
    }
}
