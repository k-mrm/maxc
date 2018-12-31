#include"maxc.h"

void *Ast::make(Token token) {
    pos = 0;

    node = eval(token.token_v);
}

ast_t *Ast::make_node(token_t token, ast_t *left, ast_t *right) {
    ast_t *new_node = new ast_t;

    if(token.value == "+")
        new_node->type = ND_TYPE_PLUS;
    else if(token.value == "-")
        new_node->type = ND_TYPE_MINUS;
    
    new_node->left = left;
    new_node->right = right;

    return new_node;
}

ast_t *Ast::make_num_node(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        fprintf(stderr, "This is not number: %s", token.value.c_str());
        exit(1);
    }
    ast_t *new_node = new ast_t;

    new_node->type = ND_TYPE_NUM;
    new_node->value = token.value;
    new_node->left = nullptr;
    new_node->right = nullptr;

    return new_node;
}

ast_t *Ast::eval(std::vector<token_t> tokens) {
    ast_t *left = make_num_node(tokens[pos++]);

    if(tokens[pos].type == TOKEN_TYPE_SYMBOL) {
        pos++;

        return make_node(tokens[pos], left, eval(tokens));
    }

    return left;
}

void Ast::show() {
    ;
}
