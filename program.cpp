#include "maxc.h"

std::string regs[] = {"rsi", "rdi", "rdx", "rcx", "r8", "r9"};

void Program::out(Ast_v asts) {
    emit_head();

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

void Program::emit_head() {
    puts(".intel_syntax noprefix");
    puts(".global main");
}

void Program::emit_num(Ast *ast) {
    Node_number *n = (Node_number *)ast;
    std::cout << "\tpush " << n->number << std::endl;
}

void Program::emit_binop(Ast *ast) {
    Node_binop *b = (Node_binop *)ast;

    gen(b->left);
    gen(b->right);

    puts("\tpop rdi");
    puts("\tpop rax");

    x86_ord = [&]() -> std::string {
        if(b->symbol == "+")
            return "add";
        if(b->symbol == "-")
            return "sub";
        if(b->symbol == "*")
            return "imul";
        if(b->symbol == "/") {
            puts("\tmov rdx, 0");
            puts("\tdiv rdi");
            return "null_op";
        }
        if(b->symbol == "%") {
            puts("\tmov rdx, 0");
            puts("\tdiv rdi");
            puts("\tmov rax, rdx");
            return "null_op";
        }

        error("??????? in emit_binop");
        return "null_op";
    }();

    if(x86_ord != "null_op")
        std::cout << "\t" << x86_ord << " rax, rdi" << std::endl;

    puts("\tpush rax");
}

void Program::emit_assign(Ast *ast) {
    Node_assignment *a = (Node_assignment *)ast;
    emit_assign_left(a->dst);
    gen(a->src);

    puts("\tpop rdi");
    puts("\tpop rax");
    puts("\tmov [rax], rdi");
    puts("\tpush rdi");
}

void Program::emit_assign_left(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    int p = get_var_pos(v->name);
    puts("\tmov rax, rbp");
    printf("\tsub rax, %d\n", p * 8);
    puts("\tpush rax");
}

void Program::emit_func_def(Ast *ast) {
    Node_func_def *f = (Node_func_def *)ast;

    std::cout << f->name << ":" << std::endl;

    emit_func_head();
    for(Ast *b: f->block) {
        gen(b);
    }

    emit_func_end();
}

void Program::emit_return(Ast *ast) {
    Node_return *r = (Node_return *)ast;
    gen(r->cont);

    puts("\tpop rax");
    puts("\tleave");
    puts("\tret");
}

void Program::emit_func_call(Ast *ast) {
    Node_func_call *f = (Node_func_call *)ast;
    std::cout << "\tcall " << f->name << std::endl;
    //TODO arg
}

void Program::emit_func_head() {
    puts("\tpush rbp");
    puts("\tmov rbp, rsp");
    puts("\tsub rsp, 256");
}

void Program::emit_func_end() {
    puts("\tleave");
    puts("\tmov eax, 0");
    puts("\tret");
}

void Program::emit_vardecl(Ast *ast) {
    Node_var_decl *v = (Node_var_decl *)ast;

    for(auto a: v->decl_v) {
        vars.push_back(a);
    }

    if(!isused_var) {
    } //TODO move to emit_func_head;

    isused_var = true;
}

void Program::emit_variable(Ast *ast) {
    emit_assign_left(ast);
    puts("\tpop rax");
    puts("\tmov rax, [rax]");
    puts("\tpush rax");
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
        case TYPE_INT:
            return 4;
        case TYPE_VOID:
            return 0;
    }
}
