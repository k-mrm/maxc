#include"maxc.h"

void Ast::make(Token token) {
    for(int pos = 0; token.token_v[pos].type != TOKEN_TYPE_END; pos++) {
        std::cout << token.token_v[pos].value << std::endl;
    }
}

ast_t *Ast::make_node(token_t token, ast_t *left, ast_t *right) {
    ;
}

ast_t *Ast::make_num_node(token_t token) {
    ;
}

void Ast::show() {
    ;
}
