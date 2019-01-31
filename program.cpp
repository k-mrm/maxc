#include "maxc.h"

std::string regs[] = {"rsi", "rdi", "rdx", "rcx", "r8", "r9"};

void Program::out(Ast_v asts) {
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
            case ND_TYPE_SYMBOL:
                emit_binop(ast);
                break;
            case ND_TYPE_ASSIGNMENT:
                emit_assign(ast);
                break;
            case ND_TYPE_IF:
                emit_if(ast);
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
            default:
                error("??? in gen");
        }
    }
}

void Program::emit_num(Ast *ast) {
    Node_number *n = (Node_number *)ast;
    std::cout << "\tmov $" << n->number << ", %rax" << std::endl;
}

void Program::emit_binop(Ast *ast) {
    Node_binop *b = (Node_binop *)ast;

    gen(b->left);
    puts("\tpush %rax");
    gen(b->right);

    /*
    puts("\tpop %rdi");
    puts("\tpop %rax");
    */

    x86_ord = [&]() -> std::string {
        if(b->symbol == "+")    return "add";
        if(b->symbol == "-")    return "sub";
        if(b->symbol == "*")    return "imul";
        if(b->symbol == "/") {
            puts("\tmov %rax, %rdi");
            puts("\tpop %rax");
            puts("\tmov $0, %rdx");
            puts("\tidiv %rdi");
            return "null_op";
        }
        if(b->symbol == "%") {
            puts("\tmov $0, %rdx");
            puts("\tidiv %rdi");
            puts("\tmov %rdx, %rax");
            return "null_op";
        }
        if(b->symbol == "==") {
            emit_cmp("sete", b);
            return "null_op";
        }
        if(b->symbol == "!=") {
            emit_cmp("setne", b);
            return "null_op";
        }

        error("??????? in emit_binop");
        return "null_op";
    }();

    if(x86_ord != "null_op") {
        puts("\tpop %rdi");
        std::cout << "\t" << x86_ord << " %rdi, %rax" << std::endl;
    }

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
    emit_assign_left(a->dst);
}

void Program::emit_assign_left(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int off = get_var_pos(v->name);

    printf("\tmov %%rax, -%d(%%rbp)\n", off * 8);
}

void Program::emit_func_def(Ast *ast) {
    Node_func_def *f = (Node_func_def *)ast;

    emit_func_head(f);

    for(Ast *b: f->block) {
        gen(b);
    }

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

    puts("\tleave");
    puts("\tret");
}

void Program::emit_func_call(Ast *ast) {
    Node_func_call *f = (Node_func_call *)ast;
    puts("\tmov $0, %rax");
    std::cout << "\tcall " << f->name << std::endl;
    //TODO arg
}

void Program::emit_func_head(Node_func_def *f) {
    puts("\t.text");
    printf("\t.global %s\n", f->name.c_str());
    printf("%s:\n", f->name.c_str());
    //TODO arg
    puts("\tpush %rbp");
    puts("\tmov %rsp, %rbp");
    puts("\tsub $256, %rsp");
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
    Node_var_decl *v = (Node_var_decl *)ast;

    for(auto a: v->decl_v) {
        vars.push_back(a);
    }
}

void Program::emit_variable(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int off = get_var_pos(v->name);

    printf("\tmov -%d(%%rbp), %%rax\n", off * 8);
}

int Program::get_var_pos(std::string name) {
    int cnt = 1;
    for(var_t v: vars) {
        if(v.name == name)
            return cnt;
        cnt++;
    }

    error("not variable");
    return 0;
}

int Program::get_type_size(var_type ty) {
    switch(ty) {
        case TYPE_INT:  return 4;
        case TYPE_VOID: return 0;
    }
}

std::string Program::get_label() {
    std::string l = ".L";

    l += std::to_string(labelnum++);

    return l;
}
