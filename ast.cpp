#include"maxc.h"

node_t *Ast::make_node(token_t token, node_t *left, node_t *right) {
    std::cout << token.value << std::endl;
    node_t *node = new node_t;

    if(token.type == TOKEN_TYPE_SYMBOL) {
        if(token.value == "+") {
            node->type = ND_TYPE_PLUS;
            node->left = left;
            node->right = right;

            return node;
        }
        else if(token.value == "-") {
            node->type = ND_TYPE_MINUS;
            node->left = left;
            node->right = right;

            return node;
        }
    }
}

node_t *Ast::make_num_node(token_t token) {
    node_t *node = new node_t;

    node->type = ND_TYPE_NUM;
    node->left = nullptr;
    node->right = nullptr;
    node->value = token.value;

    return node;
}

void Ast::show() {
    ;
}
