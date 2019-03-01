#include"maxc.h"

Ast_v Parser::run(Token _token) {
    token = _token;
    env.current = new env_t(true);
    Ast_v program = eval();

    return program;
}

Ast_v Parser::eval() {
    Ast_v program;

    while(!token.is_type(TOKEN_TYPE::END)) {
        program.push_back(statement());

        token.skip(";");
    }
    return program;
}

Ast *Parser::statement() {
    if(token.is_value(";")) {
        token.step();
        return statement();
    }
    else if(token.skip("{"))
        return make_block();
    else if(token.is_value("if"))
        return make_if();
    else if(token.is_value("for"))
        return make_for();
    else if(token.is_value("while"))
        return make_while();
    else if(token.is_value("return")) {
        token.step();
        return make_return();
    }
    else if(token.is_value("print")) {
        token.step();
        return make_print();
    }
    else if(token.is_value("println")) {
        token.step();
        return make_println();
    }
    else if(is_func_def())
        return func_def();
    else if(is_func_proto())
        return func_proto();
    else if(is_var_decl())
        return var_decl();
    else
        return expr();
}

Ast *Parser::expr() {
    /*
    if(token.is_type(TOKEN_TYPE::IDENTIFER)) {
        if(token.see(1).value == "=")
            return assignment();
        else
            return expr_first();
    }
    */
    return expr_first();
}

Ast *Parser::expr_first() {
    return expr_assign();
}

Ast *Parser::func_def() {
    Type *ty = eval_type();
    std::string name = token.get().value;
    token.step();

    if(token.abs_skip("(")) {
        env.make();
        Varlist args;
        var_t ainfo;

        while(!token.skip(")")) {
            Type *arg_ty = eval_type();
            std::string arg_name = token.get().value;

            ainfo = (var_t){arg_ty, arg_name};
            Node_variable *a = new Node_variable(ainfo, false);
            args.push(a);
            env.get()->vars.push(a);
            vls.push(a);

            token.step();
            token.skip(",");
        }
        token.abs_skip("{");
        Ast_v b;
        while(!token.skip("}")) {
            b.push_back(statement());
            token.skip(";");
        }

        Ast *t = new Node_func_def(ty, name, args, b, vls);
        vls.reset();
        env.escape();
        return t;
    }

    return nullptr;
}

Ast *Parser::func_call() {
    std::string name = token.get().value;
    token.step();
    if(token.skip("(")) {
        Ast_v args;

        while(!token.skip(")")) {
            args.push_back(expr_first());
            //token.step();
            token.skip(",");
        }

        return new Node_func_call(name, args);
    }
    else {
        error("error");
        return nullptr;
    }
}

Ast *Parser::func_proto() {
    Type *retty = eval_type();
    std::string name = token.get().value;
    token.step();
    token.abs_skip("(");
    Type_v tys;

    while(!token.skip(")")) {
        Type *argty = eval_type();
        if(token.get().type == TOKEN_TYPE::IDENTIFER)
            token.step();
        tys.push_back(argty);
        token.skip(",");
    }
    return new Node_func_proto(retty, name, tys);
}

Ast *Parser::var_decl() {
    var_t info;
    Type *ty = eval_type();
    for(int i = 0; i < skip_ptr(); i++)
        ty = new Type(ty);
    Ast_v init;
    bool isglobal = env.isglobal();
    Varlist v;

    while(1) {
        debug("call var_decl\n");
        std::string name = token.get().value;
        token.step();

        if(token.skip("=")) {
            debug("call initialize\n");
            init.push_back(expr_first());
        }
        else init.push_back(nullptr);

        info = (var_t){ty, name};
        Node_variable *var = new Node_variable(info, isglobal);
        v.push(var);
        env.get()->vars.push(var);  debug("push env vlist: %s\n", info.name.c_str());
        vls.push(var);

        if(token.skip(";")) break;
        token.abs_skip(",");
    }

    debug("return vardeclAST\n");
    return new Node_vardecl(v, init);
}

Type *Parser::eval_type() {
    if(token.is_value("int")) {
        token.step();
        return new Type(CTYPE::INT);
    }
    else if(token.is_value("void")) {
        token.step();
        return new Type(CTYPE::VOID);
    }
    else if(token.is_value("char")) {
        token.step();
        return new Type(CTYPE::CHAR);
    }
    else {
        error("eval_type ?????");
        return nullptr;
    }
}

Ast *Parser::make_assign(Ast *dst, Ast *src) {
    if(!dst)
        return nullptr;
    assert(dst->get_nd_type() == NDTYPE::VARIABLE);
    return new Node_assignment(dst, src);
}

Ast *Parser::make_block() {
    Ast_v cont;
    env.make();
    while(!token.skip("}")) {
        Ast *b = statement();
        token.skip(";");
        cont.push_back(b);
    }

    env.escape();
    return new Node_block(cont);
}

Ast *Parser::make_if() {
    if(token.skip("if")) {
        token.skip("(");
        Ast *cond = expr();
        token.skip(")");
        Ast *then = statement();
        token.skip(";");

        if(token.skip("else")) {
            Ast *el = statement();

            return new Node_if(cond, then, el);
        }

        return new Node_if(cond, then, nullptr);
    }
    else
        return nullptr;
}

Ast *Parser::expr_if() {
    token.abs_skip("if");
    token.abs_skip("(");
    Ast *cond = expr();
    token.abs_skip(")");
    Ast *then = statement();
    token.skip(";");

    if(token.skip("else")) {
        Ast *el = statement();

        return new Node_exprif(cond, then, el);
    }
    return new Node_exprif(cond, then, nullptr);
}

Ast *Parser::make_for() {
    if(token.skip("for")) {
        token.skip("(");
        Ast *init = expr();
        token.abs_skip(";");
        Ast *cond = expr();
        token.abs_skip(";");
        Ast *reinit = expr();
        token.abs_skip(")");
        Ast *body = statement();

        return new Node_for(init, cond, reinit, body);
    }
    return nullptr;
}

Ast *Parser::make_while() {
    if(token.skip("while")) {
        token.skip("(");
        Ast *cond = expr();
        token.skip(")");
        Ast *body = statement();

        return new Node_while(cond, body);
    }
    return nullptr;
}

Ast *Parser::make_return() {
    Node_return *r = new Node_return(expr_first());
    token.skip(";");
    return r;
}

Ast *Parser::make_print() {
    token.abs_skip("(");
    Ast *c = expr();
    token.abs_skip(")");

    return new Node_print(c);
}

Ast *Parser::make_println() {
    token.abs_skip("(");
    Ast *c = expr();
    token.abs_skip(")");

    return new Node_println(c);
}

Ast *Parser::expr_num(token_t token) {
    if(token.type != TOKEN_TYPE::NUM) {
        error("not a number: %s", token.value.c_str());
    }
    return new Node_number(atoi(token.value.c_str()));
}

Ast *Parser::expr_char(token_t token) {
    assert(token.type == TOKEN_TYPE::CHAR);
    assert(token.value.length() == 1);
    char c = token.value[0];
    return new Node_char(c);
}

Ast *Parser::expr_var(token_t tk) {
    /*if(env.get()->vars.var_v.empty()) {
        debug("empty\n");
        goto verr;
    }*/
    for(env_t *e = env.get(); ; e = e->parent) {
        if(!e->vars.get().empty())
            break;
        if(e->isglb) {
            debug("empty\n");
            goto verr;
        }
    }

    env.get()->vars.show();
    for(env_t *e = env.get(); ; e = e->parent) {
        for(auto v: e->vars.get()) {
            if(v->vinfo.name == tk.value) {
                debug("%s found\n", tk.value.c_str());
                return v;
            }
        }
        if(e->isglb) {
            debug("it is glooobal\n");
            goto verr;
        }
    }

verr:
    error("undeclared variable: %s\n", tk.value.c_str());
    return nullptr;
}

Ast *Parser::expr_assign() {
    Ast *left = expr_logic_or();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("=")) {
            token.step();
            left = make_assign(left, expr_logic_and());
        }
        else
            return left;
    }
}

Ast *Parser::expr_logic_or() {
    Ast *left = expr_logic_and();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("||")) {
            token.step();
            left = new Node_binop("||", left, expr_logic_and());
        }
        else if(token.is_value("or")) {
            token.step();
            left = new Node_binop("||", left, expr_logic_and());
        }
        else
            return left;
    }
}

Ast *Parser::expr_logic_and() {
    Ast *left = expr_equality();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("&&")) {
            token.step();
            left = new Node_binop("&&", left, expr_equality());
        }
        else if(token.is_value("and")) {
            token.step();
            left = new Node_binop("&&", left, expr_equality());
        }
        else
            return left;
    }
}

Ast *Parser::expr_equality() {
    Ast *left = expr_comp();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("==")) {
            token.step();
            left = new Node_binop("==", left, expr_comp());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("!=")) {
            token.step();
            left = new Node_binop("!=", left, expr_comp());
        }
        else
            return left;
    }
}

Ast *Parser::expr_comp() {
    Ast *left = expr_add();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("<")) {
            token.step();
            left = new Node_binop("<", left, expr_add());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value(">")) {
            token.step();
            left = new Node_binop(">", left, expr_add());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("<=")) {
            token.step();
            left = new Node_binop("<=", left, expr_add());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value(">=")) {
            token.step();
            left = new Node_binop(">=", left, expr_add());
        }
        else
            return left;
    }
}

Ast *Parser::expr_add() {
    Ast *left = expr_mul();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("+")) {
            token.step();
            left = new Node_binop("+", left, expr_mul());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("-")) {
            token.step();
            left = new Node_binop("-", left, expr_mul());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_mul() {
    Ast *left = expr_unary();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("*")) {
            token.step();
            left = new Node_binop("*", left, expr_unary());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("/")) {
            token.step();
            left = new Node_binop("/", left, expr_unary());
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("%")) {
            token.step();
            left = new Node_binop("%", left, expr_unary());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_unary() {
    token.save();

    if(token.is_value("++") || token.is_value("--") || token.is_value("&") ||
       token.is_value("!")){
        std::string op = token.get().value;
        token.step();
        Ast *operand = expr_unary();
        debug("call unary: %s\n", op.c_str());
        return new Node_unaop(op, operand);
    }
    else if(token.is_value("*")) {
        token.step();
        Ast *operand = expr_unary();
        //Node_variable *v = (Node_variable *)operand;
        assert(operand->ctype->get().type == CTYPE::PTR);
        return new Node_unaop("*", operand);
    }

    token.rewind();
    return expr_unary_postfix();
}

Ast *Parser::expr_unary_postfix() {
    //TODO
    return expr_primary();
}

Ast *Parser::expr_primary() {
    while(1) {
        if(token.is_value("if"))
            return expr_if();
        else if(is_func_call())
            return func_call();
        else if(token.is_type(TOKEN_TYPE::IDENTIFER)) {
            Ast *v = expr_var(token.get_step());
            if(v != nullptr)
                return v;
            else
                return nullptr;
        }
        else if(token.is_type(TOKEN_TYPE::NUM))
            return expr_num(token.get_step());
        else if(token.is_type(TOKEN_TYPE::CHAR))
            return expr_char(token.get_step());
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("(")) {
            token.step();
            Ast *left = expr_first();

            if(token.abs_skip(")"))
                return left;

            return nullptr;
        }
        else if(token.is_value(";"))
            return nullptr;
        else if(token.is_value(")"))
            return nullptr;

        error("in expr_primary func: \" %s \"\n", token.get_step().value.c_str());
        return nullptr;
    }
}

bool Parser::is_func_def() {
    if(token.is_type()) {
        token.save();
        token.step();
        token.step();

        if(token.skip("(")) {
            while(!token.is_value(")"))
                token.step();
            token.skip(")");
            if(token.skip("{")) {
                token.rewind();

                return true;
            }
        }
        token.rewind();

        return false;
    }
    else
        return false;
}

bool Parser::is_func_call() {
    if(token.is_type(TOKEN_TYPE::IDENTIFER)) {
        token.save();
        token.step();
        if(token.is_value("(")) {
            token.rewind();
            return true;
        }
        else {
            token.rewind();
            return false;
        }
    }
    else
        return false;
}

bool Parser::is_func_proto() {
    if(token.is_type()) {
        token.save();
        token.step();
        token.step();
        if(token.skip("(")) {
            while(!token.is_value(")"))
                token.step();
            token.skip(")");
            if(token.skip(";")) {
                token.rewind();

                return true;
           }
        }
        token.rewind();

        return false;
    }
    else
        return false;
}

bool Parser::is_var_decl() {
    if(token.is_type()) {
        token.save();
        token.step();
        skip_ptr();

        if(token.is_type(TOKEN_TYPE::IDENTIFER)) {
            token.rewind();

            return true;
        }
        token.rewind();

        return false;
    }
    else
        return false;
}

int Parser::skip_ptr() {
    int c = 0;
    while(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("*")) {
        token.step(); c++;
    }
    debug("pointer %d\n", c);

    return c;
}

void Parser::show(Ast *ast) {
    /*
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case ND_TYPE_NUM: {
                Node_number *n = (Node_number *)ast;
                std::cout << n->number << " ";
                break;
            }
            case ND_TYPE_BINARY: {
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
                printf("var_decl: ");
                for(auto decl: v->decl_v)
                    std::cout << "(" << decl.type->show() << ", " << decl.name << ")";
                break;
            }
            case ND_TYPE_ASSIGNMENT: {
                Node_assignment *a = (Node_assignment *)ast;
                printf("(= (");
                show(a->dst);
                printf(") (");
                show(a->src);
                printf("))");
                break;
            }
            case ND_TYPE_IF: {
                Node_if *i = (Node_if *)ast;
                printf("(if ");
                show(i->cond);
                printf("(");
                show(i->then_s);
                printf(")");
                if(i->else_s) {
                    printf("(else ");
                    show(i->else_s);
                    printf(")");
                }
                printf(")");
                break;
            }
            case ND_TYPE_WHILE: {
                Node_while *w = (Node_while *)ast;
                printf("(while ");
                show(w->cond);
                printf("(");
                show(w->body);
                printf("))");
                break;
            }
            case ND_TYPE_BLOCK: {
                Node_block *b = (Node_block *)ast;
                for(Ast *c: b->cont) {
                    show(c);
                    puts("");
                }
                break;
            }
            case ND_TYPE_RETURN: {
                Node_return *r = (Node_return *)ast;
                printf("return: ");
                show(r->cont);
                puts("");
                break;
            }
            case ND_TYPE_FUNCDEF: {
                Node_func_def *f = (Node_func_def *)ast;
                printf("func-def: (");
                std::cout << f->name << "(";
                for(auto a: f->args)
                    std::cout << "(" << a.type->show() << "," << a.name << ")";
                std::cout << ") -> " << f->ret_type->show() << "(" << std::endl;
                for(Ast *b: f->block) {
                    show(b);
                    puts("");
                }
                printf("))");
                break;
            }
            case ND_TYPE_FUNCCALL: {
                Node_func_call *f = (Node_func_call *)ast;
                printf("(func-call: (");
                std::cout << f->name << "(" << std::endl;
                for(Ast *a: f->arg_v) {
                    show(a);
                    puts("");
                }
                printf("))");
                break;
            }
            case ND_TYPE_VARIABLE: {
                Node_variable *v = (Node_variable *)ast;
                printf("(var: ");
                std::cout << v->name << ")";
                break;
            }
            default: {
                fprintf(stderr, "error show\n");
            }
        }
    }
    */
}
