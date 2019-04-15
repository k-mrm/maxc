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
    if(token.skip("{"))
        return make_block();
    else if(token.is_value("if"))
        return make_if();
    else if(token.is_value("for"))
        return make_for();
    else if(token.is_value("while"))
        return make_while();
    else if(token.skip("return"))
        return make_return();
    else if(token.skip("print"))
        return make_print();
    else if(token.skip("println"))
        return make_println();
    else if(token.skip("let"))
        return var_decl();
    else if(token.skip("fn"))
        return func_def();
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
    std::string name = token.get().value;
    token.step();

    if(token.expect("(")) {
        env.make();
        Varlist args;
        var_t ainfo;
        Type_v argtys;
        if(token.skip(")")) goto skiparg;

        for(;;) {
            std::string arg_name = token.get().value; token.step();
            token.expect(":");
            Type *arg_ty = eval_type();
            argtys.push_back(arg_ty);

            ainfo = (var_t){arg_ty, arg_name};
            NodeVariable *a = new NodeVariable(ainfo, false);
            args.push(a);
            env.get()->vars.push(a);
            vls.push(a);

            if(token.skip(")")) break;
            token.expect(",");
        }
skiparg:
        token.expect("->");
        Type *rty = eval_type();
        Type *fntype = new Type(CTYPE::FUNCTION);
        fntype->fnarg = argtys;
        fntype->fnret = rty;
        auto finfo = func_t(name, args, fntype);

        env.current->parent->vars.push(
                new NodeVariable(finfo,
                    env.get()->parent->isglb)
                );
        token.expect("{");
        Ast_v b;
        while(!token.skip("}")) {
            b.push_back(statement());
            token.skip(";");
        }

        Ast *t = new NodeFunction(finfo, b, vls);
        vls.reset();
        env.escape();
        return t;
    }

    return nullptr;
}

Ast *Parser::var_decl() {
    var_t info;
    Ast_v init;
    bool isglobal = env.isglobal();
    Varlist v;
    Type *ty;
    NodeVariable *var;
    Ast *initast;

    while(1) {
        std::string name = token.get().value;
        token.step();
        token.expect(":");
        ty = eval_type();

        if(token.skip("=")) {
            initast = expr();
            checktype(ty, initast->ctype);
            init.push_back(initast);
        }
        else init.push_back(nullptr);

        info = (var_t){ty, name};
        var = new NodeVariable(info, isglobal);
        v.push(var);
        env.get()->vars.push(var);
        vls.push(var);

        if(token.is_value(";")) break;
        token.expect(",");
    }

    return new NodeVardecl(v, init);
}

Type *Parser::eval_type() {
    Type *ty;
    if(token.skip("(")) {   //tuple
        ty = new Type(CTYPE::TUPLE);
        for(;;) {
            ty->tuple.push_back(eval_type());
            if(token.skip(")")) break;
            token.expect(",");
        }
    }
    else if(token.skip("int"))
        ty = new Type(CTYPE::INT);
    else if(token.skip("uint"))
        ty = new Type(CTYPE::UINT);
    else if(token.skip("int64"))
        ty = new Type(CTYPE::INT64);
    else if(token.skip("uint64"))
        ty = new Type(CTYPE::UINT64);
    else if(token.skip("char"))
        ty = new Type(CTYPE::CHAR);
    else if(token.skip("string"))
        ty = new Type(CTYPE::STRING);
    else if(token.skip("none"))     //TODO:only function rettype
        ty = new Type(CTYPE::NONE);
    else if(token.skip("fn")) {
        ty = new Type(CTYPE::FUNCTION);
        token.expect("(");
        while(!token.skip(")")) {
            ty->fnarg.push_back(eval_type());
            if(token.skip(")")) break;
            token.expect(",");
        }
        token.expect("->");
        ty->fnret = eval_type();
    }
    else {
        error(token.get().line, token.get().col,
                "unknown type name: `%s`", token.get().value.c_str());
        token.step();
        return nullptr;
    }

    for(;;) {
        if(token.skip2("[", "]")) ty = new Type(ty);
        else break;
    }
    return ty;
}

Ast *Parser::make_assign(Ast *dst, Ast *src) {
    if(!dst)
        return nullptr;
    checktype(dst->ctype, src->ctype);
    return new NodeAssignment(dst, src);
}

Ast *Parser::make_assigneq(std::string op, Ast *dst, Ast *src) {
    ;
}

Ast *Parser::make_block() {
    Ast_v cont;
    env.make();
    Ast *b;
    while(!token.skip("}")) {
        b = statement();
        token.expect(";");
        cont.push_back(b);
    }

    env.escape();
    return new NodeBlock(cont);
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

            return new NodeIf(cond, then, el);
        }

        return new NodeIf(cond, then, nullptr);
    }
    else
        return nullptr;
}

Ast *Parser::expr_if() {
    token.expect("if");
    token.expect("(");
    Ast *cond = expr();
    token.expect(")");
    Ast *then = statement();
    token.skip(";");

    if(token.skip("else")) {
        Ast *el = statement();

        return new NodeExprif(cond, then, el);
    }
    return new NodeExprif(cond, then, nullptr);
}

Ast *Parser::make_for() {
    if(token.skip("for")) {
        token.skip("(");
        Ast *init = expr();
        token.expect(";");
        Ast *cond = expr();
        token.expect(";");
        Ast *reinit = expr();
        token.expect(")");
        Ast *body = statement();

        return new NodeFor(init, cond, reinit, body);
    }
    return nullptr;
}

Ast *Parser::make_while() {
    if(token.skip("while")) {
        token.skip("(");
        Ast *cond = expr();
        token.skip(")");
        Ast *body = statement();

        return new NodeWhile(cond, body);
    }
    return nullptr;
}

Ast *Parser::make_return() {
    return new NodeReturn(expr());
}

Ast *Parser::make_print() {
    token.expect("(");
    if(token.skip(")")) {
        warning(token.get().line, token.get().col,
                "You don't have the contents of `print`, but are you OK?");
        return new NodePrint(nullptr);
    }
    Ast *c = expr();
    token.expect(")");

    return new NodePrint(c);
}

Ast *Parser::make_println() {
    token.expect("(");
    if(token.skip(")")) {
        warning(token.get().line, token.get().col,
                "You don't have the contents of `println`, but are you OK?");
        return new NodePrintln(nullptr);
    }
    Ast *c = expr();
    token.expect(")");

    return new NodePrintln(c);
}

Ast *Parser::make_format() {
    token.expect("(");
    if(token.skip(")")) {
        error(token.get().line, token.get().col,
                "No content of `format`");
        return nullptr;
    }
    if(!token.is_type(TOKEN_TYPE::STRING)) {
        error(token.get().line, token.get().col,
                "`format`'s first argument must be string");
        while(!token.step_to(";"));
        return nullptr;
    }
    //format("{}, world{}", "Hello", 2);
    std::string cont = token.get().value;
    token.step();
    char *p;
    char *s = const_cast<char*>(cont.c_str());
    unsigned int ncnt = 0;
    while((p = const_cast<char*>(strstr(s, "{}"))) != NULL) {
        p += 2;
        s = p;
        ++ncnt;
    }

    Ast_v args;

    if(ncnt != 0) {
        while(1) {
            token.expect(",");
            args.push_back(expr());

            if(token.skip(")")) break;
        }
        if(args.size() != ncnt) {
            error(token.get().line, token.get().col,
                    "positional arguments in format, but there is %d %s",
                    args.size(), args.size() >= 2 ? "arguments" : "argument");
            return nullptr;
        }

        debug("%d\n", ncnt);
        return new NodeFormat(cont, ncnt, args);
    }
    else {
        if(!token.expect(")")) {
            while(!token.step_to(";"));
        }
        return new NodeFormat(cont, 0, std::vector<Ast *>());
    }
}

Ast *Parser::make_typeof() {
    token.expect("(");
    if(token.is_value(")")) {
        error(token.get().line, token.get().col, "`typeof` must have an argument");
        token.step();
        return new NodeTypeof(nullptr);
    }
    Ast *var = expr();
    if(var->get_nd_type() != NDTYPE::VARIABLE) {
        error(token.get().line, token.get().col, "`typeof`'s argument must be variable");
        token.step();
        return new NodeTypeof(nullptr);
    }
    token.expect(")");

    return new NodeTypeof((NodeVariable *)var);
}

Ast *Parser::read_lsmethod(Ast *left) {
    if(token.skip("size"))
        return new NodeDotop(left, Method::ListSize, new Type(CTYPE::INT));
    else
        return nullptr; //TODO
}

Ast *Parser::read_strmethod(Ast *left) {
    if(token.skip("len"))
        return new NodeDotop(left, Method::StringLength, new Type(CTYPE::INT));
    else
        return nullptr; //TODO err handling
}

Ast *Parser::read_tuplemethod(Ast *left) {
    Ast *index = expr_num(token.get());
    int i = atoi(token.get_step().value.c_str());
    return new NodeAccess(left, index, left->ctype->tuple[i], true);
}

Ast *Parser::expr_num(token_t tk) {
    if(tk.type != TOKEN_TYPE::NUM) {
        error(token.see(-1).line, token.see(-1).col, "not a number: %s", tk.value.c_str());
    }
    return new NodeNumber(atoi(tk.value.c_str()));
}

Ast *Parser::expr_char(token_t token) {
    assert(token.type == TOKEN_TYPE::CHAR);
    assert(token.value.length() == 1);
    char c = token.value[0];
    return new NodeChar(c);
}

Ast *Parser::expr_string(token_t token) {
    std::string s = token.value;
    return new NodeString(s);
}

Ast *Parser::expr_var(token_t tk) {
    for(env_t *e = env.get(); ; e = e->parent) {
        if(!e->vars.get().empty())
            break;
        if(e->isglb) {
            //debug("empty\n");
            goto verr;
        }
    }

    //env.get()->vars.show();
    for(env_t *e = env.get(); ; e = e->parent) {
        for(auto &v: e->vars.get()) {
            if(v->ctype->isfunction()) {
                if(v->finfo.name == tk.value) return v;
            }
            else if(v->vinfo.name == tk.value) {
                //debug("%s found\n", tk.value.c_str());
                return v;
            }
        }
        if(e->isglb) {
            //debug("it is glooobal\n");
            goto verr;
        }
    }

verr:
    error(token.see(-1).line, token.see(-1).col, "undeclared variable: `%s`", tk.value.c_str());
    while(!token.step_to(";"));
    return nullptr;
}

Ast *Parser::expr_assign() {
    Ast *left = expr_ternary();

    if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("=")) {
        if(left == nullptr)
            return nullptr;
        if(left->get_nd_type() != NDTYPE::VARIABLE && left->get_nd_type() != NDTYPE::ACCESS) {
            error(token.see(-1).line, token.see(-1).col,
                    "left side of the expression is not valid");
        }
        token.step();
        left = make_assign(left, expr_assign());
    }

    return left;
}

Ast *Parser::expr_ternary() {
    Ast *left = expr_logic_or();

    if(!token.skip("?")) return left;
    Ast *then = expr();
    token.expect(":");
    Ast *els = expr_ternary();
    Type *t = checktype(then->ctype, els->ctype);

    return new NodeTernop(left, then, els, t);
}

Ast *Parser::expr_logic_or() {
    Ast *left = expr_logic_and();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("||")) {
            token.step();
            t = expr_logic_and();
            left = new NodeBinop("||", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::IDENTIFER) && token.is_value("or")) {
            token.step();
            t = expr_logic_and();
            left = new NodeBinop("||", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else
            return left;
    }
}

Ast *Parser::expr_logic_and() {
    Ast *left = expr_equality();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("&&")) {
            token.step();
            t = expr_equality();
            left = new NodeBinop("&&", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::IDENTIFER) && token.is_value("and")) {
            token.step();
            t = expr_equality();
            left = new NodeBinop("&&", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else
            return left;
    }
}

Ast *Parser::expr_equality() {
    Ast *left = expr_comp();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("==")) {
            token.step();
            t = expr_comp();
            left = new NodeBinop("==", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("!=")) {
            token.step();
            t = expr_comp();
            left = new NodeBinop("!=", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else
            return left;
    }
}

Ast *Parser::expr_comp() {
    Ast *left = expr_add();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("<")) {
            token.step();
            t = expr_add();
            left = new NodeBinop("<", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value(">")) {
            token.step();
            t = expr_add();
            left = new NodeBinop(">", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("<=")) {
            token.step();
            t = expr_add();
            left = new NodeBinop("<=", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value(">=")) {
            token.step();
            t = expr_add();
            left = new NodeBinop(">=", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else
            return left;
    }
}

Ast *Parser::expr_add() {
    Ast *left = expr_mul();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("+")) {
            token.step();
            t = expr_mul();
            left = new NodeBinop("+", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("-")) {
            token.step();
            t = expr_mul();
            left = new NodeBinop("-", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_mul() {
    Ast *left = expr_unary();
    Ast *t;

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("*")) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("*", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("/")) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("/", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("%")) {
            token.step();
            t = expr_unary();
            left = new NodeBinop("%", left, t,
                    checktype(left->ctype, t->ctype));
        }
        else
            return left;
    }
}

Ast *Parser::expr_unary() {
    token.save();

    if(token.is_type(TOKEN_TYPE::SYMBOL) && (token.is_value("++") || token.is_value("--") ||
                token.is_value("&") || token.is_value("!"))){
        std::string op = token.get().value;
        token.step();
        Ast *operand = expr_unary();
        if(operand->get_nd_type() != NDTYPE::VARIABLE)
            error(token.see(-1).line, token.see(-1).col, "lvalue required as `%s` operand", op.c_str());
        return new NodeUnaop(op, operand, operand->ctype);
    }
    /*
    else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("*")) {
        token.step();
        Ast *operand = expr_unary();
        //NodeVariable *v = (NodeVariable *)operand;
        assert(operand->ctype->get().type == CTYPE::PTR);
        return new NodeUnaop("*", operand);
    }
    */

    token.rewind();
    return expr_unary_postfix();
}

Ast *Parser::expr_unary_postfix() {
    Ast *left = expr_primary();

    while(1) {
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value(".")) {
            token.step();
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
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("[")) {
            token.step();
            Ast *index = expr();
            token.expect("]");
            Type *ty = left->ctype->ptr;
            left = new NodeAccess(left, index, ty);
        }
        else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("(")) {
            token.step();
            Ast_v args;
            if(!left->ctype->isfunction()) {
                error("error"); return nullptr;
            }

            if(token.skip(")")) goto fin;

            for(;;) {
                args.push_back(expr());
                if(token.skip(")")) break;
                debug("%s", token.get().value.c_str());
                token.expect(",");
            }

fin:
            left = new NodeFnCall(left, args);
        }
        else
            return left;
    }
}

Ast *Parser::expr_primary() {
    if(token.is_value("if"))
        return expr_if();
    else if(token.skip("typeof"))
        return make_typeof();
    else if(token.skip("format"))
        return make_format();
    else if(token.is_stmt()) {
        error(token.get().line, token.get().col, "`%s` is statement, not expression",
                token.get().value);
        token.step();
    }
    else if(token.is_type(TOKEN_TYPE::IDENTIFER)) {
        Ast *v = expr_var(token.get_step());
        if(v != nullptr) return v;
        else return nullptr;
    }
    else if(token.is_type(TOKEN_TYPE::NUM))
        return expr_num(token.get_step());
    else if(token.is_type(TOKEN_TYPE::CHAR))
        return expr_char(token.get_step());
    else if(token.is_type(TOKEN_TYPE::STRING))
        return expr_string(token.get_step());
    else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("(")) {
        token.step();
        Ast *left = expr();

        if(token.skip(",")) { //tuple
            if(token.skip(")")) {
                error("error"); //TODO
                return nullptr;
            }
            Ast_v exs; Ast *a;
            Type *ty = new Type(CTYPE::TUPLE);
            exs.push_back(left);
            ty->tupletype_push(left->ctype);
            for(;;) {
                a = expr();
                ty->tupletype_push(a->ctype);
                exs.push_back(a);
                if(token.skip(")")) return new NodeTuple(exs, exs.size(), ty);
                token.expect(",");
            }
        }

        if(token.expect(")")) return left;

        return nullptr;
    }
    else if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("[")) {
        token.step();
        if(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("]")) {
            error("error");
            return nullptr;
        }
        Ast_v elem;
        Ast *a = expr();
        Type *bty = a->ctype;
        elem.push_back(a);
        for(;;) {
            if(token.skip("]")) break;
            token.expect(",");
            a = expr();
            checktype(bty, a->ctype);
            elem.push_back(a);
            /*
            if(token.skip(";")) {
                Ast *nindex = expr();
                if(nindex->ctype->get().type != CTYPE::INT)
                    error("error"); //TODO
                token.expect("]");
                return new Node_list(elem, nindex);
            }
            */
        }

        bty = new Type(bty);
        return new NodeList(elem, elem.size(), bty);
    }
    else if(token.is_value(";"))
        return nullptr;
    else if(token.is_value(")"))
        return nullptr;
    else if(token.is_type(TOKEN_TYPE::END)) {
        error(token.get().line, token.get().col,
                "expected declaration or statement at end of input");
        exit(1);
    }

    error(token.see(-1).line, token.see(-1).col,
            "unknown token ` %s `", token.get_step().value.c_str());
    return nullptr;
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

int Parser::skip_ptr() {
    int c = 0;
    while(token.is_type(TOKEN_TYPE::SYMBOL) && token.is_value("*")) {
        token.step(); ++c;
    }

    return c;
}

Type *Parser::checktype(Type *ty1, Type *ty2) {
    bool swapped = false;
    if(ty1->islist()) {
        if(!ty2->islist()) goto err;
        Type *b = ty1;
        for(;;) {
            ty1 = ty1->ptr;
            ty2 = ty2->ptr;
            if(ty1 == nullptr && ty2 == nullptr) return b;
            if(ty1 == nullptr || ty2 == nullptr) goto err;
            checktype(ty1, ty2);
        }
    }
    else if(ty1->istuple()) {
        if(!ty2->istuple()) goto err;
        if(ty1->tuple.size() != ty2->tuple.size()) goto err;
        int s = ty1->tuple.size();
        int cnt = 0;
        for(;;) {
            checktype(ty1->tuple[cnt], ty2->tuple[cnt]);
            ++cnt;
            if(cnt == s) return ty1;
        }
    }

    if(ty1->get().type == ty2->get().type) return ty1;
    if(ty1->get().type > ty2->get().type) {
        std::swap(ty1, ty2);
        swapped = true;
    }
    switch(ty1->get().type) {
        case CTYPE::NONE:
            goto err;
        case CTYPE::INT:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else if(ty2->get().type == CTYPE::UINT || ty2->get().type == CTYPE::INT64 ||
                    ty2->get().type == CTYPE::UINT64)
                return ty2;
            else
                goto err;
        case CTYPE::UINT:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else if(ty2->get().type == CTYPE::INT64)
                return new Type(CTYPE::UINT64);
            else if(ty2->get().type == CTYPE::UINT64)
                return ty2;
            else
                goto err;
        case CTYPE::UINT64:
            if(ty2->get().type == CTYPE::CHAR)
                return ty1;
            else
                goto err;
        case CTYPE::CHAR:
        case CTYPE::STRING:
        case CTYPE::LIST:
                goto err;
        default:
            error("unimplemented(check type)"); return nullptr;
    }
err:
    if(swapped) std::swap(ty1, ty2);
    error(token.get().line, token.get().col,
            "expected type `%s`, found type `%s`", ty1->show().c_str(), ty2->show().c_str());
    return nullptr;
}

bool Parser::ensure_hasmethod(Type *ty) {
    switch(ty->get().type) {
        case CTYPE::LIST:
        case CTYPE::STRING:
        case CTYPE::TUPLE:
            return true;
        default:
            error(token.get().line, token.get().col, "this type does not have method");
            return false;
    }
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
                printf("(");
                std::cout << b->symbol << " ";
                show(b->left);
                show(b->right);
                printf(")");
                break;
            }
            case ND_TYPE_VARDECL:
            {
                Node_var_decl *v = (Node_var_decl *)ast;
                printf("var_decl: ");
                for(auto decl: v->decl_v)
                    std::cout << "(" << decl.type->show() << ", " << decl.name << ")";
                break;
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
            case ND_TYPE_WHILE:
            {
                Node_while *w = (Node_while *)ast;
                printf("(while ");
                show(w->cond);
                printf("(");
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
            case ND_TYPE_FUNCCALL:
            {
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
            case ND_TYPE_VARIABLE:
            {
                NodeVariable *v = (NodeVariable *)ast;
                printf("(var: ");
                std::cout << v->name << ")";
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
