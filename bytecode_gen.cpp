#include "bytecode_gen.h"
#include "bytecode.h"
#include "error.h"
#include "maxc.h"

void BytecodeGenerator::compile(Ast_v &asts, bytecode &iseq) {
    for(Ast *ast : asts)
        gen(ast, iseq, false);

    Bytecode::push_0arg(iseq, OpCode::END);
}

void BytecodeGenerator::gen(Ast *ast, bytecode &iseq, bool use_ret) {
    if(ast == nullptr) {
        return;
    }

    switch(ast->get_nd_type()) {
    case NDTYPE::NUM:
        emit_num(ast, iseq, use_ret);
        break;
    case NDTYPE::BOOL:
        emit_bool(ast, iseq, use_ret);
        break;
    case NDTYPE::CHAR:
        emit_char(ast, iseq, use_ret);
        break;
    case NDTYPE::STRING:
        emit_string(ast, iseq, use_ret);
        break;
    case NDTYPE::LIST:
        emit_list(ast, iseq);
        break;
    case NDTYPE::SUBSCR:
        emit_listaccess(ast, iseq);
        break;
    case NDTYPE::TUPLE:
        emit_tuple(ast, iseq);
        break;
    case NDTYPE::BINARY:
        emit_binop(ast, iseq, use_ret);
        break;
    case NDTYPE::DOT:
        emit_dotop(ast, iseq);
        break;
    case NDTYPE::UNARY:
        emit_unaop(ast, iseq, use_ret);
        break;
    case NDTYPE::TERNARY:
        emit_ternop(ast, iseq);
        break;
    case NDTYPE::ASSIGNMENT:
        emit_assign(ast, iseq);
        break;
    case NDTYPE::IF:
        emit_if(ast, iseq);
        break;
    case NDTYPE::FOR:
        emit_for(ast, iseq);
        break;
    case NDTYPE::WHILE:
        emit_while(ast, iseq);
        break;
    case NDTYPE::BLOCK:
        emit_block(ast, iseq);
        break;
    case NDTYPE::RETURN:
        emit_return(ast, iseq);
        break;
    case NDTYPE::VARIABLE:
        emit_load(ast, iseq);
        break;
    case NDTYPE::FUNCCALL:
        emit_func_call(ast, iseq, use_ret);
        break;
    case NDTYPE::FUNCDEF:
        emit_func_def(ast, iseq);
        break;
    case NDTYPE::VARDECL:
        emit_vardecl(ast, iseq);
        break;
    default:
        error("??? in gen");
    }
}

void BytecodeGenerator::emit_num(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeNumber *n = (NodeNumber *)ast;

    if(n->isfloat) {
        int key = ctable.push_float(n->fnumber);

        Bytecode::push_fpush(iseq, key);
    }
    else {
        if(n->number == 0) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_0);
        }
        if(n->number == 1) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_1);
        }
        else if(n->number == 2) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_2);
        }
        else if(n->number == 3) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_3);
        }
        else
            Bytecode::push_ipush(iseq, n->number);
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_bool(Ast *ast, bytecode &iseq, bool use_ret) {
    auto b = (NodeBool *)ast;

    if(b->boolean)
        Bytecode::push_0arg(iseq, OpCode::PUSHTRUE);
    else
        Bytecode::push_0arg(iseq, OpCode::PUSHFALSE);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_char(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeChar *c = (NodeChar *)ast;

    // vcpush(OpCode::PUSH, (char)c->ch);
}

void BytecodeGenerator::emit_string(Ast *ast, bytecode &iseq, bool use_ret) {
    int key = ctable.push_str(((NodeString *)ast)->string.c_str());

    Bytecode::push_strset(iseq, key);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_list(Ast *ast, bytecode &iseq) {
    auto *l = (NodeList *)ast;

    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen(l->elem[i], iseq, true);
    // vcpush(OpCode::LISTSET, l->nsize);
}

void BytecodeGenerator::emit_listaccess(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;

    if(l->istuple) {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        // vcpush(OpCode::CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        Bytecode::push_0arg(iseq, OpCode::SUBSCR);
    }
}

void BytecodeGenerator::emit_tuple(Ast *ast, bytecode &iseq) {
    auto t = (NodeTuple *)ast;

    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen(t->exprs[i], iseq, true);

    // vcpush(OpCode::TUPLESET, t->nsize);
}

void BytecodeGenerator::emit_binop(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeBinop *b = (NodeBinop *)ast;

    gen(b->left, iseq, true);
    gen(b->right, iseq, true);

    if(!b->ctype->isfloat()) {
        if(b->symbol == "+") {
            Bytecode::push_0arg(iseq, OpCode::ADD);
        }
        else if(b->symbol == "-") {
            Bytecode::push_0arg(iseq, OpCode::SUB);
        }
        else if(b->symbol == "*") {
            Bytecode::push_0arg(iseq, OpCode::MUL);
        }
        else if(b->symbol == "/") {
            Bytecode::push_0arg(iseq, OpCode::DIV);
        }
        else if(b->symbol == "%") {
            Bytecode::push_0arg(iseq, OpCode::MOD);
        }
        else if(b->symbol == "==") {
            Bytecode::push_0arg(iseq, OpCode::EQ);
        }
        else if(b->symbol == "!=") {
            Bytecode::push_0arg(iseq, OpCode::NOTEQ);
        }
        else if(b->symbol == "||") {
            Bytecode::push_0arg(iseq, OpCode::LOGOR);
        }
        else if(b->symbol == "&&") {
            Bytecode::push_0arg(iseq, OpCode::LOGAND);
        }
        else if(b->symbol == "<") {
            Bytecode::push_0arg(iseq, OpCode::LT);
        }
        else if(b->symbol == "<=") {
            Bytecode::push_0arg(iseq, OpCode::LTE);
        }
        else if(b->symbol == ">") {
            Bytecode::push_0arg(iseq, OpCode::GT);
        }
        else if(b->symbol == ">=") {
            Bytecode::push_0arg(iseq, OpCode::GTE);
        }
    }
    else {
        if(b->symbol == "+") {
            Bytecode::push_0arg(iseq, OpCode::FADD);
        }
        else if(b->symbol == "-") {
            Bytecode::push_0arg(iseq, OpCode::FSUB);
        }
        else if(b->symbol == "*") {
            Bytecode::push_0arg(iseq, OpCode::FMUL);
        }
        else if(b->symbol == "/") {
            Bytecode::push_0arg(iseq, OpCode::FDIV);
        }
        else if(b->symbol == "%") {
            Bytecode::push_0arg(iseq, OpCode::FMOD);
        }
        else if(b->symbol == "==") {
            Bytecode::push_0arg(iseq, OpCode::FEQ);
        }
        else if(b->symbol == "!=") {
            Bytecode::push_0arg(iseq, OpCode::FNOTEQ);
        }
        else if(b->symbol == "||") {
            Bytecode::push_0arg(iseq, OpCode::FLOGOR);
        }
        else if(b->symbol == "&&") {
            Bytecode::push_0arg(iseq, OpCode::FLOGAND);
        }
        else if(b->symbol == "<") {
            Bytecode::push_0arg(iseq, OpCode::FLT);
        }
        else if(b->symbol == "<=") {
            Bytecode::push_0arg(iseq, OpCode::FLTE);
        }
        else if(b->symbol == ">") {
            Bytecode::push_0arg(iseq, OpCode::FGT);
        }
        else if(b->symbol == ">=") {
            Bytecode::push_0arg(iseq, OpCode::FGTE);
        }
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_dotop(Ast *ast, bytecode &iseq) {
    auto d = (NodeDotop *)ast;
    gen(d->left, iseq, true);

    if(d->isobj) {
        // CallMethod
        // vcpush(OpCode::CALLMethod, d->method);
    }
    else
        error("unimplemented: struct"); // struct
}

void BytecodeGenerator::emit_ternop(Ast *ast, bytecode &iseq) {
    auto t = (NodeTernop *)ast;

    gen(t->cond, iseq, true);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(t->then, iseq, true);

    size_t then_epos = iseq.size();
    Bytecode::push_jmp(iseq, 0); // goto if statement end

    size_t else_spos = iseq.size();
    Bytecode::replace_int32(cpos, iseq, else_spos);

    gen(t->els, iseq, true);

    size_t epos = iseq.size();
    Bytecode::replace_int32(then_epos, iseq, epos);
}

void BytecodeGenerator::emit_unaop(Ast *ast, bytecode &iseq, bool use_ret) {
    auto u = (NodeUnaop *)ast;

    gen(u->expr, iseq, true);

    /*
    if(u->op == "!") {
        puts("\tcmp $0, %rax");
        puts("\tsete %al");
        puts("\tmovzb %al, %rax");
        return;
    }
    */
    if(u->op == "++") {
        Bytecode::push_0arg(iseq, OpCode::INC);
    }
    else if(u->op == "--") {
        Bytecode::push_0arg(iseq, OpCode::DEC);
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_assign(Ast *ast, bytecode &iseq) {
    // debug("called assign\n");
    auto a = (NodeAssignment *)ast;

    gen(a->src, iseq, true);

    if(a->dst->get_nd_type() == NDTYPE::SUBSCR)
        emit_listaccess_store(a->dst, iseq);
    else
        emit_store(a->dst, iseq);
}

void BytecodeGenerator::emit_store(Ast *ast, bytecode &iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    Bytecode::push_store(iseq, v->vid, v->isglobal);
}

void BytecodeGenerator::emit_listaccess_store(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;

    gen(l->index, iseq, true);
    gen(l->ls, iseq, false);

    Bytecode::push_0arg(iseq, OpCode::SUBSCR_STORE);
}

void BytecodeGenerator::emit_func_def(Ast *ast, bytecode &iseq) {
    auto f = (NodeFunction *)ast;

    bytecode fn_iseq;

    for(int n = f->finfo.args.get().size() - 1; n >= 0; n--) {
        auto a = f->finfo.args.get()[n];
        emit_store(a, fn_iseq);
    }

    for(Ast *b : f->block)
        gen(b, fn_iseq, false);

    Bytecode::push_0arg(fn_iseq, OpCode::RET);

    userfunction fn_object = userfunction(fn_iseq, f->lvars);

    int key = ctable.push_userfunc(fn_object);

    Bytecode::push_functionset(iseq, key);

    /*
    lmap[f->name] = nline;
    vcpush(OpCode::FNBEGIN, f->name);

    vcpush(OpCode::FNEND, f->name);
    */

    emit_store(f->fnvar, iseq);
}

void BytecodeGenerator::emit_if(Ast *ast, bytecode &iseq) {
    auto i = (NodeIf *)ast;

    gen(i->cond, iseq, true);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(i->then_s, iseq, false);

    if(i->else_s) {
        size_t then_epos = iseq.size();
        Bytecode::push_jmp(iseq, 0); // goto if statement end

        size_t else_spos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, else_spos);

        gen(i->else_s, iseq, false);

        size_t epos = iseq.size();
        Bytecode::replace_int32(then_epos, iseq, epos);
    }
    else {
        size_t pos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, pos);
    }
}

void BytecodeGenerator::emit_for(Ast *ast, bytecode &iseq) {
    /*
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init, iseq, true);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OpCode::LABEL, begin);
    if(f->cond) {
        gen(f->cond, iseq, true);
        vcpush(OpCode::JMP_NOTEQ, end);
    }
    gen(f->body, iseq, false);
    if(f->reinit)
        gen(f->reinit, iseq, true);
    vcpush(OpCode::JMP, begin);
    lmap[end] = nline;
    vcpush(OpCode::LABEL, end);*/
}

void BytecodeGenerator::emit_while(Ast *ast, bytecode &iseq) {
    auto w = (NodeWhile *)ast;

    size_t begin = iseq.size();

    gen(w->cond, iseq, true);

    size_t pos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(w->body, iseq, false);

    Bytecode::push_jmp(iseq, begin);

    size_t end = iseq.size();
    Bytecode::replace_int32(pos, iseq, end);
}

void BytecodeGenerator::emit_return(Ast *ast, bytecode &iseq) {
    gen(((NodeReturn *)ast)->cont, iseq, true);

    Bytecode::push_0arg(iseq, OpCode::RET);
}

void BytecodeGenerator::emit_func_call(Ast *ast, bytecode &iseq, bool use_ret) {
    auto f = (NodeFnCall *)ast;

    if(((NodeVariable *)f->func)->finfo.isbuiltin) {
        return emit_bltinfunc_call(f, iseq, use_ret);
    }

    for(auto a : f->args)
        gen(a, iseq, true);

    gen(f->func, iseq, false);

    Bytecode::push_0arg(iseq, OpCode::CALL);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_bltinfunc_call(NodeFnCall *f,
                                            bytecode &iseq,
                                            bool use_ret) {
    for(auto a : f->args)
        gen(a, iseq, true);

    NodeVariable *fn = (NodeVariable *)f->func;

    switch(fn->finfo.fnkind) {
    case BltinFnKind::Print:
        switch(f->args[0]->ctype->type.type) { // XXX
        case CTYPE::INT:
            fn->finfo.fnkind = BltinFnKind::PrintInt;
            break;
        case CTYPE::DOUBLE:
            fn->finfo.fnkind = BltinFnKind::PrintFloat;
            break;
        case CTYPE::BOOL:
            fn->finfo.fnkind = BltinFnKind::PrintBool;
            break;
        case CTYPE::CHAR:
            fn->finfo.fnkind = BltinFnKind::PrintChar;
            break;
        case CTYPE::STRING:
            fn->finfo.fnkind = BltinFnKind::PrintString;
            break;
        default:
            error("unimplemented: Print");
        }
        break;
    case BltinFnKind::Println:
        switch(f->args[0]->ctype->type.type) { // XXX
        case CTYPE::INT:
            fn->finfo.fnkind = BltinFnKind::PrintlnInt;
            break;
        case CTYPE::DOUBLE:
            fn->finfo.fnkind = BltinFnKind::PrintlnFloat;
            break;
        case CTYPE::BOOL:
            fn->finfo.fnkind = BltinFnKind::PrintlnBool;
            break;
        case CTYPE::CHAR:
            fn->finfo.fnkind = BltinFnKind::PrintlnChar;
            break;
        case CTYPE::STRING:
            fn->finfo.fnkind = BltinFnKind::PrintlnString;
            break;
        default:
            debug("%s", f->args[0]->ctype->show().c_str());
            error("unimplemented: Println");
        }
        break;
    default:
        error("unimplemented: No function");
    }

    Bytecode::push_bltinfn_set(iseq, fn->finfo.fnkind);

    Bytecode::push_bltinfn_call(iseq, f->args.size());

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_block(Ast *ast, bytecode &iseq) {
    auto b = (NodeBlock *)ast;

    for(Ast *a : b->cont)
        gen(a, iseq, false);
}

void BytecodeGenerator::emit_vardecl(Ast *ast, bytecode &iseq) {
    NodeVardecl *v = (NodeVardecl *)ast;

    if(v->init != nullptr) {
        gen(v->init, iseq, true);

        emit_store(v->var, iseq);
    }
}

void BytecodeGenerator::emit_load(Ast *ast, bytecode &iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    Bytecode::push_load(iseq, v->vid, v->isglobal);
}

void BytecodeGenerator::show(bytecode &a, size_t &i) {
    printf("%04ld ", i);

    switch((OpCode)a[i++]) {
    case OpCode::PUSH:
        printf("push");
        break;
    case OpCode::IPUSH: {
        int i32 = Bytecode::read_int32(a, i);
        printf("ipush %d", i32);
        break;
    }
    case OpCode::PUSHCONST_0:
        printf("pushconst0");
        break;
    case OpCode::PUSHCONST_1:
        printf("pushconst1");
        break;
    case OpCode::PUSHCONST_2:
        printf("pushconst2");
        break;
    case OpCode::PUSHCONST_3:
        printf("pushconst3");
        break;
    case OpCode::PUSHTRUE:
        printf("pushtrue");
        break;
    case OpCode::PUSHFALSE:
        printf("pushfalse");
        break;
    case OpCode::FPUSH: {
        int id = Bytecode::read_int32(a, i);

        printf("fpush %lf", ctable.table[id].number);
        break;
    }
    case OpCode::POP:
        printf("pop");
        break;
    case OpCode::ADD:
        printf("add");
        break;
    case OpCode::SUB:
        printf("sub");
        break;
    case OpCode::MUL:
        printf("mul");
        break;
    case OpCode::DIV:
        printf("div");
        break;
    case OpCode::MOD:
        printf("mod");
        break;
    case OpCode::LOGOR:
        printf("or");
        break;
    case OpCode::LOGAND:
        printf("and");
        break;
    case OpCode::EQ:
        printf("eq");
        break;
    case OpCode::NOTEQ:
        printf("noteq");
        break;
    case OpCode::LT:
        printf("lt");
        break;
    case OpCode::LTE:
        printf("lte");
        break;
    case OpCode::GT:
        printf("gt");
        break;
    case OpCode::GTE:
        printf("gte");
        break;
    case OpCode::FADD:
        printf("fadd");
        break;
    case OpCode::FSUB:
        printf("fsub");
        break;
    case OpCode::FMUL:
        printf("fmul");
        break;
    case OpCode::FDIV:
        printf("fdiv");
        break;
    case OpCode::FMOD:
        printf("fmod");
        break;
    case OpCode::FLOGOR:
        printf("for");
        break;
    case OpCode::FLOGAND:
        printf("fand");
        break;
    case OpCode::FEQ:
        printf("feq");
        break;
    case OpCode::FNOTEQ:
        printf("fnoteq");
        break;
    case OpCode::FLT:
        printf("flt");
        break;
    case OpCode::FLTE:
        printf("flte");
        break;
    case OpCode::FGT:
        printf("fgt");
        break;
    case OpCode::FGTE:
        printf("fgte");
        break;
    case OpCode::INC:
        printf("inc");
        break;
    case OpCode::DEC:
        printf("dec");
        break;
    case OpCode::JMP: {
        int i32 = Bytecode::read_int32(a, i);
        printf("jmp %d", i32);
        break;
    }
    case OpCode::JMP_EQ:
        printf("jmpeq");
        break;
    case OpCode::JMP_NOTEQ: {
        int i32 = Bytecode::read_int32(a, i);
        printf("jmpneq %d", i32);
        break;
    }
    case OpCode::FORMAT:
        printf("format");
        break;
    case OpCode::TYPEOF:
        printf("typeof");
        break;
    case OpCode::STORE_LOCAL: {
        int id = Bytecode::read_int32(a, i);

        printf("store_local %d", id);

        break;
    }
    case OpCode::STORE_GLOBAL: {
        int id = Bytecode::read_int32(a, i);

        printf("store_global %d", id);

        break;
    }
    case OpCode::LISTSET:
        printf("listset");
        break;
    case OpCode::SUBSCR:
        printf("subscr");
        break;
    case OpCode::SUBSCR_STORE:
        printf("subscr_store");
        break;
    case OpCode::STRINGSET: {
        int k = Bytecode::read_int32(a, i);
        printf("stringset %s", ctable.table[k].str);
        break;
    }
    case OpCode::TUPLESET:
        printf("tupleset");
        break;
    case OpCode::FUNCTIONSET: {
        int k = Bytecode::read_int32(a, i);
        userfunction f = ctable.table[k].func;

        printf("funcset ->\n");

        printf("length: %lu\n", f.code.size());

        for(size_t n = 0; n < f.code.size();) {
            printf("  ");
            show(f.code, n);
            puts("");
        }

        break;
    }
    case OpCode::BLTINFN_SET: {
        int n = Bytecode::read_int32(a, i);

        printf("bltinfn %d", n);

        break;
    }
    case OpCode::LOAD_GLOBAL: {
        int id = Bytecode::read_int32(a, i);

        printf("load_global %d", id);

        break;
    }
    case OpCode::LOAD_LOCAL: {
        int id = Bytecode::read_int32(a, i);

        printf("load_local %d", id);

        break;
    }
    case OpCode::RET:
        printf("ret");
        break;
    case OpCode::CALL:
        printf("call");
        break;
    case OpCode::CALL_BLTIN: {
        int n = Bytecode::read_int32(a, i);

        printf("bltinfn-call arg:%d", n);

        break;
    }
    case OpCode::CALLMethod:
        printf("callmethod");
        break;
    case OpCode::END:
        printf("end");
        break;
    default:
        printf("!Error!");
        break;
    }
}
