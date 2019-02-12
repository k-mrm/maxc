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
            case ND_TYPE_NUM:
                emit_num(ast);
                break;
            case ND_TYPE_BINARY:
                emit_binop(ast);
                break;
            case ND_TYPE_UNARY:
                emit_unaop(ast);
                break;
            case ND_TYPE_ASSIGNMENT:
                emit_assign(ast);
                break;
            case ND_TYPE_IF:
                emit_if(ast);
                break;
            case ND_TYPE_EXPRIF:
                emit_exprif(ast);
                break;
            case ND_TYPE_FOR:
                emit_for(ast);
                break;
            case ND_TYPE_WHILE:
                emit_while(ast);
                break;
            case ND_TYPE_BLOCK:
                emit_block(ast);
                break;
            case ND_TYPE_RETURN:
                emit_return(ast);
                break;
            case ND_TYPE_VARIABLE:
                emit_variable(ast);
                break;
            case ND_TYPE_FUNCCALL:
                emit_func_call(ast);
                break;
            case ND_TYPE_FUNCDEF:
                emit_func_def(ast);
                break;
            case ND_TYPE_VARDECL:
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

void Program::emit_binop(Ast *ast) {
    Node_binop *b = (Node_binop *)ast;

    if(emit_log_andor(b))   return;

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

void Program::emit_addr(Ast *ast) {
    assert(ast->get_nd_type() == ND_TYPE_VARIABLE);
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
    //puts("\tpush %rax");

    std::string o = [&]() -> std::string {
        if(u->op == "++")   return "inc";
        if(u->op == "--")   return "dec";
        else                return "??????";
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
    for(regn = 0; regn < f->args.var_v.size(); regn++)
        printf("\tpush %%%s\n", regs[regn].c_str());
    /*
    for(auto l: f->args)
        vars.push_back(l.name);
    */

    int off = 0;
    for(Node_variable *a: f->lvars.var_v) {
        //printf("[debug] vinfo: %s\n", a->vinfo.name.c_str());
        off += 8;
        a->offset = off;
        //printf("[debug] %d\n", a->offset);
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

    if(v->init) {
        Node_variable *a = (Node_variable *)v->var;
        int off = a->offset;
        gen(v->init);
        printf("\tmov %%rax, -%d(%%rbp)\n", off);
    }

    /*
    for(auto a: v->decl_v) {
        vars.push_back(a.name);

        if(a.init) {
            int off = get_var_pos(a.name);
            gen(a.init);
            printf("\tmov %%rax, -%d(%%rbp)\n", off * 8);
        }
    }
    */
}

void Program::emit_variable(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int off = v->offset;

    printf("\tmov -%d(%%rbp), %%rax\n", off);
}

std::string Program::get_label() {
    std::string l = ".L";

    l += std::to_string(labelnum++);

    return l;
}
