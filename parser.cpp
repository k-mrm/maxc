#include"maxc.h"

Ast *Parser::run(Token token) {
    Ast *ast = expr_add(token.token_v);
    //show(ast);

    return ast;
}

Ast *Parser::expr_num(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        fprintf(stderr, "[error] not a number");
        exit(1);
    }
    return new Node_number(token.value);
}

Ast *Parser::expr_add(std::vector<token_t> tokens) {
    Ast *left = expr_mul(tokens);

    while(1) {
        if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "+") {
            pos++;
            left = new Node_binop("+", left, expr_mul(tokens));
        }
        if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "-") {
            pos++;
            left = new Node_binop("-", left, expr_mul(tokens));
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_mul(std::vector<token_t> tokens) {
    Ast *left = expr_primary(tokens);

    while(1) {
        if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "*") {
            pos++;
            left = new Node_binop("*", left, expr_primary(tokens));
        }
        else if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "/") {
            pos++;
            left = new Node_binop("/", left, expr_primary(tokens));
        }
        else if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "%") {
            pos++;
            left = new Node_binop("%", left, expr_primary(tokens));
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_primary(std::vector<token_t> tokens) {
    while(1) {
        if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == "(") {
            pos++;
            Ast *left = expr_add(tokens);

            if(tokens[pos].type == TOKEN_TYPE_SYMBOL && tokens[pos].value == ")") {
                pos++;
                return left;
            }
            else {
                fprintf(stderr, "[error] ) <- not found");
                exit(1);
            }
        }
        if(tokens[pos].type == TOKEN_TYPE_NUM)
            return expr_num(tokens[pos++]);

        fprintf(stderr, "expr_primary: ????");
        exit(1);
    }
}

void Parser::show(Ast *ast) {
    switch(ast->get_nd_type()) {
        case ND_TYPE_NUM: {
            Node_number *n = (Node_number *)ast;
            std::cout << n->number << " ";
            break;
        }
        case ND_TYPE_SYMBOL: {
            Node_binop *b = (Node_binop *)ast;
            printf("(");
            std::cout << b->symbol << " ";
            show(b->left);
            show(b->right);
            printf(")");
            break;
        }
        default: {
            fprintf(stderr, "???????");
            break;
        }
    }
}
