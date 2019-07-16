#include "parser.h"
#include "error.h"

Ast_v &Parser::run() {
    env.current = new env_t(true);

    set_global();

    return eval();
}

void Parser::set_global() {
    Type *fntype = new Type(CTYPE::FUNCTION);

    std::vector<std::string> bltfns_name = {
        "print",
        "println",
    };
    std::vector<BltinFnKind> bltfns_kind = {BltinFnKind::Print,
                                            BltinFnKind::Println};

    std::vector<NodeVariable *> bltfns;

    for(size_t i = 0; i < bltfns_name.size(); ++i) {
        func_t finfo = func_t(bltfns_name[i], bltfns_kind[i], fntype);
        bltfns.push_back(new NodeVariable(finfo, true));
    }

    env.current->vars.push(bltfns);
}

Ast_v &Parser::eval() {
    while(!token.is(TKind::End)) {
        program.push_back(statement());
    }

    ngvar = env.get()->vars.get().size();

    return program;
}

Ast *Parser::statement() {
    if(token.is(TKind::Lbrace))
        return make_block();
    else if(token.skip(TKind::If))
        return make_if();
    else if(token.skip(TKind::For))
        return make_for();
    else if(token.skip(TKind::While))
        return make_while();
    else if(token.skip(TKind::Return))
        return make_return();
    else if(token.skip(TKind::Let))
        return var_decl(false);
    else if(token.skip(TKind::Const))
        return var_decl(true);
    else if(token.skip(TKind::Fn))
        return func_def();
    else if(token.skip(TKind::Typedef)) {
        make_typedef();
        return nullptr;
    }
    else
        return expr();
}

Ast *Parser::expr() {
    /*
    if(token.is(TOKEN_TYPE::IDENTIFER)) {
        if(token.see(1).value == "=")
            return assignment();
        else
            return expr_first();
    }
    */
    return expr_assign();
}

Ast *Parser::func_def() {
    std::string name = token.get().value;
    token.step();

    if(!token.expect(TKind::Lparen)) {
        return nullptr;
    }

    env.make();

    Varlist args;
    var_t arg_info;
    func_t fn_arg_info;
    Type_v argtys;

    if(!token.skip(TKind::Rparen))
        for(;;) {
            std::string arg_name = token.get().value;
            token.step();

            token.expect(TKind::Colon);

            Type *arg_ty = eval_type();
            argtys.push_back(arg_ty);

            if(arg_ty->isfunction())
                fn_arg_info = func_t(arg_name, arg_ty);
            else
                arg_info = (var_t){0, arg_ty, arg_name};

            NodeVariable *a = arg_ty->isfunction()
                                  ? new NodeVariable(fn_arg_info, false)
                                  : new NodeVariable(arg_info, false);

            args.push(a);
            env.get()->vars.push(a);
            vls.push(a);

            if(token.skip(TKind::Rparen))
                break;
            token.expect(TKind::Comma);
        }

    token.expect(TKind::Colon);

    Type *ret_ty = eval_type();
    Type *fntype = new Type(CTYPE::FUNCTION);

    fntype->fnarg = argtys;
    fntype->fnret = ret_ty;

    func_t finfo = func_t(name, args, fntype);

    NodeVariable *function = new NodeVariable(finfo, env.get()->parent->isglb);

    env.current->parent->vars.push(function);

    token.expect(TKind::Lbrace);

    Ast_v block;

    for(;;) {
        block.push_back(statement());

        if(token.skip(TKind::Rbrace))
            break;
    }

    Ast *t = new NodeFunction(function, finfo, block, vls);

    vls.reset();

    env.escape();

    return t;
}

Ast *Parser::var_decl(bool isconst) {
    var_t info;
    func_t finfo;

    Ast *init;

    bool isglobal = env.isglobal();

    Type *ty;
    NodeVariable *var;

    std::string name = token.get().value;
    token.step();

    token.expect(TKind::Colon);

    ty = eval_type();

    int vattr = 0;

    if(isconst)
        vattr |= (int)VarAttr::Const;

    /*
     *  let a: int = 100;
     *             ^
     */
    if(token.skip(TKind::Assign)) {
        init = expr();
    }
    else if(isconst) {
        error(token.see(0).start, token.see(0).end, "const must initialize");

        init = nullptr;
    }
    else {
        init = nullptr;
    }

    if(ty != nullptr) {
        if(ty->isfunction())
            finfo = func_t(name, ty);
        else
            info = (var_t){vattr, ty, name};

        var = ty->isfunction() ? new NodeVariable(finfo, isglobal)
                               : new NodeVariable(info, isglobal);

        env.get()->vars.push(var);
        vls.push(var);
    }
    else {
        var = nullptr;
    }

    token.expect(TKind::Semicolon);

    return new NodeVardecl(var, init);
}

Type *Parser::eval_type() {
    Type *ty;
    if(token.skip(TKind::Lparen)) { // tuple
        ty = new Type(CTYPE::TUPLE);

        for(;;) {
            ty->tuple.push_back(eval_type());
            if(token.skip(TKind::Rparen))
                break;
            token.expect(TKind::Comma);
        }
    }
    else if(token.skip(TKind::TInt))
        ty = new Type(CTYPE::INT);
    else if(token.skip(TKind::TUint))
        ty = new Type(CTYPE::UINT);
    else if(token.skip(TKind::TInt64))
        ty = new Type(CTYPE::INT64);
    else if(token.skip(TKind::TUint64))
        ty = new Type(CTYPE::UINT64);
    else if(token.skip(TKind::TBool))
        ty = new Type(CTYPE::BOOL);
    else if(token.skip(TKind::TChar))
        ty = new Type(CTYPE::CHAR);
    else if(token.skip(TKind::TString))
        ty = new Type(CTYPE::STRING);
    else if(token.skip(TKind::TFloat))
        ty = new Type(CTYPE::DOUBLE);
    else if(token.skip(TKind::TNone)) // TODO :only function rettype
        ty = new Type(CTYPE::NONE);
    else if(token.skip(TKind::Fn)) {
        ty = new Type(CTYPE::FUNCTION);

        token.expect(TKind::Lparen);

        while(!token.skip(TKind::Rparen)) {
            ty->fnarg.push_back(eval_type());
            if(token.skip(TKind::Rparen))
                break;
            token.expect(TKind::Comma);
        }

        token.expect(TKind::Colon);
        ty->fnret = eval_type();
    }
    else if(typemap.count(token.get().value) != 0) {
        ty = typemap[token.get().value];

        token.step();
    }
    else {
        error(token.get().start,
              token.get().end,
              "unknown type name: `%s`",
              token.get().value.c_str());

        token.step();

        return new Type(CTYPE::NONE);
    }

    for(;;) {
        if(token.skip2(TKind::Lboxbracket, TKind::Rboxbracket))
            ty = new Type(ty);
        else
            break;
    }

    return ty;
}

Ast *Parser::make_assign(Ast *dst, Ast *src) {
    if(!dst)
        return nullptr;

    return new NodeAssignment(dst, src);
}

Ast *Parser::make_assigneq(std::string op, Ast *dst, Ast *src) {
    return nullptr; // TODO
}

Ast *Parser::make_block() {
    token.expect(TKind::Lbrace);
    Ast_v cont;
    env.make();
    Ast *b;

    for(;;) {
        if(token.skip(TKind::Rbrace))
            break;
        b = statement();

        cont.push_back(b);
    }

    env.escape();

    return new NodeBlock(cont);
}

Ast *Parser::make_if() {
    token.expect(TKind::Lparen);
    Ast *cond = expr();
    token.expect(TKind::Rparen);
    Ast *then = make_block();
    // token.skip(TKind::Semicolon);

    if(token.skip(TKind::Else)) {
        Ast *el;

        if(token.skip(TKind::If))
            el = make_if();
        else
            el = make_block();

        return new NodeIf(cond, then, el);
    }

    return new NodeIf(cond, then, nullptr);
}

Ast *Parser::expr_if() {
    token.expect(TKind::If);
    token.expect(TKind::Lparen);
    Ast *cond = expr();
    token.expect(TKind::Rparen);
    Ast *then = statement();

    if(token.skip(TKind::Else)) {
        Ast *el = statement();

        return new NodeExprif(cond, then, el);
    }

    return new NodeExprif(cond, then, nullptr);
}

Ast *Parser::make_for() {
    token.expect(TKind::Lparen);
    Ast *init = expr();
    token.expect(TKind::Semicolon);
    Ast *cond = expr();
    token.expect(TKind::Semicolon);
    Ast *reinit = expr();
    token.expect(TKind::Rparen);
    Ast *body = statement();

    return new NodeFor(init, cond, reinit, body);
}

Ast *Parser::make_while() {
    token.expect(TKind::Lparen);
    Ast *cond = expr();
    token.expect(TKind::Rparen);
    Ast *body = statement();

    return new NodeWhile(cond, body);
}

Ast *Parser::make_return() {
    NodeReturn *ret = new NodeReturn(expr());

    token.expect(TKind::Semicolon);

    return ret;
}

void Parser::make_typedef() {
    std::string to = token.get().value;
    token.step();
    token.expect(TKind::Assign);
    Type *from = eval_type();
    token.expect(TKind::Semicolon);
    typemap[to] = from;
}

Ast *Parser::expr_num(token_t tk) {
    /*
    if(tk.type != TKind::Num) {
        error(token.see(-1).line, token.see(-1).col,
                "not a number: %s", tk.value.c_str());
    }
    */
    if(strchr(tk.value.c_str(), '.'))
        return new NodeNumber(atof(tk.value.c_str()));
    else
        return new NodeNumber(atoi(tk.value.c_str()));
}

Ast *Parser::expr_bool() {
    if(token.skip(TKind::True))
        return new NodeBool(true);
    if(token.skip(TKind::False))
        return new NodeBool(false);
    token.step();

    return nullptr;
}

Ast *Parser::expr_char(token_t token) {
    assert(token.value.length() == 1);
    char c = token.value[0];

    return new NodeChar(c);
}

Ast *Parser::expr_string(token_t token) { return new NodeString(token.value); }

Ast *Parser::expr_var(token_t tk) {
    for(env_t *e = env.get();; e = e->parent) {
        if(!e->vars.get().empty())
            break;
        if(e->isglb) {
            // debug("empty\n");
            goto verr;
        }
    }

    // env.get()->vars.show();
    for(env_t *e = env.get();; e = e->parent) {
        for(auto &v : e->vars.get()) {
            if(v->ctype->isfunction()) {
                if(v->finfo.name == tk.value)
                    return v;
            }
            else if(v->vinfo.name == tk.value) {
                // debug("%s found\n", tk.value.c_str());

                return v;
            }
        }
        if(e->isglb) {
            // debug("it is glooobal\n");
            goto verr;
        }
    }

verr:
    /*
    error(token.see(-1).line, token.see(-1).col,
            "undeclared variable: `%s`", tk.value.c_str());*/
    error(tk.start, tk.end, "undeclared variable: `%s`", tk.value.c_str());

    return nullptr;
}

Ast *Parser::expr_assign() {
    Ast *left = expr_ternary();

    if(token.is(TKind::Assign)) {
        if(left == nullptr) {
            return nullptr;
        }
        /*
        if(left->get_nd_type() != NDTYPE::VARIABLE && left->get_nd_type() !=
        NDTYPE::SUBSCR) { error(token.see(-1).line, token.see(-1).col, "left
        side of the expression is not valid");
        }

        ((NodeVariable *)left)->vinfo.vattr &= ~((int)VarAttr::Uninit);
        */

        token.step();
        left = make_assign(left, expr_assign());
    }

    return left;
}

Ast *Parser::expr_ternary() {
    Ast *left = expr_logic_or();

    if(!token.skip(TKind::Question))
        return left;

    Ast *then = expr();
    token.expect(TKind::Colon);
    Ast *els = expr_ternary();

    return new NodeTernop(left, then, els);
}

Ast *Parser::expr_logic_or() {
    Ast *left = expr_logic_and();
    Ast *t;

    while(1) {
        if(token.is(TKind::LogOr)) {
            token.step();
            t = expr_logic_and();
            left = new NodeBinop("||", left, t);
        }
        else if(token.is(TKind::KOr)) {
            token.step();
            t = expr_logic_and();
            left = new NodeBinop("||", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_logic_and() {
    Ast *left = expr_equality();
    Ast *t;

    while(1) {
        if(token.is(TKind::LogAnd)) {
            token.step();
            t = expr_equality();
            left = new NodeBinop("&&", left, t);
        }
        else if(token.is(TKind::KAnd)) {
            token.step();
            t = expr_equality();
            left = new NodeBinop("&&", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_equality() {
    Ast *left = expr_comp();

    while(1) {
        if(token.is(TKind::Eq)) {
            token.step();
            Ast *t = expr_comp();
            left = new NodeBinop("==", left, t);
        }
        else if(token.is(TKind::Neq)) {
            token.step();
            Ast *t = expr_comp();
            left = new NodeBinop("!=", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_comp() {
    Ast *left = expr_add();
    Ast *t;

    while(1) {
        if(token.is(TKind::Lt)) {
            token.step();
            t = expr_add();
            left = new NodeBinop("<", left, t);
        }
        else if(token.is(TKind::Gt)) {
            token.step();
            t = expr_add();
            left = new NodeBinop(">", left, t);
        }
        else if(token.is(TKind::Lte)) {
            token.step();
            t = expr_add();
            left = new NodeBinop("<=", left, t);
        }
        else if(token.is(TKind::Gte)) {
            token.step();
            t = expr_add();
            left = new NodeBinop(">=", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_add() {
    Ast *left = expr_mul();

    while(1) {
        if(token.is(TKind::Plus)) {
            token.step();
            Ast *t = expr_mul();
            left = new NodeBinop("+", left, t);
        }
        else if(token.is(TKind::Minus)) {
            token.step();
            Ast *t = expr_mul();
            left = new NodeBinop("-", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_mul() {
    Ast *left = expr_unary();
    Ast *t;

    while(1) {
        if(token.is(TKind::Asterisk)) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("*", left, t);
        }
        else if(token.is(TKind::Div)) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("/", left, t);
        }
        else if(token.is(TKind::Mod)) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("%", left, t);
        }
        else
            return left;
    }
}

Ast *Parser::expr_unary() {
    token.save();

    if(token.is(TKind::Inc) || token.is(TKind::Dec)
       /*|| token.is("&") || token.is("!") */) {
        std::string op = token.get().value;
        token.step();
        Ast *operand = expr_unary();
        /*
        if(operand->get_nd_type() != NDTYPE::VARIABLE)      //TODO subscript?
            error(
            token.see(-1).line,
            token.see(-1).col,
            "lvalue required as `%s` operand",
            op.c_str()
            );
            */

        return new NodeUnaop(op, operand, operand->ctype);
    }

    token.rewind();
    return expr_unary_postfix();
}

Ast *Parser::expr_unary_postfix() {
    Ast *left = expr_primary();

    while(1) {
        if(token.is(TKind::Dot)) {
            token.step();

            std::string method_name = token.get().value;

            /*
            if(ensure_hasmethod(left->ctype)) {
                switch(left->ctype->get().type) {
                    case CTYPE::LIST:
                        left = read_lsmethod(left); break;
                    case CTYPE::STRING:
                        left = read_strmethod(left); break;
                    case CTYPE::TUPLE:
                        left = read_tuplemethod(left); break;
                    default:
                        break;
                }
            }
            else
                return nullptr;
            */
        }
        else if(token.is(TKind::Lboxbracket)) {
            token.step();
            Ast *index = expr();
            token.expect(TKind::Rboxbracket);
            Type *ty = left->ctype->ptr;
            left = new NodeSubscript(left, index, ty);
        }
        else if(token.is(TKind::Lparen)) {
            token.step();
            Ast_v args;

            if(token.skip(TKind::Rparen))
                ;
            else {
                for(;;) {
                    args.push_back(expr());
                    if(token.skip(TKind::Rparen))
                        break;
                    token.expect(TKind::Comma);
                }
            }

            // TODO Type check
            left = new NodeFnCall(left, args);
        }
        else
            return left;
    }
}

Ast *Parser::expr_primary() {
    if(token.is(TKind::True) || token.is(TKind::False)) {
        return expr_bool();
    }
    else if(token.is_stmt()) {
        /*
        error(
        token.get().line,
        token.get().col,
        "`%s` is statement, not expression",
        token.get().value);*/
        token.step();
    }
    else if(token.is(TKind::Identifer)) {
        Ast *v = expr_var(token.get_step());
        if(v != nullptr)
            return v;
        return nullptr;
    }
    else if(token.is(TKind::Num))
        return expr_num(token.get_step());
    else if(token.is(TKind::Char))
        return expr_char(token.get_step());
    else if(token.is(TKind::String))
        return expr_string(token.get_step());
    else if(token.is(TKind::Lparen)) {
        token.step();
        Ast *left = expr();

        if(token.skip(TKind::Comma)) { // tuple
            if(token.skip(TKind::Rparen)) {
                error("error"); // TODO
                return nullptr;
            }
            Ast_v exs;
            Ast *a;
            Type *ty = new Type(CTYPE::TUPLE);
            exs.push_back(left);
            ty->tupletype_push(left->ctype);

            for(;;) {
                a = expr();
                ty->tupletype_push(a->ctype);
                exs.push_back(a);
                if(token.skip(TKind::Rparen))
                    return new NodeTuple(exs, exs.size(), ty);
                token.expect(TKind::Comma);
            }
        }

        if(!token.expect(TKind::Rparen))
            token.step();

        return left;
    }
    else if(token.is(TKind::Lboxbracket)) {
        token.step();
        if(token.is(TKind::Rboxbracket)) { // TODO: Really?
            error("error");
            return nullptr;
        }

        Ast_v elem;
        Ast *a = expr();
        elem.push_back(a);

        for(;;) {
            if(token.skip(TKind::Rboxbracket))
                break;
            token.expect(TKind::Comma);
            a = expr();
            elem.push_back(a);
            /*
            if(token.skip(TKind::Semicolon)) {
                Ast *nindex = expr();
                if(nindex->ctype->get().type != CTYPE::INT)
                    error("error"); //TODO
                token.expect("]");
                return new Node_list(elem, nindex);
            }
            */
        }

        return new NodeList(elem, elem.size());
    }
    else if(token.is(TKind::Semicolon)) {
        token.step();
        return nullptr;
    }
    else if(token.is(TKind::Rparen))
        return nullptr; //?
    else if(token.is(TKind::End)) {
        /*
        error(token.get().line, token.get().col,
                "expected declaration or statement at end of input");
                */
        exit(1);
    }

    /*
    error(token.see(-1).line, token.see(-1).col,
            "unknown token ` %s `", token.get_step().value.c_str());
            */

    return nullptr;
}

void Parser::expect_type(CTYPE expected, Ast *ty) {
    int t = (int)ty->ctype->get().type;
    if(t & (int)expected)
        return;

    error("unexpected type");
}

void Parser::show(Ast *ast) {
    /*
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case ND_TYPE_NUM:
            {
                Node_number *n = (Node_number *)ast;
                std::cout << n->number << " ";
                break;
            }
            case ND_TYPE_BINARY:
            {
                NodeBinop *b = (NodeBinop *)ast;
                printf(TKind::Lparen);
                std::cout << b->symbol << " ";
                show(b->left);
                show(b->right);
                printf(TKind::Rparen);
                break;
            }
            case ND_TYPE_VARDECL:
            {
                Node_var_decl *v = (Node_var_decl *)ast;
                printf("var_decl: ");
                for(auto decl: v->decl_v)
                    std::cout << TKind::Lparen << decl.type->show() << ", " <<
    decl.name << TKind::Rparen; break;
            }
            case ND_TYPE_ASSIGNMENT:
            {
                Node_assignment *a = (Node_assignment *)ast;
                printf("(= (");
                show(a->dst);
                printf(") (");
                show(a->src);
                printf("))");
                break;
            }
            case ND_TYPE_IF:
            {
                Node_if *i = (Node_if *)ast;
                printf("(if ");
                show(i->cond);
                printf(TKind::Lparen);
                show(i->then_s);
                printf(TKind::Rparen);
                if(i->else_s) {
                    printf("(else ");
                    show(i->else_s);
                    printf(TKind::Rparen);
                }
                printf(TKind::Rparen);
                break;
            }
            case ND_TYPE_WHILE:
            {
                Node_while *w = (Node_while *)ast;
                printf("(while ");
                show(w->cond);
                printf(TKind::Lparen);
                show(w->body);
                printf("))");
                break;
            }
            case ND_TYPE_BLOCK:
            {
                Node_block *b = (Node_block *)ast;
                for(Ast *c: b->cont) {
                    show(c);
                    puts("");
                }
                break;
            }
            case ND_TYPE_RETURN:
            {
                Node_return *r = (Node_return *)ast;
                printf("return: ");
                show(r->cont);
                puts("");
                break;
            }
            case ND_TYPE_FUNCDEF:
            {
                Node_func_def *f = (Node_func_def *)ast;
                printf("func-def: (");
                std::cout << f->name << TKind::Lparen;
                for(auto a: f->args)
                    std::cout << TKind::Lparen << a.type->show() << TKind::Comma
    << a.name << TKind::Rparen; std::cout << ") -> " << f->ret_type->show() <<
    TKind::Lparen << std::endl; for(Ast *b: f->block) { show(b); puts("");
                }
                printf("))");
                break;
            }
            case ND_TYPE_FUNCCALL:
            {
                Node_func_call *f = (Node_func_call *)ast;
                printf("(func-call: (");
                std::cout << f->name << TKind::Lparen << std::endl;
                for(Ast *a: f->arg_v) {
                    show(a);
                    puts("");
                }
                printf("))");
                break;
            }
            case ND_TYPE_VARIABLE:
            {
                NodeVariable *v = (NodeVariable *)ast;
                printf("(var: ");
                std::cout << v->name << TKind::Rparen;
                break;
            }
            default:
            {
                fprintf(stderr, "error show\n");
            }
        }
    }
    */
}
