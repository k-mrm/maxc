#include "maxc.h"

void Program::compile(Ast_v asts, Env e) {
    env = e;
    for(Ast *ast: asts) {
        gen(ast);
    }
}

void Program::gen(Ast *ast) {
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case NDTYPE::NUM:
                emit_num(ast);
                break;
            case NDTYPE::CHAR:
                emit_char(ast);
                break;
            case NDTYPE::STRING:
                emit_string(ast);
                break;
            case NDTYPE::BINARY:
                emit_binop(ast);
                break;
            case NDTYPE::UNARY:
                emit_unaop(ast);
                break;
            case NDTYPE::ASSIGNMENT:
                emit_assign(ast);
                break;
            case NDTYPE::IF:
                emit_if(ast);
                break;
            case NDTYPE::EXPRIF:
                emit_exprif(ast);
                break;
            case NDTYPE::FOR:
                emit_for(ast);
                break;
            case NDTYPE::WHILE:
                emit_while(ast);
                break;
            case NDTYPE::BLOCK:
                emit_block(ast);
                break;
            case NDTYPE::PRINT:
                emit_print(ast);
                break;
            case NDTYPE::PRINTLN:
                emit_println(ast);
                break;
            case NDTYPE::TYPEOF:
                emit_typeof(ast);
                break;
            case NDTYPE::RETURN:
                emit_return(ast);
                break;
            case NDTYPE::VARIABLE:
                emit_variable(ast);
                break;
            case NDTYPE::FUNCCALL:
                emit_func_call(ast);
                break;
            case NDTYPE::FUNCDEF:
                emit_func_def(ast);
                break;
            case NDTYPE::VARDECL:
                emit_vardecl(ast);
                break;
            default:    error("??? in gen");
        }
    }
}

void Program::emit_num(Ast *ast) {
    Node_number *n = (Node_number *)ast;
    vcpush(OPCODE::PUSH, n->number);
}

void Program::emit_char(Ast *ast) {
    Node_char *c = (Node_char *)ast;
    vcpush(OPCODE::PUSH, (char)c->ch);
}

void Program::emit_string(Ast *ast) {
    auto *s = (Node_string *)ast;
    vcpush(OPCODE::PUSH, s->string);
}

void Program::emit_binop(Ast *ast) {
    Node_binop *b = (Node_binop *)ast;
    Node_variable *v = (Node_variable *)b->left;

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

    /*
    if(emit_log_andor(b))   return;
    if(v->vinfo.type->get().type == CTYPE::PTR) { emit_pointer(b); return; }
    //TODO type checking in parser.cpp
    */
}

void Program::emit_pointer(Node_binop *b) {
    gen(b->left);
    puts("\tpush %rax");
    gen(b->right);
    Node_variable *v = (Node_variable *)b->left;
    int size = v->vinfo.type->get_size();

    if(size > 1)
        printf("\timul $%d, %%rax\n", size);
    puts("\tmov %rax, %rdi");
    puts("\tpop %rax");
    puts("\tadd %rdi, %rax");
}

void Program::emit_addr(Ast *ast) {
    /*
    assert(ast->get_nd_type() == NDTYPE::VARIABLE);
    Node_variable *v = (Node_variable *)ast;
    int off = v->offset;
    printf("\tlea %d(%%rbp), %%rax\n", -off);
    */
}

void Program::emit_unaop(Ast *ast) {
    Node_unaop *u = (Node_unaop *)ast;
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
    if(u->op == "*") {
        //Node_variable *v = (Node_variable *)u->expr;
        assert(u->expr->ctype->get().type == CTYPE::PTR);
        std::string reg;
        reg = [&]() -> std::string {
            switch(u->expr->ctype->get_size()) {
                case 1: return "%dil";
                case 4: return "%edi";
                case 8: return "%rdi";
                default: error("? %d", u->expr->ctype->get_size()); return "";
            }
        }();
        puts("\tmov $0, %edi");
        printf("\tmov (%%rax), %s\n", reg.c_str());
        puts("\tmov %rdi, %rax");
        return;
    }
    //puts("\tpush %rax");
    */
    if(u->op == "++")
        vcpush(OPCODE::INC);
    else if(u->op == "--")
        vcpush(OPCODE::DEC);
    emit_store(u->expr);
}

void Program::emit_assign(Ast *ast) {
    debug("called assign\n");
    Node_assignment *a = (Node_assignment *)ast;

    gen(a->src);
    emit_store(a->dst);
}

void Program::emit_store(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;

    vcpush(OPCODE::STORE, v);
    //int off = v->offset;

    //printf("\tmov %%rax, -%d(%%rbp)\n", off);
}

void Program::emit_func_def(Ast *ast) {
    Node_func_def *f = (Node_func_def *)ast;

    lmap[f->name] = nline;
    vcpush(OPCODE::FNBEGIN, f->name);
    emit_func_head(f);

    for(Ast *b: f->block) gen(b);

    emit_func_end();

    vcpush(OPCODE::FNEND, f->name);
}

void Program::emit_if(Ast *ast) {
    Node_if *i = (Node_if *)ast;

    gen(i->cond);
    std::string l1 = get_label();
    vcpush(OPCODE::JMP_NOTEQ, l1);
    gen(i->then_s);

    if(i->else_s) {
        std::string l2 = get_label();
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

void Program::emit_exprif(Ast *ast) {
    isexpr = true;
    debug("exprif call\n");

    Node_exprif *i = (Node_exprif *)ast;
    gen(i->cond);
    puts("\ttest %rax, %rax");
    std::string l1 = get_label();
    endlabel = get_label();
    printf("\tje %s\n", l1.c_str());
    debug("kokokokok\n");
    gen(i->then_s);

    if(i->else_s) {
        std::string l2 = get_label();
        printf("\tjmp %s\n", l2.c_str());
        printf("%s:\n", l1.c_str());
        gen(i->else_s);
        printf("%s:\n", l2.c_str());
    }
    else
        printf("%s:\n", l1.c_str());
    printf("%s:\n", endlabel.c_str());

    isexpr = false;
}

void Program::emit_for(Ast *ast) {
    Node_for *f = (Node_for *)ast;

    if(f->init)
        gen(f->init);
    std::string begin = get_label();
    std::string end = get_label();
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

void Program::emit_while(Ast *ast) {
    Node_while *w = (Node_while *)ast;
    std::string begin = get_label();
    std::string end = get_label();

    lmap[begin] = nline;
    vcpush(OPCODE::LABEL, begin);
    gen(w->cond);
    vcpush(OPCODE::JMP_NOTEQ, end);
    gen(w->body);
    vcpush(OPCODE::JMP, begin);
    lmap[end] = nline;
    vcpush(OPCODE::LABEL, end);
}

void Program::emit_return(Ast *ast) {
    Node_return *r = (Node_return *)ast;
    gen(r->cont);

    vcpush(OPCODE::RET);
    /*
    if(!isexpr) {
        puts("\tleave");
        puts("\tret");
    }
    else {
        printf("\tjmp %s\n", endlabel.c_str());
    }
    */
}

void Program::emit_print(Ast *ast) {
    Node_print *p = (Node_print *)ast;
    gen(p->cont);
    vcpush(OPCODE::PRINT);
}

void Program::emit_println(Ast *ast) {
    Node_print *p = (Node_print *)ast;
    gen(p->cont);
    vcpush(OPCODE::PRINTLN);
}

void Program::emit_typeof(Ast *ast) {
    Node_typeof *t = (Node_typeof *)ast;
    gen(t->var);
    vcpush(OPCODE::TYPEOF);
}

void Program::emit_func_call(Ast *ast) {
    Node_func_call *f = (Node_func_call *)ast;

    for(auto a: f->arg_v)
        gen(a);
    vcpush(OPCODE::CALL, f->name);
    /*
    for(regn = 0; regn < f->arg_v.size(); regn++)
        printf("\tpush %%%s\n", regs[regn].c_str());
    for(Ast *a: f->arg_v) {
        gen(a);
        puts("\tpush %rax");
    }

    for(regn = f->arg_v.size() - 1; regn >= 0; regn--)
        printf("\tpop %%%s\n", regs[regn].c_str());

    std::cout << "\tcall " << f->name << std::endl;
    for(regn = f->arg_v.size() - 1; regn > 0; regn--)
        printf("\tpop %%%s\n", regs[regn].c_str());
    */
    //TODO arg
}

void Program::emit_func_head(Node_func_def *f) {
    //TODO func-start VM code

    int n;
    for(n = f->args.get().size() - 1; n >= 0; n--)
        vcpush(OPCODE::STORE, f->args.get()[n]);

    for(Node_variable *a: f->lvars.get()) {
        debug("vinfo: %s\n", a->vinfo.name.c_str());
        //a->id = off++;
        debug("id is %d\n", a->vid);
    }
}

void Program::emit_func_end() {
    vcpush(OPCODE::RET);
}

void Program::emit_block(Ast *ast) {
    Node_block *b = (Node_block *)ast;

    for(Ast *a: b->cont)
        gen(a);
}

void Program::emit_vardecl(Ast *ast) {
    Node_vardecl *v = (Node_vardecl *)ast;
    int n = 0;

    for(Node_variable *a: v->var.get()) {
        if(v->init[n] != nullptr) {
            //printf("#[debug]: offset is %d\n", a->offset);
            gen(v->init[n]);
            vcpush(OPCODE::STORE, a);
        }
        else {
            vcpush(OPCODE::PUSH, 0);
            vcpush(OPCODE::STORE, a);
        }
        n++;
    }
}

void Program::emit_variable(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    vcpush(OPCODE::LOAD, v);
    //int off = v->offset;

    //printf("\tmov %d(%%rbp), %%rax\n", -off);
}

std::string Program::get_label() {
    std::string l = ".L";
    l += std::to_string(labelnum++);

    return l;
}

int Program::align(int n, int base) {
    int r = n % base;
    return (r == 0) ? n : n - r + base;
}

void Program::show() {
    for(auto a: vmcodes) {
        printf("%04x ", a.nline);
        opcode2str(a.type);
        switch(a.type) {
            case OPCODE::PUSH:
                if(a.vtype == VALUE::INT) {
                    printf(" %d", a.value); break;
                }
                else if(a.vtype == VALUE::CHAR) {
                    printf(" %c", a.ch); break;
                }
                else if(a.vtype == VALUE::STRING) {
                    printf(" \"%s\"", a.str.c_str()); break;
                }
                else
                    break;
            case OPCODE::STORE:
            case OPCODE::LOAD:
                //printf(" %s(id:%d)", a.var->var->vinfo.name.c_str(), a.var->var->vid);
                std::cout << " `" << a.var->var->vinfo.name << "`(id:" << a.var->var->vid << ")";
                break;
            case OPCODE::LABEL:
            case OPCODE::JMP:
            case OPCODE::JMP_EQ:
            case OPCODE::JMP_NOTEQ:
            case OPCODE::CALL:
            case OPCODE::FNBEGIN:
            case OPCODE::FNEND:
                printf(" %s(%x)", a.str.c_str(), lmap[a.str]);

            default:
                break;
        }
        puts("");
    }
}

void Program::opcode2str(OPCODE o) {
    switch(o) {
        case OPCODE::PUSH:      printf("push"); break;
        case OPCODE::POP:       printf("pop"); break;
        case OPCODE::ADD:       printf("add"); break;
        case OPCODE::SUB:       printf("sub"); break;
        case OPCODE::MUL:       printf("mul"); break;
        case OPCODE::DIV:       printf("div"); break;
        case OPCODE::MOD:       printf("mod"); break;
        case OPCODE::LOGOR:     printf("or");  break;
        case OPCODE::LOGAND:    printf("and"); break;
        case OPCODE::EQ:        printf("eq");  break;
        case OPCODE::NOTEQ:     printf("noteq"); break;
        case OPCODE::LT:        printf("lt"); break;
        case OPCODE::LTE:       printf("lte"); break;
        case OPCODE::GT:        printf("gt"); break;
        case OPCODE::GTE:       printf("gte"); break;
        case OPCODE::INC:       printf("inc"); break;
        case OPCODE::DEC:       printf("dec"); break;
        case OPCODE::LABEL:     printf("label"); break;
        case OPCODE::JMP:       printf("jmp"); break;
        case OPCODE::JMP_EQ:    printf("jmp_eq"); break;
        case OPCODE::JMP_NOTEQ: printf("jmp_neq"); break;
        case OPCODE::PRINT:     printf("print"); break;
        case OPCODE::PRINTLN:   printf("println"); break;
        case OPCODE::TYPEOF:    printf("typeof"); break;
        case OPCODE::STORE:     printf("store"); break;
        case OPCODE::LOAD:      printf("load"); break;
        case OPCODE::RET:       printf("ret"); break;
        case OPCODE::CALL:      printf("call"); break;
        case OPCODE::FNBEGIN:   printf("fnbegin"); break;
        case OPCODE::FNEND:     printf("fnend"); break;
        default: error("??????"); break;
    }
}

//VMcode push
void Program::vcpush(OPCODE t) {
    vmcodes.push_back(vmcode_t(t, nline++));
}

void Program::vcpush(OPCODE t, int n) {
    vmcodes.push_back(vmcode_t(t, n, nline++));
}

void Program::vcpush(OPCODE t, char c) {
    vmcodes.push_back(vmcode_t(t, c, nline++));
}

void Program::vcpush(OPCODE t, std::string s) {
    vmcodes.push_back(vmcode_t(t, s, nline++));
}

void Program::vcpush(OPCODE t, Node_variable *v) {
    vmcodes.push_back(vmcode_t(t, v, nline++));
}
