#include "bytecode_gen.h"
#include "bytecode.h"
#include "error.h"
#include "maxc.h"

static void gen(Ast *, Bytecode *iseq, bool);
static void emit_num(Ast *, Bytecode *, bool);
static void emit_bool(Ast *, Bytecode *, bool);
static void emit_char(Ast *, Bytecode *, bool);
static void emit_string(Ast *, Bytecode *, bool);
static void emit_list(Ast *, Bytecode *);
static void emit_listaccess(Ast *, Bytecode *);
static void emit_tuple(Ast *, Bytecode *);
static void emit_binop(Ast *, Bytecode *, bool);
static void emit_member(Ast *, Bytecode *, bool);
static void emit_unaop(Ast *, Bytecode *, bool);
static void emit_if(Ast *, Bytecode *);
static void emit_for(Ast *, Bytecode *);
static void emit_while(Ast *, Bytecode *);
static void emit_return(Ast *, Bytecode *);
static void emit_block(Ast *, Bytecode *);
static void emit_assign(Ast *, Bytecode *);
static void emit_struct_init(Ast *, Bytecode *, bool);
static void emit_store(Ast *, Bytecode *);
static void emit_member_store(Ast *, Bytecode *);
static void emit_listaccess_store(Ast *, Bytecode *);
static void emit_func_def(Ast *, Bytecode *);
static void emit_func_call(Ast *, Bytecode *, bool);
static void emit_bltinfunc_call(NodeFnCall *, Bytecode *, bool);
static void emit_bltinfncall_print(NodeFnCall *, Bytecode *, bool);
static void emit_bltinfncall_println(NodeFnCall *, Bytecode *, bool);
static void emit_vardecl(Ast *, Bytecode *);
static void emit_load(Ast *, Bytecode *, bool);

Vector *ltable;

Bytecode *compile(Vector *ast) {
    Bytecode *iseq = New_Bytecode();
    ltable = New_Vector();

    for(int i = 0; i < ast->len; ++i)
        gen((Ast *)ast->data[i], iseq, false);

    push_0arg(iseq, OP_END);

    return iseq;
}

static void gen(Ast *ast, Bytecode *iseq, bool use_ret) {
    if(ast == NULL) {
        return;
    }

    switch(ast->type) {
    case NDTYPE_NUM:
        emit_num(ast, iseq, use_ret);
        break;
    case NDTYPE_BOOL:
        emit_bool(ast, iseq, use_ret);
        break;
    case NDTYPE_CHAR:
        emit_char(ast, iseq, use_ret);
        break;
    case NDTYPE_STRING:
        emit_string(ast, iseq, use_ret);
        break;
    case NDTYPE_STRUCT:
        break;
    case NDTYPE_STRUCTINIT:
        emit_struct_init(ast, iseq, use_ret);
        break;
    case NDTYPE_LIST:
        emit_list(ast, iseq);
        break;
    case NDTYPE_SUBSCR:
        emit_listaccess(ast, iseq);
        break;
    case NDTYPE_TUPLE:
        emit_tuple(ast, iseq);
        break;
    case NDTYPE_BINARY:
        emit_binop(ast, iseq, use_ret);
        break;
    case NDTYPE_MEMBER:
        emit_member(ast, iseq, use_ret);
        break;
    case NDTYPE_UNARY:
        emit_unaop(ast, iseq, use_ret);
        break;
    case NDTYPE_ASSIGNMENT:
        emit_assign(ast, iseq);
        break;
    case NDTYPE_IF:
    case NDTYPE_EXPRIF:
        emit_if(ast, iseq);
        break;
    case NDTYPE_FOR:
        emit_for(ast, iseq);
        break;
    case NDTYPE_WHILE:
        emit_while(ast, iseq);
        break;
    case NDTYPE_BLOCK:
        emit_block(ast, iseq);
        break;
    case NDTYPE_RETURN:
        emit_return(ast, iseq);
        break;
    case NDTYPE_VARIABLE:
        emit_load(ast, iseq, use_ret);
        break;
    case NDTYPE_FUNCCALL:
        emit_func_call(ast, iseq, use_ret);
        break;
    case NDTYPE_FUNCDEF:
        emit_func_def(ast, iseq);
        break;
    case NDTYPE_VARDECL:
        emit_vardecl(ast, iseq);
        break;
    default:
        error("??? in gen");
    }
}

static void emit_num(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeNumber *n = (NodeNumber *)ast;

    if(type_is(CAST_AST(n)->ctype, CTYPE_DOUBLE)) {
        int key = lpool_push_float(ltable, n->fnumber);

        push_fpush(iseq, key);
    }
    else {
        if(n->number == 0) {
            push_0arg(iseq, OP_PUSHCONST_0);
        }
        else if(n->number == 1) {
            push_0arg(iseq, OP_PUSHCONST_1);
        }
        else if(n->number == 2) {
            push_0arg(iseq, OP_PUSHCONST_2);
        }
        else if(n->number == 3) {
            push_0arg(iseq, OP_PUSHCONST_3);
        }
        else
            push_ipush(iseq, n->number);
    }

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

void emit_bool(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeBool *b = (NodeBool *)ast;

    if(b->boolean)
        push_0arg(iseq, OP_PUSHTRUE);
    else
        push_0arg(iseq, OP_PUSHFALSE);

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

void emit_char(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeChar *c = (NodeChar *)ast;

    // vcpush(OP_PUSH, (char)c->ch);
}

void emit_string(Ast *ast, Bytecode *iseq, bool use_ret) {
    int key = lpool_push_str(ltable, ((NodeString *)ast)->string);

    push_strset(iseq, key);

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

void emit_list(Ast *ast, Bytecode *iseq) {
    NodeList *l = (NodeList *)ast;

    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen((Ast *)l->elem->data[i], iseq, true);
    // vcpush(OP_LISTSET, l->nsize);
}

void emit_struct_init(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeStructInit *s = (NodeStructInit *)ast;

    push_structset(iseq, CAST_AST(s)->ctype->strct.nfield);

    //TODO
}

void emit_listaccess(Ast *ast, Bytecode *iseq) {
    NodeSubscript *l = (NodeSubscript *)ast;

    if(l->istuple) {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        // vcpush(OP_CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        push_0arg(iseq, OP_SUBSCR);
    }
}

static void emit_tuple(Ast *ast, Bytecode *iseq) {
    NodeTuple *t = (NodeTuple *)ast;

    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen((Ast *)t->exprs->data[i], iseq, true);

    // vcpush(OP_TUPLESET, t->nsize);
}

static void emit_binop(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeBinop *b = (NodeBinop *)ast;

    gen(b->left, iseq, true);
    gen(b->right, iseq, true);

    if(!type_is(b->left->ctype, CTYPE_DOUBLE)) {
        switch(b->op) {
        case BIN_ADD:
            push_0arg(iseq, OP_ADD);
            break;
        case BIN_SUB:
            push_0arg(iseq, OP_SUB);
            break;
        case BIN_MUL:
            push_0arg(iseq, OP_MUL);
            break;
        case BIN_DIV:
            push_0arg(iseq, OP_DIV);
            break;
        case BIN_MOD:
            push_0arg(iseq, OP_MOD);
            break;
        case BIN_EQ:
            push_0arg(iseq, OP_EQ);
            break;
        case BIN_NEQ:
            push_0arg(iseq, OP_NOTEQ);
            break;
        case BIN_LOR:
            push_0arg(iseq, OP_LOGOR);
            break;
        case BIN_LAND:
            push_0arg(iseq, OP_LOGAND);
            break;
        case BIN_LT:
            push_0arg(iseq, OP_LT);
            break;
        case BIN_LTE:
            push_0arg(iseq, OP_LTE);
            break;
        case BIN_GT:
            push_0arg(iseq, OP_GT);
            break;
        case BIN_GTE:
            push_0arg(iseq, OP_GTE);
            break;
        }
    }
    else {
        switch(b->op) {
        case BIN_ADD:
            push_0arg(iseq, OP_FADD);
            break;
        case BIN_SUB:
            push_0arg(iseq, OP_FSUB);
            break;
        case BIN_MUL:
            push_0arg(iseq, OP_FMUL);
            break;
        case BIN_DIV:
            push_0arg(iseq, OP_FDIV);
            break;
        case BIN_MOD:
            push_0arg(iseq, OP_FMOD);
            break;
        case BIN_EQ:
            push_0arg(iseq, OP_FEQ);
            break;
        case BIN_NEQ:
            push_0arg(iseq, OP_FNOTEQ);
            break;
        case BIN_LOR:
            push_0arg(iseq, OP_FLOGOR);
            break;
        case BIN_LAND:
            push_0arg(iseq, OP_FLOGAND);
            break;
        case BIN_LT:
            push_0arg(iseq, OP_FLT);
            break;
        case BIN_LTE:
            push_0arg(iseq, OP_FLTE);
            break;
        case BIN_GT:
            push_0arg(iseq, OP_FGT);
            break;
        case BIN_GTE:
            push_0arg(iseq, OP_FGTE);
            break;
        }
    }

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

void emit_member(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeMember *m = (NodeMember *)ast;

    gen(m->left, iseq, true);

    NodeVariable *rhs = (NodeVariable *)m->right;

    size_t i = 0;
    for(; i < m->left->ctype->strct.nfield; ++i) {
        if(m->left->ctype->strct.field[i]->name == rhs->name) {
            break;
        }
    }

    push_member_load(iseq, i);
}

void emit_unaop(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeUnaop *u = (NodeUnaop *)ast;

    gen(u->expr, iseq, true);

    /*
    if(u->op == "!") {
        puts("\tcmp $0, %rax");
        puts("\tsete %al");
        puts("\tmovzb %al, %rax");
        return;
    }
    */
    switch(u->op) {
    case UNA_INC:
        push_0arg(iseq, OP_INC);
        break;
    case UNA_DEC:
        push_0arg(iseq, OP_DEC);
        break;
    default:
        mxc_unimplemented("sorry");
    }

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

static void emit_assign(Ast *ast, Bytecode *iseq) {
    // debug("called assign\n");
    NodeAssignment *a = (NodeAssignment *)ast;

    gen(a->src, iseq, true);

    if(a->dst->type == NDTYPE_SUBSCR)
        emit_listaccess_store(a->dst, iseq);
    else if(a->dst->type == NDTYPE_MEMBER)
        emit_member_store(a->dst, iseq);
    else
        emit_store(a->dst, iseq);
}

static void emit_store(Ast *ast, Bytecode *iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    push_store(iseq, v->vid, v->isglobal);
}

static void emit_member_store(Ast *ast, Bytecode *iseq) {
    NodeMember *m = (NodeMember *)ast;

    gen(m->left, iseq, true);

    NodeVariable *rhs = (NodeVariable *)m->right;

    size_t i = 0;
    for(; i < m->left->ctype->strct.nfield; ++i) {
        if(m->left->ctype->strct.field[i]->name == rhs->name) {
            break;
        }
    }

    push_member_store(iseq, i);
}

static void emit_listaccess_store(Ast *ast, Bytecode *iseq) {
    NodeSubscript *l = (NodeSubscript *)ast;

    gen(l->index, iseq, true);
    gen(l->ls, iseq, false);

    push_0arg(iseq, OP_SUBSCR_STORE);
}

static void emit_func_def(Ast *ast, Bytecode *iseq) {
    NodeFunction *f = (NodeFunction *)ast;

    Bytecode *fn_iseq = New_Bytecode();

    for(int n = f->finfo.args->vars->len - 1; n >= 0; n--) {
        NodeVariable *a = f->finfo.args->vars->data[n];
        emit_store((Ast *)a, fn_iseq);
    }

    if(f->block->type == NDTYPE_BLOCK) {
        NodeBlock *b = (NodeBlock *)f->block;
        for(size_t i = 0; i < b->cont->len; i++) {
            gen(b->cont->data[i],
                fn_iseq,
                i == b->cont->len - 1 ? true : false); // last expression
        }
    }
    else {
        gen(f->block, fn_iseq, true);
    }

    push_0arg(fn_iseq, OP_RET);

    userfunction *fn_object = New_Userfunction(fn_iseq, f->lvars);

    int key = lpool_push_userfunc(ltable, fn_object);

    push_functionset(iseq, key);

    /*
    lmap[f->name] = nline;
    vcpush(OP_FNBEGIN, f->name);

    vcpush(OP_FNEND, f->name);
    */

    emit_store((Ast *)f->fnvar, iseq);
}

static void emit_if(Ast *ast, Bytecode *iseq) {
    NodeIf *i = (NodeIf *)ast;

    gen(i->cond, iseq, true);

    size_t cpos = iseq->len;
    push_jmpneq(iseq, 0);

    gen(i->then_s, iseq, i->isexpr);

    if(i->else_s) {
        size_t then_epos = iseq->len;
        push_jmp(iseq, 0); // goto if statement end

        size_t else_spos = iseq->len;
        replace_int32(cpos, iseq, else_spos);

        gen(i->else_s, iseq, i->isexpr);

        size_t epos = iseq->len;
        replace_int32(then_epos, iseq, epos);
    }
    else {
        size_t pos = iseq->len;
        replace_int32(cpos, iseq, pos);
    }
}

void emit_for(Ast *ast, Bytecode *iseq) {
    /*
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init, iseq, true);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OP_LABEL, begin);
    if(f->cond) {
        gen(f->cond, iseq, true);
        vcpush(OP_JMP_NOTEQ, end);
    }
    gen(f->body, iseq, false);
    if(f->reinit)
        gen(f->reinit, iseq, true);
    vcpush(OP_JMP, begin);
    lmap[end] = nline;
    vcpush(OP_LABEL, end);*/
}

void emit_while(Ast *ast, Bytecode *iseq) {
    NodeWhile *w = (NodeWhile *)ast;

    size_t begin = iseq->len;

    gen(w->cond, iseq, true);

    size_t pos = iseq->len;
    push_jmpneq(iseq, 0);

    gen(w->body, iseq, false);

    push_jmp(iseq, begin);

    size_t end = iseq->len;
    replace_int32(pos, iseq, end);
}

static void emit_return(Ast *ast, Bytecode *iseq) {
    gen(((NodeReturn *)ast)->cont, iseq, true);

    push_0arg(iseq, OP_RET);
}

static void emit_func_call(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeFnCall *f = (NodeFnCall *)ast;

    if(((NodeVariable *)f->func)->finfo.isbuiltin) {
        return emit_bltinfunc_call(f, iseq, use_ret);
    }

    for(int i = 0; i < f->args->len; ++i)
        gen((Ast *)f->args->data[i], iseq, true);

    gen(f->func, iseq, true);

    push_0arg(iseq, OP_CALL);

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

static void emit_bltinfunc_call(NodeFnCall *f,
                         Bytecode *iseq,
                         bool use_ret) {

    NodeVariable *fn = (NodeVariable *)f->func;

    if(fn->finfo.fnkind == BLTINFN_PRINT) {
        return emit_bltinfncall_print(f, iseq, false);
    }

    if(fn->finfo.fnkind == BLTINFN_PRINTLN) {
        return emit_bltinfncall_println(f, iseq, false);
    }

    for(int i = 0; i < f->args->len; ++i)
        gen((Ast *)f->args->data[i], iseq, true);

    enum BLTINFN callfn = fn->finfo.fnkind;

    switch(fn->finfo.fnkind) {
    case BLTINFN_STRINGSIZE:
        callfn = BLTINFN_STRINGSIZE;
        break;
    case BLTINFN_STRINGISEMPTY:
        callfn = BLTINFN_STRINGISEMPTY;
        break;
    case BLTINFN_INTTOFLOAT:
        callfn = BLTINFN_INTTOFLOAT;
        break;
    case BLTINFN_OBJECTID:
        callfn = BLTINFN_OBJECTID;
        break;
    default:
        error("unimplemented: No function in bytecode_gen.c");
    }

    push_bltinfn_set(iseq, callfn);

    push_bltinfn_call(iseq, f->args->len);

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}

static void emit_bltinfncall_println(NodeFnCall *f,
                              Bytecode *iseq,
                              bool use_ret) {
    NodeVariable *fn = (NodeVariable *)f->func;

    for(size_t i = 0; i < f->args->len; ++i) {
        enum BLTINFN callfn = fn->finfo.fnkind;

        gen((Ast *)f->args->data[i], iseq, true);

        switch(CAST_AST(f->args->data[i])->ctype->type) {
        case CTYPE_INT:
            callfn = i != f->args->len - 1 ? BLTINFN_PRINTINT
                                           : BLTINFN_PRINTLNINT;
            break;
        case CTYPE_DOUBLE:
            callfn = i != f->args->len - 1 ? BLTINFN_PRINTFLOAT
                                           : BLTINFN_PRINTLNFLOAT;
            break;
        case CTYPE_BOOL:
            callfn = i != f->args->len - 1 ? BLTINFN_PRINTBOOL
                                           : BLTINFN_PRINTLNBOOL;
            break;
        case CTYPE_CHAR:
            callfn = i != f->args->len - 1 ? BLTINFN_PRINTCHAR
                                           : BLTINFN_PRINTLNCHAR;
            break;
        case CTYPE_STRING:
            callfn = i != f->args->len - 1 ? BLTINFN_PRINTSTRING
                                           : BLTINFN_PRINTLNSTRING;
            break;
        default:
            error("unimplemented: Print");
        }

        push_bltinfn_set(iseq, callfn);

        push_bltinfn_call(iseq, 1);

        if(!use_ret)
            push_0arg(iseq, OP_POP);
    }
}

static void emit_bltinfncall_print(NodeFnCall *f,
                            Bytecode *iseq,
                            bool use_ret) {
    NodeVariable *fn = (NodeVariable *)f->func;

    for(size_t i = 0; i < f->args->len; ++i) {
        enum BLTINFN callfn = fn->finfo.fnkind;

        gen((Ast *)f->args->data[i], iseq, true);

        switch(CAST_AST(f->args->data[i])->ctype->type) {
        case CTYPE_INT:
            callfn = BLTINFN_PRINTINT;
            break;
        case CTYPE_DOUBLE:
            callfn = BLTINFN_PRINTFLOAT;
            break;
        case CTYPE_BOOL:
            callfn = BLTINFN_PRINTBOOL;
            break;
        case CTYPE_CHAR:
            callfn = BLTINFN_PRINTCHAR;
            break;
        case CTYPE_STRING:
            callfn = BLTINFN_PRINTSTRING;
            break;
        default:
            error("unimplemented: Print");
        }

        push_bltinfn_set(iseq, callfn);

        push_bltinfn_call(iseq, 1);

        if(!use_ret)
            push_0arg(iseq, OP_POP);
    }
}

static void emit_block(Ast *ast, Bytecode *iseq) {
    NodeBlock *b = (NodeBlock *)ast;

    for(int i = 0; i < b->cont->len; ++i)
        gen((Ast *)b->cont->data[i], iseq, false);
}

static void emit_vardecl(Ast *ast, Bytecode *iseq) {
    NodeVardecl *v = (NodeVardecl *)ast;

    if(v->init != NULL) {
        gen(v->init, iseq, true);

        emit_store((Ast *)v->var, iseq);
    }
}

static void emit_load(Ast *ast, Bytecode *iseq, bool use_ret) {
    NodeVariable *v = (NodeVariable *)ast;

    push_load(iseq, v->vid, v->isglobal);

    if(!use_ret)
        push_0arg(iseq, OP_POP);
}
