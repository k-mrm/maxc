#include"maxc.h"

Ast_v Parser::run(Token _token) {
    token = _token;
    //Ast *ast = statement();
    Ast_v program = eval();
    for(Ast *ast: program) {
        show(ast);
        puts("");
    }

    return program;
}

Ast *Parser::statement() {
    if(token.is_value("var"))
        return var_decl();
    else if(token.is_value(";"))
        token.step();
    else
        return expr_add();
}


Ast_v Parser::eval() {
    Ast_v program;

    token.skip(";");

    while(!token.is_type(TOKEN_TYPE_END)) {
        program.push_back(statement());

        token.skip(";");
    }
    return program;
}


Ast *Parser::var_decl() {
    std::vector<var_t> decls;
    var_type ty = eval_type();

    while(!token.skip(";")) {
        std::string name = token.get().value;
        decls.push_back((var_t){ty, name});

        token.step();
    }

    return new Node_var_decl(decls);
}

var_type Parser::eval_type() {
    if(token.is_value("var")) {
        token.step();
        return TYPE_INT;
    }
    else
        error("eval_type ?????");
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
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("-")) {
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
            return expr_num(token.get_step());

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
        case ND_TYPE_VARDECL: {
            Node_var_decl *v = (Node_var_decl *)ast;
            printf("var ");
            for(auto decl: v->decl_v)
                std::cout << "(" << decl.type << ", " << decl.name << ")";
            break;
        }
        default: {
            error("??????");
        }
    }
}
