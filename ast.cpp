#include"maxc.h"

void Ast::make(Token token) {
    for(int pos = 0; token.token_v[pos].type != TOKEN_TYPE_END; pos++) {
        std::cout << token.token_v[pos].value << std::endl;
        eval(token.token_v[pos]);
    }
}

ast_t *Ast::make_node(token_t token, ast_t *left, ast_t *right) {
    ;
}

ast_t *Ast::make_num_node(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        fprintf(stderr, "This is not number: %s", token.value.c_str());
        exit(1);
    }
    ast_t *new_node = new ast_t;

    new_node->type = ND_TYPE_NUM;
    new_node->value = token.value;

    return new_node;
}

void Ast::eval(token_t token) {
    ast_t *left = make_num_node(token);
}
void Ast::show() {
    ;
}
