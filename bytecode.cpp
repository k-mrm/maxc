#include "maxc.h"

void BytecodeGenerator::compile(Ast_v asts, Env e) {
    env = e;
    for(Ast *ast: asts)
        gen(ast);
    vcpush(OPCODE::END);
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
        vcpush(OPCODE::PUSHCONST_1);
    }
    else if(n->number == 2) {
        vcpush(OPCODE::PUSHCONST_2);
    }
    else if(n->number == 3) {
        vcpush(OPCODE::PUSHCONST_3);
    }
    else
        vcpush(OPCODE::IPUSH, n->number);
}

void BytecodeGenerator::emit_bool(Ast *ast) {
    auto b = (NodeBool *)ast;
    if(b->boolean == true)
        vcpush(OPCODE::PUSHTRUE);
    else if(b->boolean == false)
        vcpush(OPCODE::PUSHFALSE);
}

void BytecodeGenerator::emit_char(Ast *ast) {
    NodeChar *c = (NodeChar *)ast;
    vcpush(OPCODE::PUSH, (char)c->ch);
}

void BytecodeGenerator::emit_string(Ast *ast) {
    auto *s = (NodeString *)ast;
    vcpush(OPCODE::STRINGSET, s->string);
}

void BytecodeGenerator::emit_list(Ast *ast) {
    auto *l = (NodeList *)ast;
    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen(l->elem[i]);
    vcpush(OPCODE::LISTSET, l->nsize);
}

void BytecodeGenerator::emit_listaccess(Ast *ast) {
    auto l = (NodeSubscript *)ast;
    if(l->istuple) {
        gen(l->index);
        gen(l->ls);
        vcpush(OPCODE::CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index);
        gen(l->ls);
        vcpush(OPCODE::SUBSCR);
    }
}

void BytecodeGenerator::emit_tuple(Ast *ast) {
    auto t = (NodeTuple *)ast;
    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen(t->exprs[i]);
    vcpush(OPCODE::TUPLESET, t->nsize);
}

void BytecodeGenerator::emit_binop(Ast *ast) {
    NodeBinop *b = (NodeBinop *)ast;

    if(b->ctype->isobject()) {
        emit_object_oprator(b); return;
    }

    gen(b->left);
    gen(b->right);

    if(b->symbol == "+") {
        vcpush(OPCODE::ADD); return;
    }
    if(b->symbol == "-") {
        vcpush(OPCODE::SUB); return;
    }
    if(b->symbol == "*") {
        vcpush(OPCODE::MUL); return;
    }
    if(b->symbol == "/") {
        vcpush(OPCODE::DIV); return;
    }
    if(b->symbol == "%") {
        vcpush(OPCODE::MOD); return;
    }
    if(b->symbol == "==") {
        vcpush(OPCODE::EQ); return;
    }
    if(b->symbol == "!=") {
        vcpush(OPCODE::NOTEQ); return;
    }
    if(b->symbol == "||") {
        vcpush(OPCODE::LOGOR); return;
    }
    if(b->symbol == "&&") {
        vcpush(OPCODE::LOGAND); return;
    }
    if(b->symbol == "<") {
        vcpush(OPCODE::LT); return;
    }
    if(b->symbol == "<=") {
        vcpush(OPCODE::LTE); return;
    }
    if(b->symbol == ">") {
        vcpush(OPCODE::GT); return;
    }
    if(b->symbol == ">=") {
        vcpush(OPCODE::GTE); return;
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
        vcpush(OPCODE::CALLMethod, d->method);
    }
    else error("unimplemented");    //struct
}

void BytecodeGenerator::emit_ternop(Ast *ast) {
    auto t = (NodeTernop *)ast;

    gen(t->cond);
    char *l1 = get_label();
    vcpush(OPCODE::JMP_NOTEQ, l1);
    gen(t->then);

    char *l2 = get_label();
    vcpush(OPCODE::JMP, l2);
    lmap[l1] = nline;
    vcpush(OPCODE::LABEL, l1);
    gen(t->els);
    lmap[l2] = nline;
    vcpush(OPCODE::LABEL, l2);
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
        vcpush(OPCODE::INC);
    else if(u->op == "--")
        vcpush(OPCODE::DEC);
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
        vcpush(OPCODE::ISTORE, v);
    else
        vcpush(OPCODE::STORE, v); //TODO
}

void BytecodeGenerator::emit_listaccess_store(Ast *ast) {
    auto l = (NodeSubscript *)ast;
    gen(l->index);
    gen(l->ls);
    vcpush(OPCODE::SUBSCR_STORE);
}

void BytecodeGenerator::emit_func_def(Ast *ast) {
    auto f = (NodeFunction *)ast;

    const char *fname = f->finfo.name.c_str();
    vcpush(OPCODE::FNBEGIN, fname);
    fnpc.push(nline);

    int n;
    for(n = f->finfo.args.get().size() - 1; n >= 0; n--) {
        auto a = f->finfo.args.get()[n];
        switch(a->ctype->get().type) {
            case CTYPE::INT:
                vcpush(OPCODE::ISTORE, a); break;
            default: //TODO
                vcpush(OPCODE::STORE, a); break;
        }
    }

    for(Ast *b: f->block) gen(b);
    vcpush(OPCODE::RET);
    vcpush(OPCODE::FNEND, fname);

    /*
    lmap[f->name] = nline;
    vcpush(OPCODE::FNBEGIN, f->name);

    vcpush(OPCODE::FNEND, f->name);
    */

    vcpush(OPCODE::FUNCTIONSET, fnpc.top(), nline - 1);
    fnpc.pop();
    emit_store(f->fnvar);
}

void BytecodeGenerator::emit_if(Ast *ast) {
    auto i = (NodeIf *)ast;

    gen(i->cond);
    char *l1 = get_label();
    vcpush(OPCODE::JMP_NOTEQ, l1);
    gen(i->then_s);

    if(i->else_s) {
        char *l2 = get_label();
        vcpush(OPCODE::JMP, l2);
        lmap[l1] = nline;
        vcpush(OPCODE::LABEL, l1);
        gen(i->else_s);
        lmap[l2] = nline;
        vcpush(OPCODE::LABEL, l2);
    }
    else {
        lmap[l1] = nline;
        vcpush(OPCODE::LABEL, l1);
    }
}

void BytecodeGenerator::emit_for(Ast *ast) {
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OPCODE::LABEL, begin);
    if(f->cond) {
        gen(f->cond);
        vcpush(OPCODE::JMP_NOTEQ, end);
    }
    gen(f->body);
    if(f->reinit)
        gen(f->reinit);
    vcpush(OPCODE::JMP, begin);
    lmap[end] = nline;
    vcpush(OPCODE::LABEL, end);
}

void BytecodeGenerator::emit_while(Ast *ast) {
    auto w = (NodeWhile *)ast;
    char *begin = get_label();
    char *end = get_label();

    lmap[begin] = nline;
    vcpush(OPCODE::LABEL, begin);
    gen(w->cond);
    vcpush(OPCODE::JMP_NOTEQ, end);
    gen(w->body);
    vcpush(OPCODE::JMP, begin);
    lmap[end] = nline;
    vcpush(OPCODE::LABEL, end);
}

void BytecodeGenerator::emit_return(Ast *ast) {
    auto r = (NodeReturn *)ast;
    gen(r->cont);

    vcpush(OPCODE::RET);
}

void BytecodeGenerator::emit_print(Ast *ast) {
    auto p = (NodePrint *)ast;
    gen(p->cont);
    vcpush(OPCODE::PRINT);
}

void BytecodeGenerator::emit_println(Ast *ast) {
    auto p = (NodePrintln *)ast;
    gen(p->cont);
    vcpush(OPCODE::PRINTLN);
}

void BytecodeGenerator::emit_format(Ast *ast) {
    /*
    auto f = (NodeFormat *)ast;
    for(int n = f->args.size() - 1; n >= 0; --n)
        gen(f->args[n]);

    vcpush(OPCODE::FORMAT, f->cont, (unsigned int)f->narg);
    */
}

void BytecodeGenerator::emit_typeof(Ast *ast) {
    auto t = (NodeTypeof *)ast;
    gen(t->var);
    vcpush(OPCODE::TYPEOF);
}

void BytecodeGenerator::emit_func_call(Ast *ast) {
    auto f = (NodeFnCall *)ast;
    assert(f->func->get_nd_type() == NDTYPE::VARIABLE);
    for(auto a: f->args) gen(a);
    gen(f->func);
    vcpush(OPCODE::CALL);
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
                vcpush(OPCODE::ISTORE, a);
            else
                vcpush(OPCODE::STORE, a); //TODO
        }
        else {
            if(a->ctype->get().type == CTYPE::INT) {
                vcpush(OPCODE::IPUSH, 0);
                vcpush(OPCODE::ISTORE, a);
            }
            else {
                vcpush(OPCODE::PUSH, 0);
                vcpush(OPCODE::STORE, a); //TODO
            }
        }
        ++n;
    }
}

void BytecodeGenerator::emit_load(Ast *ast) {
    NodeVariable *v = (NodeVariable *)ast;
    vcpush(OPCODE::LOAD, v);
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
        //case OPCODE::PUSH:
        case OPCODE::IPUSH:
            printf(" %d", a.num); break;
        case OPCODE::STORE:
        case OPCODE::ISTORE:
        case OPCODE::LOAD:
            //printf(" %s(id:%d)", a.var->var->vinfo.name.c_str(), a.var->var->vid);
            if(a.var->ctype->isfunction())
                std::cout << " `" << a.var->finfo.name << "`(id:" << a.var->vid << ")";
            else
                std::cout << " `" << a.var->vinfo.name << "`(id:" << a.var->vid << ")";
            break;
        case OPCODE::LABEL:
        case OPCODE::JMP:
        case OPCODE::JMP_EQ:
        case OPCODE::JMP_NOTEQ:
        case OPCODE::FNBEGIN:
        case OPCODE::FNEND:
            printf(" %s(%d)", a.str, lmap[a.str]); break;

        case OPCODE::FORMAT:
            printf(" \"%s\", %d", a.str, a.nfarg); break;
        case OPCODE::LISTSET:
        case OPCODE::TUPLESET:
            printf(" (size: %d)", (int)a.size); break;
        case OPCODE::STRINGSET:
            printf(" %s", a.str); break;
        case OPCODE::FUNCTIONSET:
            printf(" %ld - %ld", a.fnstart, a.fnend); break;

        default:
            break;
    }
}

void BytecodeGenerator::opcode2str(OPCODE o) {
    switch(o) {
        case OPCODE::PUSH:          printf("push"); break;
        case OPCODE::IPUSH:         printf("ipush"); break;
        case OPCODE::PUSHCONST_1:   printf("pushconst1"); break;
        case OPCODE::PUSHCONST_2:   printf("pushconst2"); break;
        case OPCODE::PUSHCONST_3:   printf("pushconst3"); break;
        case OPCODE::PUSHTRUE:      printf("pushtrue"); break;
        case OPCODE::PUSHFALSE:     printf("pushfalse"); break;
        case OPCODE::POP:           printf("pop"); break;
        case OPCODE::ADD:           printf("add"); break;
        case OPCODE::SUB:           printf("sub"); break;
        case OPCODE::MUL:           printf("mul"); break;
        case OPCODE::DIV:           printf("div"); break;
        case OPCODE::MOD:           printf("mod"); break;
        case OPCODE::LOGOR:         printf("or");  break;
        case OPCODE::LOGAND:        printf("and"); break;
        case OPCODE::EQ:            printf("eq");  break;
        case OPCODE::NOTEQ:         printf("noteq"); break;
        case OPCODE::LT:            printf("lt"); break;
        case OPCODE::LTE:           printf("lte"); break;
        case OPCODE::GT:            printf("gt"); break;
        case OPCODE::GTE:           printf("gte"); break;
        case OPCODE::INC:           printf("inc"); break;
        case OPCODE::DEC:           printf("dec"); break;
        case OPCODE::LABEL:         printf("label"); break;
        case OPCODE::JMP:           printf("jmp"); break;
        case OPCODE::JMP_EQ:        printf("jmp_eq"); break;
        case OPCODE::JMP_NOTEQ:     printf("jmp_neq"); break;
        case OPCODE::PRINT:         printf("print"); break;
        case OPCODE::PRINTLN:       printf("println"); break;
        case OPCODE::FORMAT:        printf("format"); break;
        case OPCODE::TYPEOF:        printf("typeof"); break;
        case OPCODE::STORE:         printf("store"); break;
        case OPCODE::ISTORE:        printf("istore"); break;
        case OPCODE::LISTSET:       printf("listset"); break;
        case OPCODE::SUBSCR:        printf("subscr"); break;
        case OPCODE::SUBSCR_STORE:  printf("subscr_store"); break;
        case OPCODE::STRINGSET:     printf("stringset"); break;
        case OPCODE::TUPLESET:      printf("tupleset"); break;
        case OPCODE::FUNCTIONSET:   printf("funcset"); break;
        case OPCODE::LOAD:          printf("load"); break;
        case OPCODE::RET:           printf("ret"); break;
        case OPCODE::CALL:          printf("call"); break;
        case OPCODE::CALLMethod:    printf("callmethod"); break;
        case OPCODE::FNBEGIN:       printf("fnbegin"); break;
        case OPCODE::FNEND:         printf("fnend"); break;
        case OPCODE::END:           printf("end"); break;
        default: error("??????"); break;
    }
}

//VMcode push
void BytecodeGenerator::vcpush(OPCODE t) {
    vmcodes.push_back(vmcode_t(t, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, int n) {
    vmcodes.push_back(vmcode_t(t, n, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, char c) {
    vmcodes.push_back(vmcode_t(t, c, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, const char *s) {
    vmcodes.push_back(vmcode_t(t, s, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, NodeVariable *v) {
    vmcodes.push_back(vmcode_t(t, v, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, char *s, unsigned int n) {
    vmcodes.push_back(vmcode_t(t, s, n, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, size_t ls) {
    vmcodes.push_back(vmcode_t(t, ls, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, Method m) {
    vmcodes.push_back(vmcode_t(t, m, nline++));
}

void BytecodeGenerator::vcpush(OPCODE t, size_t fs, size_t fe) {
    vmcodes.push_back(vmcode_t(t, fs, fe, nline++));
}
