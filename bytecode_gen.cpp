#include "maxc.h"

void BytecodeGenerator::compile(Ast_v asts, Env e) {
    env = e;
    for(Ast *ast: asts)
        gen(ast);
    vcpush(OpCode::END);
}

void BytecodeGenerator::gen(Ast *ast) {
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case NDTYPE::NUM:
                emit_num(ast); break;
            case NDTYPE::BOOL:
                emit_bool(ast); break;
            case NDTYPE::CHAR:
                emit_char(ast); break;
            case NDTYPE::STRING:
                emit_string(ast); break;
            case NDTYPE::LIST:
                emit_list(ast); break;
            case NDTYPE::SUBSCR:
                emit_listaccess(ast); break;
            case NDTYPE::TUPLE:
                emit_tuple(ast); break;
            case NDTYPE::BINARY:
                emit_binop(ast); break;
            case NDTYPE::DOT:
                emit_dotop(ast); break;
            case NDTYPE::UNARY:
                emit_unaop(ast); break;
            case NDTYPE::TERNARY:
                emit_ternop(ast); break;
            case NDTYPE::ASSIGNMENT:
                emit_assign(ast); break;
            case NDTYPE::IF:
                emit_if(ast); break;
            case NDTYPE::FOR:
                emit_for(ast); break;
            case NDTYPE::WHILE:
                emit_while(ast); break;
            case NDTYPE::BLOCK:
                emit_block(ast); break;
            case NDTYPE::PRINT:
                emit_print(ast); break;
            case NDTYPE::PRINTLN:
                emit_println(ast); break;
            case NDTYPE::FORMAT:
                emit_format(ast); break;
            case NDTYPE::TYPEOF:
                emit_typeof(ast); break;
            case NDTYPE::RETURN:
                emit_return(ast); break;
            case NDTYPE::VARIABLE:
                emit_load(ast); break;
            case NDTYPE::FUNCCALL:
                emit_func_call(ast); break;
            case NDTYPE::FUNCDEF:
                emit_func_def(ast); break;
            case NDTYPE::VARDECL:
                emit_vardecl(ast); break;
            default:    error("??? in gen");
        }
    }
}

void BytecodeGenerator::emit_num(Ast *ast) {
    NodeNumber *n = (NodeNumber *)ast;
    if(n->number == 1) {
        vcpush(OpCode::PUSHCONST_1);
    }
    else if(n->number == 2) {
        vcpush(OpCode::PUSHCONST_2);
    }
    else if(n->number == 3) {
        vcpush(OpCode::PUSHCONST_3);
    }
    else
        vcpush(OpCode::IPUSH, n->number);
}

void BytecodeGenerator::emit_bool(Ast *ast) {
    auto b = (NodeBool *)ast;
    if(b->boolean == true)
        vcpush(OpCode::PUSHTRUE);
    else if(b->boolean == false)
        vcpush(OpCode::PUSHFALSE);
}

void BytecodeGenerator::emit_char(Ast *ast) {
    NodeChar *c = (NodeChar *)ast;
    vcpush(OpCode::PUSH, (char)c->ch);
}

void BytecodeGenerator::emit_string(Ast *ast) {
    auto *s = (NodeString *)ast;
    vcpush(OpCode::STRINGSET, s->string);
}

void BytecodeGenerator::emit_list(Ast *ast) {
    auto *l = (NodeList *)ast;
    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen(l->elem[i]);
    vcpush(OpCode::LISTSET, l->nsize);
}

void BytecodeGenerator::emit_listaccess(Ast *ast) {
    auto l = (NodeSubscript *)ast;
    if(l->istuple) {
        gen(l->index);
        gen(l->ls);
        vcpush(OpCode::CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index);
        gen(l->ls);
        vcpush(OpCode::SUBSCR);
    }
}

void BytecodeGenerator::emit_tuple(Ast *ast) {
    auto t = (NodeTuple *)ast;
    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen(t->exprs[i]);
    vcpush(OpCode::TUPLESET, t->nsize);
}

void BytecodeGenerator::emit_binop(Ast *ast) {
    NodeBinop *b = (NodeBinop *)ast;

    if(b->ctype->isobject()) {
        emit_object_oprator(b); return;
    }

    gen(b->left);
    gen(b->right);

    if(b->symbol == "+") {
        vcpush(OpCode::ADD); return;
    }
    if(b->symbol == "-") {
        vcpush(OpCode::SUB); return;
    }
    if(b->symbol == "*") {
        vcpush(OpCode::MUL); return;
    }
    if(b->symbol == "/") {
        vcpush(OpCode::DIV); return;
    }
    if(b->symbol == "%") {
        vcpush(OpCode::MOD); return;
    }
    if(b->symbol == "==") {
        vcpush(OpCode::EQ); return;
    }
    if(b->symbol == "!=") {
        vcpush(OpCode::NOTEQ); return;
    }
    if(b->symbol == "||") {
        vcpush(OpCode::LOGOR); return;
    }
    if(b->symbol == "&&") {
        vcpush(OpCode::LOGAND); return;
    }
    if(b->symbol == "<") {
        vcpush(OpCode::LT); return;
    }
    if(b->symbol == "<=") {
        vcpush(OpCode::LTE); return;
    }
    if(b->symbol == ">") {
        vcpush(OpCode::GT); return;
    }
    if(b->symbol == ">=") {
        vcpush(OpCode::GTE); return;
    }
}

void BytecodeGenerator::emit_object_oprator(Ast *ast) {
    auto b = (NodeBinop *)ast;
    gen(b->right);
    gen(b->left);
}

void BytecodeGenerator::emit_dotop(Ast *ast) {
    auto d = (NodeDotop *)ast;
    gen(d->left);
    if(d->isobj) {
        //CallMethod
        vcpush(OpCode::CALLMethod, d->method);
    }
    else error("unimplemented");    //struct
}

void BytecodeGenerator::emit_ternop(Ast *ast) {
    auto t = (NodeTernop *)ast;

    gen(t->cond);
    char *l1 = get_label();
    vcpush(OpCode::JMP_NOTEQ, l1);
    gen(t->then);

    char *l2 = get_label();
    vcpush(OpCode::JMP, l2);
    lmap[l1] = nline;
    vcpush(OpCode::LABEL, l1);
    gen(t->els);
    lmap[l2] = nline;
    vcpush(OpCode::LABEL, l2);
}

void BytecodeGenerator::emit_pointer(NodeBinop *b) {
    /*
    gen(b->left);
    puts("\tpush %rax");
    gen(b->right);
    NodeVariable *v = (NodeVariable *)b->left;
    int size = v->vinfo.type->get_size();

    if(size > 1)
        printf("\timul $%d, %%rax\n", size);
    puts("\tmov %rax, %rdi");
    puts("\tpop %rax");
    puts("\tadd %rdi, %rax"); */
}

void BytecodeGenerator::emit_addr(Ast *ast) {
    /*
    assert(ast->get_nd_type() == NDTYPE::VARIABLE);
    NodeVariable *v = (NodeVariable *)ast;
    int off = v->offset;
    printf("\tlea %d(%%rbp), %%rax\n", -off);
    */
}

void BytecodeGenerator::emit_unaop(Ast *ast) {
    auto u = (NodeUnaop *)ast;
    gen(u->expr);

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
    if(u->op == "++")
        vcpush(OpCode::INC);
    else if(u->op == "--")
        vcpush(OpCode::DEC);
    emit_store(u->expr);
}

void BytecodeGenerator::emit_assign(Ast *ast) {
    //debug("called assign\n");
    auto a = (NodeAssignment *)ast;

    gen(a->src);
    if(a->dst->get_nd_type() == NDTYPE::SUBSCR)
        emit_listaccess_store(a->dst);
    else emit_store(a->dst);
}

void BytecodeGenerator::emit_store(Ast *ast) {
    NodeVariable *v = (NodeVariable *)ast;

    if(v->ctype->get().type == CTYPE::INT)
        vcpush(OpCode::ISTORE, v);
    else
        vcpush(OpCode::STORE, v); //TODO
}

void BytecodeGenerator::emit_listaccess_store(Ast *ast) {
    auto l = (NodeSubscript *)ast;
    gen(l->index);
    gen(l->ls);
    vcpush(OpCode::SUBSCR_STORE);
}

void BytecodeGenerator::emit_func_def(Ast *ast) {
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

    for(Ast *b: f->block) gen(b);
    vcpush(OpCode::RET);
    vcpush(OpCode::FNEND, fname);

    /*
    lmap[f->name] = nline;
    vcpush(OpCode::FNBEGIN, f->name);

    vcpush(OpCode::FNEND, f->name);
    */

    vcpush(OpCode::FUNCTIONSET, fnpc.top(), nline - 1);
    fnpc.pop();
    emit_store(f->fnvar);
}

void BytecodeGenerator::emit_if(Ast *ast) {
    auto i = (NodeIf *)ast;

    gen(i->cond);
    char *l1 = get_label();
    vcpush(OpCode::JMP_NOTEQ, l1);
    gen(i->then_s);

    if(i->else_s) {
        char *l2 = get_label();
        vcpush(OpCode::JMP, l2);
        lmap[l1] = nline;
        vcpush(OpCode::LABEL, l1);
        gen(i->else_s);
        lmap[l2] = nline;
        vcpush(OpCode::LABEL, l2);
    }
    else {
        lmap[l1] = nline;
        vcpush(OpCode::LABEL, l1);
    }
}

void BytecodeGenerator::emit_for(Ast *ast) {
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OpCode::LABEL, begin);
    if(f->cond) {
        gen(f->cond);
        vcpush(OpCode::JMP_NOTEQ, end);
    }
    gen(f->body);
    if(f->reinit)
        gen(f->reinit);
    vcpush(OpCode::JMP, begin);
    lmap[end] = nline;
    vcpush(OpCode::LABEL, end);
}

void BytecodeGenerator::emit_while(Ast *ast) {
    auto w = (NodeWhile *)ast;
    char *begin = get_label();
    char *end = get_label();

    lmap[begin] = nline;
    vcpush(OpCode::LABEL, begin);
    gen(w->cond);
    vcpush(OpCode::JMP_NOTEQ, end);
    gen(w->body);
    vcpush(OpCode::JMP, begin);
    lmap[end] = nline;
    vcpush(OpCode::LABEL, end);
}

void BytecodeGenerator::emit_return(Ast *ast) {
    auto r = (NodeReturn *)ast;
    gen(r->cont);

    vcpush(OpCode::RET);
}

void BytecodeGenerator::emit_print(Ast *ast) {
    auto p = (NodePrint *)ast;
    gen(p->cont);
    vcpush(OpCode::PRINT);
}

void BytecodeGenerator::emit_println(Ast *ast) {
    auto p = (NodePrintln *)ast;
    gen(p->cont);
    vcpush(OpCode::PRINTLN);
}

void BytecodeGenerator::emit_format(Ast *ast) {
    /*
    auto f = (NodeFormat *)ast;
    for(int n = f->args.size() - 1; n >= 0; --n)
        gen(f->args[n]);

    vcpush(OpCode::FORMAT, f->cont, (unsigned int)f->narg);
    */
}

void BytecodeGenerator::emit_typeof(Ast *ast) {
    auto t = (NodeTypeof *)ast;
    gen(t->var);
    vcpush(OpCode::TYPEOF);
}

void BytecodeGenerator::emit_func_call(Ast *ast) {
    auto f = (NodeFnCall *)ast;
    assert(f->func->get_nd_type() == NDTYPE::VARIABLE);
    for(auto a: f->args) gen(a);
    gen(f->func);
    vcpush(OpCode::CALL);
}

void BytecodeGenerator::emit_block(Ast *ast) {
    auto b = (NodeBlock *)ast;

    for(Ast *a: b->cont) gen(a);
}

void BytecodeGenerator::emit_vardecl(Ast *ast) {
    NodeVardecl *v = (NodeVardecl *)ast;
    int n = 0;

    for(NodeVariable *a: v->var.get()) {
        if(v->init[n] != nullptr) {
            //printf("#[debug]: offset is %d\n", a->offset);
            gen(v->init[n]);
            if(a->ctype->get().type == CTYPE::INT)
                vcpush(OpCode::ISTORE, a);
            else
                vcpush(OpCode::STORE, a); //TODO
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

void BytecodeGenerator::emit_load(Ast *ast) {
    NodeVariable *v = (NodeVariable *)ast;
    vcpush(OpCode::LOAD, v);
}

char *BytecodeGenerator::get_label() {
    char *l = (char *)malloc(8);
    sprintf(l, "%s%d", ".L", labelnum++);

    return l;
}

void BytecodeGenerator::show(vmcode_t &a) {
    printf("%04d ", a.nline);
    opcode2str(a.type);
    switch(a.type) {
        //case OpCode::PUSH:
        case OpCode::IPUSH:
            printf(" %d", a.num); break;
        case OpCode::STORE:
        case OpCode::ISTORE:
        case OpCode::LOAD:
            //printf(" %s(id:%d)", a.var->var->vinfo.name.c_str(), a.var->var->vid);
            if(a.var->ctype->isfunction())
                std::cout << " `" << a.var->finfo.name << "`(id:" << a.var->vid << ")";
            else
                std::cout << " `" << a.var->vinfo.name << "`(id:" << a.var->vid << ")";
            break;
        case OpCode::LABEL:
        case OpCode::JMP:
        case OpCode::JMP_EQ:
        case OpCode::JMP_NOTEQ:
        case OpCode::FNBEGIN:
        case OpCode::FNEND:
            printf(" %s(%d)", a.str, lmap[a.str]); break;

        case OpCode::FORMAT:
            printf(" \"%s\", %d", a.str, a.nfarg); break;
        case OpCode::LISTSET:
        case OpCode::TUPLESET:
            printf(" (size: %d)", (int)a.size); break;
        case OpCode::STRINGSET:
            printf(" %s", a.str); break;
        case OpCode::FUNCTIONSET:
            printf(" %ld - %ld", a.fnstart, a.fnend); break;

        default:
            break;
    }
}

void BytecodeGenerator::opcode2str(OpCode o) {
    switch(o) {
        case OpCode::PUSH:          printf("push"); break;
        case OpCode::IPUSH:         printf("ipush"); break;
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
        case OpCode::JMP:           printf("jmp"); break;
        case OpCode::JMP_EQ:        printf("jmp_eq"); break;
        case OpCode::JMP_NOTEQ:     printf("jmp_neq"); break;
        case OpCode::PRINT:         printf("print"); break;
        case OpCode::PRINTLN:       printf("println"); break;
        case OpCode::FORMAT:        printf("format"); break;
        case OpCode::TYPEOF:        printf("typeof"); break;
        case OpCode::STORE:         printf("store"); break;
        case OpCode::ISTORE:        printf("istore"); break;
        case OpCode::LISTSET:       printf("listset"); break;
        case OpCode::SUBSCR:        printf("subscr"); break;
        case OpCode::SUBSCR_STORE:  printf("subscr_store"); break;
        case OpCode::STRINGSET:     printf("stringset"); break;
        case OpCode::TUPLESET:      printf("tupleset"); break;
        case OpCode::FUNCTIONSET:   printf("funcset"); break;
        case OpCode::LOAD:          printf("load"); break;
        case OpCode::RET:           printf("ret"); break;
        case OpCode::CALL:          printf("call"); break;
        case OpCode::CALLMethod:    printf("callmethod"); break;
        case OpCode::FNBEGIN:       printf("fnbegin"); break;
        case OpCode::FNEND:         printf("fnend"); break;
        case OpCode::END:           printf("end"); break;
        default: error("??????"); break;
    }
}

//VMcode push
void BytecodeGenerator::vcpush(OpCode t) {
    vmcodes.push_back(vmcode_t(t, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, int n) {
    vmcodes.push_back(vmcode_t(t, n, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, char c) {
    vmcodes.push_back(vmcode_t(t, c, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, const char *s) {
    vmcodes.push_back(vmcode_t(t, s, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, NodeVariable *v) {
    vmcodes.push_back(vmcode_t(t, v, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, char *s, unsigned int n) {
    vmcodes.push_back(vmcode_t(t, s, n, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, size_t ls) {
    vmcodes.push_back(vmcode_t(t, ls, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, Method m) {
    vmcodes.push_back(vmcode_t(t, m, nline++));
}

void BytecodeGenerator::vcpush(OpCode t, size_t fs, size_t fe) {
    vmcodes.push_back(vmcode_t(t, fs, fe, nline++));
}
