#include "maxc.h"

std::string regs[] = {"rsi", "rdi", "rdx", "rcx", "r8", "r9"};

void Program::generate(Ast_v asts, Env e) {
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
    printf("\tmov $%d, %%rax\n", n->number);
}

void Program::emit_char(Ast *ast) {
    Node_char *c = (Node_char *)ast;
    printf("\tmov $%d, %%rax\n", c->ch);
}

void Program::emit_binop(Ast *ast) {
    Node_binop *b = (Node_binop *)ast;
    //Node_variable *v = (Node_variable *)b->left;

    if(emit_log_andor(b))   return;
    if(b->left->ctype->get().type == CTYPE::PTR) { emit_pointer(b); return; }

    gen(b->left);
    puts("\tpush %rax");
    gen(b->right);
    puts("\tmov %rax, %rdi");
    puts("\tpop %rax");

    x86_ord = [&]() -> std::string {
        if(b->symbol == "+")    return "add";
        if(b->symbol == "-")    return "sub";
        if(b->symbol == "*")    return "imul";
        if(b->symbol == "/") {
            puts("\tmov $0, %rdx");
            puts("\tidiv %rdi");
            return "";
        }
        if(b->symbol == "%") {
            puts("\tmov $0, %rdx");
            puts("\tidiv %rdi");
            puts("\tmov %rdx, %rax");
            return "";
        }
        if(b->symbol == "==") {
            emit_cmp("sete", b);
            return "";
        }
        if(b->symbol == "!=") {
            emit_cmp("setne", b);
            return "";
        }
        if(b->symbol == "<") {
            emit_cmp("setl", b);
            return "";
        }
        if(b->symbol == ">") {
            emit_cmp("setg", b);
            return "";
        }
        if(b->symbol == "<=") {
            emit_cmp("setle", b);
            return "";
        }
        if(b->symbol == ">=") {
            emit_cmp("setge", b);
            return "";
        }

        error("??????? in emit_binop");
        return "";
    }();

    if(x86_ord != "") {
        //puts("\tpop %rdi");
        printf("\t%s %%rdi, %%rax\n", x86_ord.c_str());
    }
}

bool Program::emit_log_andor(Node_binop *b) {
    if(b->symbol == "&&") {
        std::string end = get_label();
        gen(b->left);
        puts("\ttest %rax, %rax");
        puts("\tmov $0, %rax");
        printf("\tje %s\n", end.c_str());
        gen(b->right);
        puts("\ttest %rax, %rax");
        puts("\tmov $0, %rax");
        printf("\tje %s\n", end.c_str());
        puts("\tmov $1, %rax");
        printf("%s:\n", end.c_str());
        return true;
    }
    if(b->symbol == "||") {
        std::string end = get_label();
        gen(b->left);
        puts("\ttest %rax, %rax");
        puts("\tmov $1, %rax");
        printf("\tjne %s\n", end.c_str());
        gen(b->right);
        puts("\ttest %rax, %rax");
        puts("\tmov $1, %rax");
        printf("\tjne %s\n", end.c_str());
        puts("\tmov $0, %rax");
        printf("%s:\n", end.c_str());
        return true;
    }
    return false;
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
    assert(ast->get_nd_type() == NDTYPE::VARIABLE);
    Node_variable *v = (Node_variable *)ast;
    int off = v->offset;
    printf("\tlea %d(%%rbp), %%rax\n", -off);
}

void Program::emit_unaop(Ast *ast) {
    Node_unaop *u = (Node_unaop *)ast;
    gen(u->expr);

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

    std::string o = [&]() -> std::string {
        if(u->op == "++")   return "inc";
        if(u->op == "--")   return "dec";
        else {
            error("internal error: %s", u->op.c_str());
            return "";
        }
    }();
    printf("\t%s %%rax\n", o.c_str());
    emit_store(u->expr);
}

void Program::emit_cmp(std::string ord, Node_binop *a) {
    gen(a->left);
    puts("\tpush %rax");
    gen(a->right);
    puts("\tpop %rdi");
    puts("\tcmp %rax, %rdi");
    printf("\t%s %%al\n", ord.c_str());
    puts("\tmovzb %al, %rax");
}

void Program::emit_assign(Ast *ast) {
    Node_assignment *a = (Node_assignment *)ast;

    gen(a->src);
    emit_store(a->dst);
}

void Program::emit_store(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int off = v->offset;

    printf("\tmov %%rax, -%d(%%rbp)\n", off);
}

void Program::emit_func_def(Ast *ast) {
    Node_func_def *f = (Node_func_def *)ast;

    emit_func_head(f);

    for(Ast *b: f->block) gen(b);

    emit_func_end();
}

void Program::emit_if(Ast *ast) {
    Node_if *i = (Node_if *)ast;

    gen(i->cond);
    puts("\ttest %rax, %rax");
    std::string l1 = get_label();
    printf("\tje %s\n", l1.c_str());
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
}

void Program::emit_exprif(Ast *ast) {
    isexpr = true;

    Node_exprif *i = (Node_exprif *)ast;
    gen(i->cond);
    puts("\ttest %rax, %rax");
    std::string l1 = get_label();
    endlabel = get_label();
    printf("\tje %s\n", l1.c_str());
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
    printf("%s:\n", begin.c_str());
    if(f->cond) {
        gen(f->cond);
        puts("\ttest %rax, %rax");
        printf("\tje %s\n", end.c_str());
    }
    gen(f->body);
    if(f->reinit)
        gen(f->reinit);
    printf("\tjmp %s\n", begin.c_str());
    printf("%s:\n", end.c_str());
}

void Program::emit_while(Ast *ast) {
    Node_while *w = (Node_while *)ast;
    std::string begin = get_label();
    std::string end = get_label();

    printf("%s:\n", begin.c_str());
    gen(w->cond);
    puts("\ttest %rax, %rax");
    printf("\tje %s\n", end.c_str());
    gen(w->body);
    printf("\tjmp %s\n", begin.c_str());
    printf("%s:\n", end.c_str());
}

void Program::emit_return(Ast *ast) {
    Node_return *r = (Node_return *)ast;
    gen(r->cont);

    if(!isexpr) {
        puts("\tleave");
        puts("\tret");
    }
    else {
        printf("\tjmp %s\n", endlabel.c_str());
    }
}

void Program::emit_func_call(Ast *ast) {
    Node_func_call *f = (Node_func_call *)ast;
    int regn;
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
    //TODO arg
}

void Program::emit_func_head(Node_func_def *f) {
    puts("\t.text");
    printf("\t.global %s\n", f->name.c_str());
    printf("%s:\n", f->name.c_str());
    //TODO arg
    puts("\tpush %rbp");
    puts("\tmov %rsp, %rbp");

    int regn;
    for(regn = 0; regn < f->args.get().size(); regn++)
        printf("\tpush %%%s\n", regs[regn].c_str());

    int off = 0;
    for(Node_variable *a: f->lvars.get()) {
        debug("vinfo: %s\n", a->vinfo.name.c_str());
        off += align(a->vinfo.type->get_size(), 8);
        a->offset = off;
        debug("%d\n", a->offset);
    }
    if(off != 0)
        printf("\tsub $%d, %%rsp\n", off);
}

void Program::emit_func_end() {
    puts("\tleave");
    puts("\tret");
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
            int off = a->offset;
            gen(v->init[n]);
            printf("\tmov %%rax, %d(%%rbp)\n", -off);
        }
        n++;
    }
}

void Program::emit_variable(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int off = v->offset;

    printf("\tmov %d(%%rbp), %%rax\n", -off);
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
