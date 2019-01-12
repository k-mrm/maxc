#include"maxc.h"

Ast *Parser::run(Token _token) {
    token = _token;
    Ast *ast = expr_add();
    //show(ast);

    return ast;
}

Ast *Parser::statement() {
    ;
}

Ast_v Parser::eval(std::vector<token_t> tokens) {
    Ast_v program;
}

Ast *var_decl() {
    std::vector<var_t *> decls;
}

Ast *Parser::expr_num(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        fprintf(stderr, "[error] not a number");
        exit(1);
    }
    return new Node_number(token.value);
}


Ast *Parser::expr_add() {
    Ast *left = expr_mul();

    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("+")) {
            token.step();
            left = new Node_binop("+", left, expr_mul());
        }
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("-")) {
            token.step();
            left = new Node_binop("-", left, expr_mul());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_mul() {
    Ast *left = expr_primary();

    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("*")) {
            token.step();
            left = new Node_binop("*", left, expr_primary());
        }
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("/")) {
            token.step();
            left = new Node_binop("/", left, expr_primary());
        }
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("%")) {
            token.step();
            left = new Node_binop("%", left, expr_primary());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_primary() {
    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("(")) {
            token.step();
            Ast *left = expr_add();

            if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value(")")) {
                token.step();
                return left;
            }
            else {
                error(") <- not found");
            }
        }
        if(token.is_type(TOKEN_TYPE_NUM))
            return expr_num(token.get());

        error("in expr_primary: ????");
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
            error("??????");
        }
    }
}
