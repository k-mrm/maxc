#include "parser.h"
#include "error.h"

Ast_v &Parser::run() { return eval(); }

Ast_v &Parser::eval() {
    while(!token.is(TKind::End)) {
        program.push_back(statement());
    }

    return program;
}

Ast *Parser::statement() {
    if(token.is(TKind::Lbrace))
        return make_block();
    else if(token.skip(TKind::For))
        return make_for();
    else if(token.skip(TKind::While))
        return make_while();
    else if(token.skip(TKind::If))
        return make_if(false);
    else if(token.skip(TKind::Return))
        return make_return();
    else if(token.skip(TKind::Let))
        return var_decl(false);
    else if(token.skip(TKind::Const))
        return var_decl(true);
    else if(token.skip(TKind::Fn))
        return func_def();
    else if(token.skip(TKind::Struct))
        return make_struct();
    else if(token.skip(TKind::Typedef)) {
        make_typedef();
        return nullptr;
    }
    else
        return expr();
}

Ast *Parser::expr() { return expr_assign(); }

Ast *Parser::func_def() {
    std::string name = token.get().value;
    token.step();

    // fn main(): typename {
    //        ^
    if(!token.expect(TKind::Lparen)) {
        return nullptr;
    }

    Varlist args;
    var_t arg_info;
    func_t fn_arg_info;
    Type_v argtys;

    if(!token.skip(TKind::Rparen))
        // fn main(a: int, b: int): typename {
        //         ^^^^^^^^^^^^^^
        for(;;) {
            std::string arg_name = token.get().value;
            token.step();

            token.expect(TKind::Colon);

            Type *arg_ty = eval_type();
            argtys.push_back(arg_ty);

            if(arg_ty->isfunction())
                fn_arg_info = func_t(arg_ty);
            else
                arg_info = (var_t){0, arg_ty};

            NodeVariable *a = arg_ty->isfunction()
                                  ? new NodeVariable(arg_name, fn_arg_info)
                                  : new NodeVariable(arg_name, arg_info);

            args.push(a);

            if(token.skip(TKind::Rparen))
                break;
            token.expect(TKind::Comma);
        }

    // fn main(): int {
    //          ^^^^^
    Type *ret_ty = token.skip(TKind::Colon) ? eval_type() : nullptr;

    Type *fntype = new Type(CTYPE::FUNCTION);

    fntype->fnarg = argtys;
    fntype->fnret = ret_ty;

    Ast *block;

    if(token.is(TKind::Lbrace)) {
        block = make_block();

        if(ret_ty == nullptr)
            fntype->fnret = mxcty_none;
    }
    else {
        token.expect(TKind::Assign);

        block = expr();

        token.expect(TKind::Semicolon);

        if(ret_ty == nullptr)
            fntype->fnret = new Type(CTYPE::UNINFERRED);
    }

    func_t finfo = func_t(args, fntype);

    NodeVariable *function = new NodeVariable(name, finfo);

    return new NodeFunction(function, finfo, block);
}

Ast *Parser::var_decl(bool isconst) {
    var_t info;
    func_t finfo;

    Ast *init;

    Type *ty;
    NodeVariable *var;

    std::string name = token.get().value;
    token.step();

    /*
     *  let a(: int) = 100;
     *        ^^^^^
     */
    if(token.skip(TKind::Colon)) {
        ty = eval_type();
    }
    else
        ty = new Type(CTYPE::UNINFERRED);

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
            finfo = func_t(ty);
        else
            info = (var_t){vattr, ty};

        var = ty->isfunction() ? new NodeVariable(name, finfo)
                               : new NodeVariable(name, info);

        vls.push(var);
    }
    else {
        var = nullptr;
    }

    token.expect(TKind::Semicolon);

    return new NodeVardecl(var, init);
}

Ast *Parser::make_struct() {
    /*
     *  struct TagName {
     *      a: int,
     *      b: string,
     *  }
     *
     */
    std::string tag = token.get_step().value;

    token.expect(TKind::Lbrace);

    Ast_v decls = {};

    if(token.skip(TKind::Rbrace))
        return new NodeStruct(tag, decls);

    for(;;) {
        std::string name = token.get_step().value;
        token.expect(TKind::Colon);

        Type *ty = eval_type();

        decls.push_back(new NodeVariable(name, (var_t){0, ty}));

        if(token.skip(TKind::Rbrace))
            break;

        token.expect(TKind::Comma);
    }

    return new NodeStruct(tag, decls);
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
        ty = mxcty_int;
    else if(token.skip(TKind::TUint))
        ty = new Type(CTYPE::UINT);
    else if(token.skip(TKind::TBool))
        ty = mxcty_bool;
    else if(token.skip(TKind::TChar))
        ty = new Type(CTYPE::CHAR);
    else if(token.skip(TKind::TString))
        ty = mxcty_string;
    else if(token.skip(TKind::TFloat))
        ty = mxcty_float;
    else if(token.skip(TKind::TNone)) // TODO :only function rettype
        ty = mxcty_none;
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

        return mxcty_none;
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

    Ast *b;

    for(;;) {
        if(token.skip(TKind::Rbrace))
            break;
        b = statement();

        if(b->isexpr()) {
            token.expect(TKind::Semicolon);
        }

        cont.push_back(b);
    }

    return new NodeBlock(cont);
}

Ast *Parser::make_if(bool isexpr) {
    Ast *cond = expr();

    Ast *then = isexpr ? expr() : make_block();

    if(token.skip(TKind::Else)) {
        Ast *el;

        if(token.skip(TKind::If))
            el = make_if(isexpr);
        else
            el = isexpr ? expr() : make_block();

        return new NodeIf(cond, then, el, isexpr);
    }

    return new NodeIf(cond, then, nullptr, isexpr);
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
    Ast *cond = expr();

    Ast *body = make_block();

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
    if(strchr(tk.value.c_str(), '.'))
        return new NodeNumber(atof(tk.value.c_str()));
    else
        return new NodeNumber(atol(tk.value.c_str()));
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

Ast *Parser::expr_var(token_t tk) { return new NodeVariable(tk.value); }

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

    for(;;) {
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

    for(;;) {
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

    for(;;) {
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

    for(;;) {
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

    for(;;) {
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

    for(;;) {
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

    for(;;) {
        if(token.is(TKind::Dot)) {
            token.step();

            Ast *memb = expr_var(token.get());

            token.step();
            /*
             *  a.function();
             *            ^
             */
            if(token.skip(TKind::Lparen)) {
                Ast_v args = {left};

                if(token.skip(TKind::Rparen))
                    ;
                else
                    for(;;) {
                        args.push_back(expr());

                        if(token.skip(TKind::Rparen))
                            break;

                        token.expect(TKind::Comma);
                    }

                left = new NodeFnCall(memb, args);
            }
            else { // struct
                left = new NodeMember(left, memb);
            }
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
    else if(token.skip(TKind::If))
        return make_if(true);
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
