#include"maxc.h"

ast_t *Ast::make(Token token) {
    ast_t *left = make_num_node(token.token_v[pos]);
    
    if(token.token_v[pos].type == TOKEN_TYPE_SYMBOL) {
        pos++;

        return make_node(token.token_v[pos], left, make(token));
    }

    return left;
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

ast_t *Ast::eval(token_t token) {
    ast_t *left = make_num_node(token);

    if(token.type == TOKEN_TYPE_SYMBOL)
        return make_node(token, left, eval(token));

    return left;
}
void Ast::show() {
    ;
}
