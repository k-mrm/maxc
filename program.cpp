#include "maxc.h"

Program::~Program() {
    puts("\tpop rax");
    puts("\tret");
}

void Program::out(Ast_v asts) {
    emit_head();

    puts("\tpush rbp");
    puts("\tmov rbp, rsp");
    puts("\tsub rsp, 208");

    for(Ast *ast: asts) {
        gen(ast);
    }
}

void Program::gen(Ast *ast) {
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case ND_TYPE_NUM: {
                emit_num(ast);
                break;
            }
            case ND_TYPE_SYMBOL: {
                emit_binop(ast);
                break;
            }
            case ND_TYPE_ASSIGNMENT: {
                emit_assign(ast);
                break;
            }
            default:
                error("??? in gen");
        }
    }
}

void Program::emit_head() {
    puts(".intel_syntax noprefix");
    puts(".global main");
    puts("main:");
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

        error("???????");
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
    std::cout << "\tmov DWORD PTR [rbp-" << 4 << "], eax" << std::endl;
}

void Program::emit_assign_left(Ast *ast) {
}

/*/
void Program::emit_variable(Ast *ast) {
    Node_variable *v = (Node_variable *)ast;
    //TODO ganbaru
    //TODO var_decl
}
*/
