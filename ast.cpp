#include"maxc.h"

void Ast::make(Token token) {
    pos = 0;

    node = expr(token.token_v);
}

ast_t *Ast::make_node(std::string value, ast_t *left, ast_t *right) {
    ast_t *new_node = new ast_t;

    if(value == "+")
        new_node->type = ND_TYPE_PLUS;
    else if(value == "-")
        new_node->type = ND_TYPE_MINUS;

    new_node->left = left;
    new_node->right = right;

    return new_node;
}

ast_t *Ast::make_num_node(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        std::cerr << "This is not number: " << token.value << std::endl;;
        exit(1);
    }
    ast_t *new_node = new ast_t;

    new_node->type = ND_TYPE_NUM;
    new_node->value = token.value;
    new_node->left = nullptr;
    new_node->right = nullptr;

    return new_node;
}

ast_t *Ast::expr(std::vector<token_t> tokens) {
    ast_t *left = make_num_node(tokens[pos++]);

    //print_pos("aaa");

    if(tokens[pos].type == TOKEN_TYPE_SYMBOL) {
        if(tokens[pos].value == "+") {
            pos++;
            return make_node("+", left, expr(tokens));
        }
        else if(tokens[pos].value == "-") {
            pos++;
            return make_node("-", left, expr(tokens));
        }
    }

    return left;
}
/* 
ast_t *Ast::mul_expr(std::vector<token_t> tokens) {
    ast_t *left = make_num_node(tokens[pos++]);

    if(tokens[pos].type)
}
*/

void Ast::show() {
    if(node->type == ND_TYPE_NUM) {
        std::cout << node->value << std::endl;

        return ;
    }

    std::string type = ret_type(node->type);
    std::cout << type << std::endl;

    show(node->left);
    show(node->right);

    /*
    if(node->type == ND_TYPE_PLUS)
        std::cout << "+" << std::endl;
    else if(node->right->type == ND_TYPE_MINUS)
        std::cout << "-" << std::endl;
    std::cout << node->left->value << std::endl;
    */
}

void Ast::show(ast_t *current) {
    if(current->type == ND_TYPE_NUM) {
        std::cout << current->value << std::endl;
        return;
    }

    std::string type = ret_type(current->type);
    std::cout << type << std::endl;

    show(current->left);
    show(current->right);
}

void Ast::print_pos(std::string msg) {
    std::cout << msg << ": " << pos << std::endl;
}

std::string Ast::ret_type(nd_type ty) {
    switch(ty) {
        case ND_TYPE_PLUS:
            return "+";
        case ND_TYPE_MINUS:
            return "-";
        default:
            printf("???\n");
            exit(1);
    }
}
