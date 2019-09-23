#include "parser.h"
#include "error.h"
#include "lexer.h"

static Vector *eval();
static Ast *statement();
static Ast *expr();
static Ast *func_def();
static Ast *make_block();
static Ast *make_if(bool);
static Ast *make_for();
static Ast *make_while();
static Ast *make_return();
static Ast *make_struct();
static Ast *make_import();
static void make_typedef();

static Ast *expr_assign();
static Ast *expr_equality();
static Ast *expr_logic_or();
static Ast *expr_logic_and();
static Ast *expr_comp();
static Ast *expr_add();
static Ast *expr_mul();
static Ast *expr_unary();
static Ast *expr_unary_postfix();
static Ast *expr_primary();

static Ast *struct_init();
static Ast *var_decl(bool);

static Ast *expr_num(Token *);
static Ast *expr_unary();
static Type *eval_type();

static Vector *tokens = NULL;
static Vector *tokens_stack;
static Vector *pos_stack;
static int pos = 0;
static int nenter = 0;

#define Step() (++pos)
#define Cur_Token() ((Token *)tokens->data[pos])
#define Get_Step_Token() ((Token *)tokens->data[pos++])
#define Cur_Token_Is(tk) ((Cur_Token()->kind) == (tk))

static Vector *enter(Vector *tk) {
    vec_push(tokens_stack, tokens);
    vec_push(pos_stack, (void *)(intptr_t)pos);

    tokens = tk;
    pos = 0;

    Vector *result = eval();

    tokens = vec_pop(tokens_stack);
    pos = (intptr_t)vec_pop(pos_stack);

    nenter++;

    return result;
}

Vector *parser_run(Vector *_token) {
    tokens_stack = New_Vector();
    pos_stack = New_Vector();

    return enter(_token);
}

static bool skip(enum TKIND tk) {
    if(Cur_Token()->kind == tk) {
        ++pos;
        return true;
    }

    return false;
}

static bool skip2(enum TKIND tk1, enum TKIND tk2) {
    int tmp = pos;

    if(Cur_Token()->kind == tk1) {
        ++pos;
        if(Cur_Token()->kind == tk2) {
            ++pos;
            return true;
        }
    }

    pos = tmp;
    return false;
}

static Token *see(int);

static bool expect(enum TKIND tk) {
    if(Cur_Token()->kind == tk) {
        ++pos;
        return true;
    }
    else {
        expect_token(see(-1)->end, see(-1)->end, tk2str(tk));
        return false;
    }
}

static Token *see(int p) { return tokens->data[pos + p]; }

static Vector *eval() {
    Vector *program = New_Vector();

    while(!Cur_Token_Is(TKIND_End)) {
        vec_push(program, statement());
    }

    return program;
}

static Ast *statement() {
    if(Cur_Token_Is(TKIND_Lbrace))
        return make_block();
    else if(skip(TKIND_For))
        return make_for();
    else if(skip(TKIND_While))
        return make_while();
    else if(skip(TKIND_If))
        return make_if(false);
    else if(skip(TKIND_Return))
        return make_return();
    else if(skip(TKIND_Let))
        return var_decl(false);
    else if(skip(TKIND_Const))
        return var_decl(true);
    else if(skip(TKIND_Fn))
        return func_def();
    else if(skip(TKIND_Struct))
        return make_struct();
    else if(skip(TKIND_Import))
        return make_import();
    else if(skip(TKIND_Typedef)) {
        make_typedef();
        return NULL;
    }
    else
        return expr();
}

static Ast *expr() { return expr_assign(); }

static Ast *func_def() {
    char *name = Cur_Token()->value;
    Step();

    // fn main(): typename {
    //        ^
    if(!expect(TKIND_Lparen)) {
        return NULL;
    }

    Varlist *args = New_Varlist();
    var_t arg_info;
    func_t fn_arg_info;
    Vector *argtys = New_Vector();

    if(!skip(TKIND_Rparen))
        /*
         * fn main(a: int, b: int): int {
         *         ^^^^^^^^^^^^^^
         *
         * fn main(a, b: int): int {
         *         ^^^^^^^^^
         */
        for(;;) {
            Vector *argnames = New_Vector();

            char *arg_name = Get_Step_Token()->value;
            vec_push(argnames, arg_name);

            if(skip(TKIND_Comma)) {
                for(;;) {
                    char *name = Get_Step_Token()->value;
                    vec_push(argnames, name);

                    if(Cur_Token_Is(TKIND_Colon)) {
                        break;
                    }

                    expect(TKIND_Comma);
                }
            }

            expect(TKIND_Colon);

            Type *arg_ty = eval_type();

            for(int i = 0; i < argnames->len; ++i)
                vec_push(argtys, arg_ty);

            if(type_is(arg_ty, CTYPE_FUNCTION))
                fn_arg_info = New_Func_t(arg_ty);
            else
                arg_info = (var_t){0, arg_ty};

            Varlist *a = New_Varlist();

            for(int i = 0; i < argnames->len; ++i) {
                varlist_push(a,
                             type_is(arg_ty, CTYPE_FUNCTION)
                                 ? new_node_variable_with_func(
                                       argnames->data[i], fn_arg_info)
                                 : new_node_variable_with_var(argnames->data[i],
                                                              arg_info));
            }

            varlist_mulpush(args, a);

            if(skip(TKIND_Rparen))
                break;
            expect(TKIND_Comma);
        }

    // fn main(): int {
    //          ^^^^^
    Type *ret_ty = skip(TKIND_Colon) ? eval_type() : NULL;

    Type *fntype = New_Type(CTYPE_FUNCTION);

    fntype->fnarg = argtys;
    fntype->fnret = ret_ty;

    Ast *block;

    if(Cur_Token_Is(TKIND_Lbrace)) {
        block = make_block();

        if(ret_ty == NULL)
            fntype->fnret = mxcty_none;
    }
    else {
        expect(TKIND_Assign);

        block = expr();

        expect(TKIND_Semicolon);

        if(ret_ty == NULL)
            fntype->fnret = New_Type(CTYPE_UNINFERRED);
    }

    func_t finfo = New_Func_t_With_Varlist(args, fntype);

    NodeVariable *function = new_node_variable_with_func(name, finfo);

    return (Ast *)new_node_function(function, finfo, block);
}

static Ast *var_decl(bool isconst) {
    var_t info;
    func_t finfo;

    Ast *init;

    Type *ty;
    NodeVariable *var;

    char *name = Cur_Token()->value;
    Step();

    /*
     *  let a(: int) = 100;
     *        ^^^^^
     */
    if(skip(TKIND_Colon)) {
        ty = eval_type();
    }
    else
        ty = New_Type(CTYPE_UNINFERRED);

    int vattr = 0;

    if(isconst)
        vattr |= VARATTR_CONST;

    /*
     *  let a: int = 100;
     *             ^
     */
    if(skip(TKIND_Assign)) {
        init = expr();
    }
    else if(isconst) {
        error_at(see(0)->start, see(0)->end, "const must initialize");

        init = NULL;
    }
    else {
        init = NULL;
    }

    if(ty != NULL) {
        if(type_is(ty, CTYPE_FUNCTION))
            finfo = New_Func_t(ty);
        else
            info = (var_t){vattr, ty};

        var = type_is(ty, CTYPE_FUNCTION)
                  ? new_node_variable_with_func(name, finfo)
                  : new_node_variable_with_var(name, info);
    }
    else {
        var = NULL;
    }

    expect(TKIND_Semicolon);

    return (Ast *)new_node_vardecl(var, init);
}

static Ast *make_struct() {
    /*
     *  struct TagName {
     *      a: int,
     *      b: string
     *  }
     *
     */
    char *tag = Get_Step_Token()->value;

    expect(TKIND_Lbrace);

    Vector *decls = New_Vector();

    if(skip(TKIND_Rbrace))
        return (Ast *)new_node_struct(tag, decls);

    for(;;) {
        char *name = Get_Step_Token()->value;
        expect(TKIND_Colon);

        Type *ty = eval_type();

        vec_push(decls, new_node_variable_with_var(name, (var_t){0, ty}));

        if(skip(TKIND_Rbrace))
            break;

        expect(TKIND_Comma);
    }

    return (Ast *)new_node_struct(tag, decls);
}

static void make_ast_from_mod(Vector *s, char *name) {
    char path[512];

    sprintf(path, "./lib/%s.mxc", name);

    char *src = read_file(path);
    if(!src) {
        memset(path, 0, 512);
        sprintf(path, "./%s.mxc", name);

        src = read_file(path);
        if(!src) {
            error("lib %s: not found", name);
            return;
        }
    }

    Vector *token = lexer_run(src);

    Vector *AST = enter(token);

    for(int i = 0; i < AST->len; i++) {
        vec_push(s, AST->data[i]);
    }
}

static Ast *make_import() {
    Vector *mod_names = New_Vector();
    Vector *statements = New_Vector();

    char *mod = Get_Step_Token()->value;

    expect(TKIND_Semicolon);

    vec_push(mod_names, mod);

    for(int i = 0; i < mod_names->len; i++) {
        make_ast_from_mod(statements, (char *)mod_names->data[i]);
    }

    return (Ast *)new_node_block_nonscope(statements);
}

static Type *eval_type() {
    Type *ty;

    if(skip(TKIND_Lparen)) { // tuple
        ty = New_Type(CTYPE_TUPLE);

        for(;;) {
            vec_push(ty->tuple, eval_type());

            if(skip(TKIND_Rparen))
                break;

            expect(TKIND_Comma);
        }
    }
    else if(skip(TKIND_TInt))
        ty = mxcty_int;
    else if(skip(TKIND_TUint))
        ty = New_Type(CTYPE_UINT);
    else if(skip(TKIND_TBool))
        ty = mxcty_bool;
    else if(skip(TKIND_TString))
        ty = mxcty_string;
    else if(skip(TKIND_TFloat))
        ty = mxcty_float;
    else if(skip(TKIND_TNone)) // TODO :only function rettype
        ty = mxcty_none;
    else if(skip(TKIND_Fn)) {
        ty = New_Type(CTYPE_FUNCTION);

        expect(TKIND_Lparen);

        while(!skip(TKIND_Rparen)) {
            vec_push(ty->fnarg, eval_type());
            if(skip(TKIND_Rparen))
                break;
            expect(TKIND_Comma);
        }

        expect(TKIND_Colon);
        ty->fnret = eval_type();
    }
    /*
    else if(typemap.count(token.get().value) != 0) {
        ty = typemap[token.get().value];

        Step();
    }*/
    else {
        char *tk = Cur_Token()->value;

        Step();

        ty = New_Type_With_Str(tk);
    }

    for(;;) {
        if(skip2(TKIND_Lboxbracket, TKIND_Rboxbracket))
            ty = New_Type_With_Ptr(ty);
        else
            break;
    }

    return ty;
}

Ast *make_assign(Ast *dst, Ast *src) {
    if(!dst)
        return NULL;

    return (Ast *)new_node_assign(dst, src);
}

Ast *make_assigneq(char *op, Ast *dst, Ast *src) {
    return NULL; // TODO
}

static Ast *make_block() {
    expect(TKIND_Lbrace);
    Vector *cont = New_Vector();

    Ast *b;

    for(;;) {
        if(skip(TKIND_Rbrace))
            break;
        b = statement();

        if(Ast_isexpr(b)) {
            expect(TKIND_Semicolon);
        }

        vec_push(cont, b);
    }

    return (Ast *)new_node_block(cont);
}

static Ast *make_if(bool isexpr) {
    Ast *cond = expr();

    Ast *then = isexpr ? expr() : make_block();

    if(skip(TKIND_Else)) {
        Ast *el;

        if(skip(TKIND_If))
            el = make_if(isexpr);
        else
            el = isexpr ? expr() : make_block();

        return (Ast *)new_node_if(cond, then, el, isexpr);
    }

    return (Ast *)new_node_if(cond, then, NULL, isexpr);
}

static Ast *make_for() {
    expect(TKIND_Lparen);
    Ast *init = expr();
    expect(TKIND_Semicolon);
    Ast *cond = expr();
    expect(TKIND_Semicolon);
    Ast *reinit = expr();
    expect(TKIND_Rparen);
    Ast *body = statement();

    return (Ast *)new_node_for(init, cond, reinit, body);
}

static Ast *make_while() {
    Ast *cond = expr();

    Ast *body = make_block();

    return (Ast *)new_node_while(cond, body);
}

static Ast *make_return() {
    NodeReturn *ret = new_node_return(expr());

    expect(TKIND_Semicolon);

    return (Ast *)ret;
}

static void make_typedef() {
    mxc_unimplemented("typedef");
    /*
    std::string to = token.get().value;
    Step();
    expect(TKIND_Assign);
    Type *from = eval_type();
    expect(TKIND_Semicolon);
    typemap[to] = from; */
}

static Ast *expr_num(Token *tk) {
    if(strchr(tk->value, '.'))
        return (Ast *)new_node_number_float(atof(tk->value));
    else
        return (Ast *)new_node_number_int(atol(tk->value));
}

static Ast *expr_bool() {
    if(skip(TKIND_True))
        return (Ast *)new_node_bool(true);
    if(skip(TKIND_False))
        return (Ast *)new_node_bool(false);

    return NULL;
}

static Ast *expr_string(Token *tk) { return (Ast *)new_node_string(tk->value); }

static Ast *expr_var(Token *tk) { return (Ast *)new_node_variable(tk->value); }

static Ast *expr_assign() {
    Ast *left = expr_logic_or();

    if(Cur_Token_Is(TKIND_Assign)) {
        if(left == NULL) {
            return NULL;
        }
        /*
        if(left->get_nd_type() != NDTYPE::VARIABLE && left->get_nd_type() !=
        NDTYPE::SUBSCR) { error(token.see(-1).line, token.see(-1).col, "left
        side of the expression is not valid");
        }
        */

        Step();
        left = make_assign(left, expr_assign());
    }

    return left;
}

static Ast *expr_logic_or() {
    Ast *left = expr_logic_and();
    Ast *t;

    for(;;) {
        if(Cur_Token_Is(TKIND_LogOr)) {
            Step();
            t = expr_logic_and();
            left = (Ast *)new_node_binary(BIN_LOR, left, t);
        }
        else if(Cur_Token_Is(TKIND_KOr)) {
            Step();
            t = expr_logic_and();
            left = (Ast *)new_node_binary(BIN_LOR, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_logic_and() {
    Ast *left = expr_equality();
    Ast *t;

    for(;;) {
        if(Cur_Token_Is(TKIND_LogAnd)) {
            Step();
            t = expr_equality();
            left = (Ast *)new_node_binary(BIN_LAND, left, t);
        }
        else if(Cur_Token_Is(TKIND_KAnd)) {
            Step();
            t = expr_equality();
            left = (Ast *)new_node_binary(BIN_LAND, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_equality() {
    Ast *left = expr_comp();

    for(;;) {
        if(Cur_Token_Is(TKIND_Eq)) {
            Step();
            Ast *t = expr_comp();
            left = (Ast *)new_node_binary(BIN_EQ, left, t);
        }
        else if(Cur_Token_Is(TKIND_Neq)) {
            Step();
            Ast *t = expr_comp();
            left = (Ast *)new_node_binary(BIN_NEQ, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_comp() {
    Ast *left = expr_add();
    Ast *t;

    for(;;) {
        if(Cur_Token_Is(TKIND_Lt)) {
            Step();
            t = expr_add();
            left = (Ast *)new_node_binary(BIN_LT, left, t);
        }
        else if(Cur_Token_Is(TKIND_Gt)) {
            Step();
            t = expr_add();
            left = (Ast *)new_node_binary(BIN_GT, left, t);
        }
        else if(Cur_Token_Is(TKIND_Lte)) {
            Step();
            t = expr_add();
            left = (Ast *)new_node_binary(BIN_LTE, left, t);
        }
        else if(Cur_Token_Is(TKIND_Gte)) {
            Step();
            t = expr_add();
            left = (Ast *)new_node_binary(BIN_GTE, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_add() {
    Ast *left = expr_mul();

    for(;;) {
        if(Cur_Token_Is(TKIND_Plus)) {
            Step();
            Ast *t = expr_mul();
            left = (Ast *)new_node_binary(BIN_ADD, left, t);
        }
        else if(Cur_Token_Is(TKIND_Minus)) {
            Step();
            Ast *t = expr_mul();
            left = (Ast *)new_node_binary(BIN_SUB, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_mul() {
    Ast *left = expr_unary();
    Ast *t;

    for(;;) {
        if(Cur_Token_Is(TKIND_Asterisk)) {
            Step();
            t = expr_unary();
            left = (Ast *)new_node_binary(BIN_MUL, left, t);
        }
        else if(Cur_Token_Is(TKIND_Div)) {
            Step();
            t = expr_unary();
            left = (Ast *)new_node_binary(BIN_DIV, left, t);
        }
        else if(Cur_Token_Is(TKIND_Mod)) {
            Step();
            t = expr_unary();
            left = (Ast *)new_node_binary(BIN_MOD, left, t);
        }
        else
            return left;
    }
}

static Ast *expr_unary() {
    int tmp = pos;

    if(Cur_Token_Is(TKIND_Inc) || Cur_Token_Is(TKIND_Dec)
       /*|| Cur_Token_Is("&") || Cur_Token_Is("!") */) {
        enum TKIND tk = Cur_Token()->kind;
        enum UNAOP op = -1;

        switch(tk) {
        case TKIND_Inc:
            op = UNA_INC;
            break;
        case TKIND_Dec:
            op = UNA_DEC;
            break;
        default:
            mxc_unimplemented("error");
        }

        Step();
        Ast *operand = expr_unary();

        return (Ast *)new_node_unary(op, operand);
    }

    pos = tmp;
    return expr_unary_postfix();
}

static Ast *expr_unary_postfix() {
    Ast *left = expr_primary();

    for(;;) {
        if(Cur_Token_Is(TKIND_Dot)) {
            Step();

            Ast *memb = expr_var(Cur_Token());

            Step();
            /*
             *  a.function();
             *            ^
             */
            if(skip(TKIND_Lparen)) {
                Vector *args = New_Vector();
                vec_push(args, left);

                if(skip(TKIND_Rparen))
                    ;
                else
                    for(;;) {
                        vec_push(args, expr());

                        if(skip(TKIND_Rparen))
                            break;

                        expect(TKIND_Comma);
                    }

                left = (Ast *)new_node_fncall(memb, args);
            }
            else { // struct
                left = (Ast *)new_node_member(left, memb);
            }
        }
        else if(Cur_Token_Is(TKIND_Lboxbracket)) {
            Step();
            Ast *index = expr();

            expect(TKIND_Rboxbracket);
            Type *ty = left->ctype->ptr;
            left = (Ast *)new_node_subscript(left, index, ty);
        }
        else if(Cur_Token_Is(TKIND_Lparen)) {
            Step();
            Vector *args = New_Vector();

            if(skip(TKIND_Rparen))
                ;
            else {
                for(;;) {
                    vec_push(args, expr());
                    if(skip(TKIND_Rparen))
                        break;
                    expect(TKIND_Comma);
                }
            }

            // TODO Type check
            left = (Ast *)new_node_fncall(left, args);
        }
        else
            return left;
    }
}

static Ast *expr_primary() {
    if(Cur_Token_Is(TKIND_True) || Cur_Token_Is(TKIND_False)) {
        return expr_bool();
    }
    else if(skip(TKIND_If))
        return make_if(true);
    else if(Cur_Token_Is(TKIND_Identifer)) {
        if(see(1)->kind == TKIND_Colon && see(2)->kind == TKIND_Lbrace) {
            return struct_init();
        }

        Ast *v = expr_var(Get_Step_Token());
        return v;
    }
    else if(Cur_Token_Is(TKIND_Num))
        return expr_num(Get_Step_Token());
    else if(Cur_Token_Is(TKIND_String))
        return expr_string(Get_Step_Token());
    else if(Cur_Token_Is(TKIND_Lparen)) {
        Step();
        Ast *left = expr();

        if(skip(TKIND_Comma)) { // tuple
            if(skip(TKIND_Rparen)) {
                error("error"); // TODO
                return NULL;
            }
            Vector *exs = New_Vector();
            Ast *a;
            Type *ty = New_Type(CTYPE_TUPLE);
            vec_push(exs, left);

            vec_push(ty->tuple, left->ctype);

            for(;;) {
                a = expr();
                vec_push(ty->tuple, a->ctype);
                vec_push(exs, a);
                if(skip(TKIND_Rparen))
                    return (Ast *)new_node_tuple(exs, exs->len, ty);
                expect(TKIND_Comma);
            }
        }

        if(!expect(TKIND_Rparen))
            Step();

        return left;
    }
    else if(Cur_Token_Is(TKIND_Lboxbracket)) {
        Step();
        if(Cur_Token_Is(TKIND_Rboxbracket)) { // TODO: Really?
            error("error");
            return NULL;
        }

        Vector *elem = New_Vector();
        Ast *a = expr();
        vec_push(elem, a);

        for(;;) {
            if(skip(TKIND_Rboxbracket))
                break;
            expect(TKIND_Comma);
            a = expr();
            vec_push(elem, a);
            /*
            if(skip(TKIND_Semicolon)) {
                Ast *nindex = expr();
                if(nindex->ctype->get().type != CTYPE_INT)
                    error("error"); //TODO
                expect("]");
                return new Node_list(elem, nindex);
            }
            */
        }

        return (Ast *)new_node_list(elem, elem->len);
    }
    else if(Cur_Token_Is(TKIND_Semicolon)) {
        Step();
        return NULL;
    }
    else if(Cur_Token_Is(TKIND_Rparen))
        return NULL; //?
    else if(Cur_Token_Is(TKIND_End)) {
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

    return NULL;
}

static Ast *struct_init() {
    /*
     *  let a = StructTag: {
     *      member1: 100,
     *      member2: "hogehoge"
     *  };
     */
    char *tagname = Get_Step_Token()->value;

    Type *tag = New_Type_With_Str(tagname);

    expect(TKIND_Colon);

    expect(TKIND_Lbrace);

    Vector *fields = New_Vector();
    Vector *inits = New_Vector();

    if(skip(TKIND_Rbrace))
        ;
    else
        for(;;) {
            vec_push(fields, expr_var(Get_Step_Token()));

            expect(TKIND_Colon);

            vec_push(inits, expr());

            if(skip(TKIND_Rbrace))
                break;
            expect(TKIND_Comma);
        }

    return (Ast *)new_node_struct_init(tag, fields, inits);
}
