#include "maxc.h"

void BytecodeGenerator::compile(
        Ast_v asts,
        Env e,
        bytecode &iseq,
        Constant &ctable_) {
    env = e;
    ctable = ctable_;

    for(Ast *ast: asts)
        gen(ast, iseq);

    Bytecode::push_0arg(iseq, OpCode::END);
}

void BytecodeGenerator::gen(Ast *ast, bytecode &iseq) {
    if(ast == nullptr) {
        return;
    }

    switch(ast->get_nd_type()) {
        case NDTYPE::NUM:
            emit_num(ast, iseq); break;
        case NDTYPE::BOOL:
            emit_bool(ast, iseq); break;
        case NDTYPE::CHAR:
            emit_char(ast, iseq); break;
        case NDTYPE::STRING:
            emit_string(ast, iseq); break;
        case NDTYPE::LIST:
            emit_list(ast, iseq); break;
        case NDTYPE::SUBSCR:
            emit_listaccess(ast, iseq); break;
        case NDTYPE::TUPLE:
            emit_tuple(ast, iseq); break;
        case NDTYPE::BINARY:
            emit_binop(ast, iseq); break;
        case NDTYPE::DOT:
            emit_dotop(ast, iseq); break;
        case NDTYPE::UNARY:
            emit_unaop(ast, iseq); break;
        case NDTYPE::TERNARY:
            emit_ternop(ast, iseq); break;
        case NDTYPE::ASSIGNMENT:
            emit_assign(ast, iseq); break;
        case NDTYPE::IF:
            emit_if(ast, iseq); break;
        case NDTYPE::FOR:
            emit_for(ast, iseq); break;
        case NDTYPE::WHILE:
            emit_while(ast, iseq); break;
        case NDTYPE::BLOCK:
            emit_block(ast, iseq); break;
        case NDTYPE::PRINT:
            emit_print(ast, iseq); break;
        case NDTYPE::PRINTLN:
            emit_println(ast, iseq); break;
        case NDTYPE::FORMAT:
            emit_format(ast, iseq); break;
        case NDTYPE::RETURN:
            emit_return(ast, iseq); break;
        case NDTYPE::VARIABLE:
            emit_load(ast, iseq); break;
        case NDTYPE::FUNCCALL:
            emit_func_call(ast, iseq); break;
        case NDTYPE::FUNCDEF:
            emit_func_def(ast, iseq); break;
        case NDTYPE::VARDECL:
            emit_vardecl(ast, iseq); break;
        default:    error("??? in gen");
    }
}

void BytecodeGenerator::emit_num(Ast *ast, bytecode &iseq) {
    NodeNumber *n = (NodeNumber *)ast;
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

void BytecodeGenerator::emit_bool(Ast *ast, bytecode &iseq) {
    auto b = (NodeBool *)ast;

    if(b->boolean == true)
        Bytecode::push_0arg(iseq, OpCode::PUSHTRUE);
    else if(b->boolean == false)
        Bytecode::push_0arg(iseq, OpCode::PUSHFALSE);
}

void BytecodeGenerator::emit_char(Ast *ast, bytecode &iseq) {
    NodeChar *c = (NodeChar *)ast;

    vcpush(OpCode::PUSH, (char)c->ch);
}

void BytecodeGenerator::emit_string(Ast *ast, bytecode &iseq) {
    vcpush(OpCode::STRINGSET, ((NodeString *)ast)->string);
}

void BytecodeGenerator::emit_list(Ast *ast, bytecode &iseq) {
    auto *l = (NodeList *)ast;

    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen(l->elem[i], iseq);
    vcpush(OpCode::LISTSET, l->nsize);
}

void BytecodeGenerator::emit_listaccess(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;

    if(l->istuple) {
        gen(l->index, iseq);
        gen(l->ls, iseq);
        vcpush(OpCode::CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index, iseq);
        gen(l->ls, iseq);
        vcpush(OpCode::SUBSCR);
    }
}

void BytecodeGenerator::emit_tuple(Ast *ast, bytecode &iseq) {
    auto t = (NodeTuple *)ast;

    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen(t->exprs[i], iseq);
    vcpush(OpCode::TUPLESET, t->nsize);
}

void BytecodeGenerator::emit_binop(Ast *ast, bytecode &iseq) {
    NodeBinop *b = (NodeBinop *)ast;

    gen(b->left, iseq);
    gen(b->right, iseq);

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

void BytecodeGenerator::emit_object_oprator(Ast *ast, bytecode &iseq) {
    auto b = (NodeBinop *)ast;
    gen(b->right, iseq);
    gen(b->left, iseq);
}

void BytecodeGenerator::emit_dotop(Ast *ast, bytecode &iseq) {
    auto d = (NodeDotop *)ast;
    gen(d->left, iseq);
    if(d->isobj) {
        //CallMethod
        vcpush(OpCode::CALLMethod, d->method);
    }
    else error("unimplemented");    //struct
}

void BytecodeGenerator::emit_ternop(Ast *ast, bytecode &iseq) {
    auto t = (NodeTernop *)ast;

    gen(t->cond, iseq);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(t->then, iseq);

    size_t then_epos = iseq.size();
    Bytecode::push_jmp(iseq, 0);    //goto if statement end

    size_t else_spos = iseq.size();
    Bytecode::replace_int32(cpos, iseq, else_spos);

    gen(t->els, iseq);

    size_t epos = iseq.size();
    Bytecode::replace_int32(then_epos, iseq, epos);
}

void BytecodeGenerator::emit_addr(Ast *ast) {
    /*
    assert(ast->get_nd_type() == NDTYPE::VARIABLE);
    NodeVariable *v = (NodeVariable *)ast;
    int off = v->offset;
    printf("\tlea %d(%%rbp), %%rax\n", -off);
    */
}

void BytecodeGenerator::emit_unaop(Ast *ast, bytecode &iseq) {
    auto u = (NodeUnaop *)ast;

    gen(u->expr, iseq);

    /*
    if(u->op == "&") {
        emit_addr(u->expr);
        return;
    }
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
}

void BytecodeGenerator::emit_assign(Ast *ast, bytecode &iseq) {
    //debug("called assign\n");
    auto a = (NodeAssignment *)ast;

    gen(a->src, iseq);
    if(a->dst->get_nd_type() == NDTYPE::SUBSCR)
        emit_listaccess_store(a->dst, iseq);
    else emit_store(a->dst, iseq);
}

void BytecodeGenerator::emit_store(Ast *ast, bytecode &iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    int id = ctable.push_var(v);

    Bytecode::push_store(iseq, id);
}

void BytecodeGenerator::emit_listaccess_store(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;
    gen(l->index, iseq);
    gen(l->ls, iseq);
    vcpush(OpCode::SUBSCR_STORE);
}

void BytecodeGenerator::emit_func_def(Ast *ast, bytecode &iseq) {
    auto f = (NodeFunction *)ast;

    const char *fname = f->finfo.name.c_str();
    vcpush(OpCode::FNBEGIN, fname);
    fnpc.push(nline);

    int n;
    for(n = f->finfo.args.get().size() - 1; n >= 0; n--) {
        auto a = f->finfo.args.get()[n];
        switch(a->ctype->get().type) {
            case CTYPE::INT:
                vcpush(OpCode::ISTORE, a); break;
            default: //TODO
                vcpush(OpCode::STORE, a); break;
        }
    }

    for(Ast *b: f->block) gen(b, iseq);

    Bytecode::push_0arg(iseq, OpCode::RET);
    vcpush(OpCode::FNEND, fname);

    /*
    lmap[f->name] = nline;
    vcpush(OpCode::FNBEGIN, f->name);

    vcpush(OpCode::FNEND, f->name);
    */

    vcpush(OpCode::FUNCTIONSET, fnpc.top(), nline - 1);
    fnpc.pop();
    emit_store(f->fnvar, iseq);
}

void BytecodeGenerator::emit_if(Ast *ast, bytecode &iseq) {
    auto i = (NodeIf *)ast;

    gen(i->cond, iseq);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(i->then_s, iseq);

    if(i->else_s) {
        size_t then_epos = iseq.size();
        Bytecode::push_jmp(iseq, 0);    //goto if statement end

        size_t else_spos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, else_spos);

        gen(i->else_s, iseq);

        size_t epos = iseq.size();
        Bytecode::replace_int32(then_epos, iseq, epos);
    }
    else {
        size_t pos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, pos);
    }
}

void BytecodeGenerator::emit_for(Ast *ast, bytecode &iseq) {
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init, iseq);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OpCode::LABEL, begin);
    if(f->cond) {
        gen(f->cond, iseq);
        vcpush(OpCode::JMP_NOTEQ, end);
    }
    gen(f->body, iseq);
    if(f->reinit)
        gen(f->reinit, iseq);
    vcpush(OpCode::JMP, begin);
    lmap[end] = nline;
    vcpush(OpCode::LABEL, end);
}

void BytecodeGenerator::emit_while(Ast *ast, bytecode &iseq) {
    auto w = (NodeWhile *)ast;

    size_t begin = iseq.size();

    gen(w->cond, iseq);

    size_t pos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(w->body, iseq);

    Bytecode::push_jmp(iseq, begin);

    size_t end = iseq.size();
    Bytecode::replace_int32(pos, iseq, end);
}

void BytecodeGenerator::emit_return(Ast *ast, bytecode &iseq) {
    gen(((NodeReturn *)ast)->cont, iseq);

    Bytecode::push_0arg(iseq, OpCode::RET);
}

void BytecodeGenerator::emit_print(Ast *ast, bytecode &iseq) {
    gen(((NodePrintln *)ast)->cont, iseq);

    Bytecode::push_0arg(iseq, OpCode::PRINT);
}

void BytecodeGenerator::emit_println(Ast *ast, bytecode &iseq) {
    gen(((NodePrintln *)ast)->cont, iseq);

    Bytecode::push_0arg(iseq, OpCode::PRINTLN);
}

void BytecodeGenerator::emit_format(Ast *ast, bytecode &iseq) {
    /*
    auto f = (NodeFormat *)ast;
    for(int n = f->args.size() - 1; n >= 0; --n)
        gen(f->args[n]);

    vcpush(OpCode::FORMAT, f->cont, (unsigned int)f->narg);
    */
}

void BytecodeGenerator::emit_func_call(Ast *ast, bytecode &iseq) {
    auto f = (NodeFnCall *)ast;
    assert(f->func->get_nd_type() == NDTYPE::VARIABLE);
    for(auto a: f->args) gen(a, iseq);
    gen(f->func, iseq);
    vcpush(OpCode::CALL);
}

void BytecodeGenerator::emit_block(Ast *ast, bytecode &iseq) {
    auto b = (NodeBlock *)ast;

    for(Ast *a: b->cont) gen(a, iseq);
}

void BytecodeGenerator::emit_vardecl(Ast *ast, bytecode &iseq) {
    NodeVardecl *v = (NodeVardecl *)ast;
    int n = 0;

    for(NodeVariable *a: v->var.get()) {
        if(v->init[n] != nullptr) {
            //printf("#[debug]: offset is %d\n", a->offset);
            gen(v->init[n], iseq);

            emit_store(a, iseq);
        }
        else {
            if(a->ctype->get().type == CTYPE::INT) {
                vcpush(OpCode::IPUSH, 0);
                vcpush(OpCode::ISTORE, a);
            }
            else {
                vcpush(OpCode::PUSH, 0);
                vcpush(OpCode::STORE, a); //TODO
            }
        }
        ++n;
    }
}

void BytecodeGenerator::emit_load(Ast *ast, bytecode &iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    int id = ctable.push_var(v);

    Bytecode::push_load(iseq, id);
}

char *BytecodeGenerator::get_label() {
    char *l = (char *)malloc(8);
    sprintf(l, "%s%d", ".L", labelnum++);

    return l;
}

void BytecodeGenerator::show(bytecode &a, size_t &i) {
    printf("%04ld ", i);

    switch((OpCode)a[i++]) {
        case OpCode::PUSH:          printf("push"); break;
        case OpCode::IPUSH: {
            int i32 = Bytecode::read_int32(a, i);
            printf("ipush %d", i32);
            break;
        }
        case OpCode::PUSHCONST_1:   printf("pushconst1"); break;
        case OpCode::PUSHCONST_2:   printf("pushconst2"); break;
        case OpCode::PUSHCONST_3:   printf("pushconst3"); break;
        case OpCode::PUSHTRUE:      printf("pushtrue"); break;
        case OpCode::PUSHFALSE:     printf("pushfalse"); break;
        case OpCode::POP:           printf("pop"); break;
        case OpCode::ADD:           printf("add"); break;
        case OpCode::SUB:           printf("sub"); break;
        case OpCode::MUL:           printf("mul"); break;
        case OpCode::DIV:           printf("div"); break;
        case OpCode::MOD:           printf("mod"); break;
        case OpCode::LOGOR:         printf("or");  break;
        case OpCode::LOGAND:        printf("and"); break;
        case OpCode::EQ:            printf("eq");  break;
        case OpCode::NOTEQ:         printf("noteq"); break;
        case OpCode::LT:            printf("lt"); break;
        case OpCode::LTE:           printf("lte"); break;
        case OpCode::GT:            printf("gt"); break;
        case OpCode::GTE:           printf("gte"); break;
        case OpCode::INC:           printf("inc"); break;
        case OpCode::DEC:           printf("dec"); break;
        case OpCode::LABEL:         printf("label"); break;
        case OpCode::JMP: {
            int i32 = Bytecode::read_int32(a, i);
            printf("jmp %d", i32); break;
        }
        case OpCode::JMP_EQ:        printf("jmpeq"); break;
        case OpCode::JMP_NOTEQ: {
            int i32 = Bytecode::read_int32(a, i);
            printf("jmpneq %d", i32); break;
        }
        case OpCode::PRINT:         printf("print"); break;
        case OpCode::PRINTLN:       printf("println"); break;
        case OpCode::FORMAT:        printf("format"); break;
        case OpCode::TYPEOF:        printf("typeof"); break;
        case OpCode::STORE:
        {
            int id = Bytecode::read_int32(a, i);
            printf("store %ld", (size_t)ctable.table[id].var);
            break;
        }
        case OpCode::LISTSET:       printf("listset"); break;
        case OpCode::SUBSCR:        printf("subscr"); break;
        case OpCode::SUBSCR_STORE:  printf("subscr_store"); break;
        case OpCode::STRINGSET:     printf("stringset"); break;
        case OpCode::TUPLESET:      printf("tupleset"); break;
        case OpCode::FUNCTIONSET:   printf("funcset"); break;
        case OpCode::LOAD:
        {
            int id = Bytecode::read_int32(a, i);
            printf("load %ld", (size_t)ctable.table[id].var);
            break;
        }
        case OpCode::RET:           printf("ret"); break;
        case OpCode::CALL:          printf("call"); break;
        case OpCode::CALLMethod:    printf("callmethod"); break;
        case OpCode::FNBEGIN:       printf("fnbegin"); break;
        case OpCode::FNEND:         printf("fnend"); break;
        case OpCode::END:           printf("end"); break;
        default: break;
    }
}

//VMcode push
void BytecodeGenerator::vcpush(OpCode t) {
    //vmcodes.push_back(vmcode_t(t, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, int n) {
    //vmcodes.push_back(vmcode_t(t, n, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, char c) {
    //vmcodes.push_back(vmcode_t(t, c, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, const char *s) {
    //vmcodes.push_back(vmcode_t(t, s, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, NodeVariable *v) {
    //vmcodes.push_back(vmcode_t(t, v, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, char *s, unsigned int n) {
    //vmcodes.push_back(vmcode_t(t, s, n, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, size_t ls) {
    //vmcodes.push_back(vmcode_t(t, ls, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, Method m) {
    //vmcodes.push_back(vmcode_t(t, m, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, size_t fs, size_t fe) {
    //vmcodes.push_back(vmcode_t(t, fs, fe, nline++));
}
