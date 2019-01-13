#include "maxc.h"

Program::Program() {
    //puts("---generated code---");
    printf(".intel_syntax noprefix\n.global main\nmain:\n");
}

Program::~Program() {
    puts("\tpop rax");
    puts("\tret");
}

void Program::gen(Ast *ast) {
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case ND_TYPE_NUM: {
                Node_number *n = (Node_number *)ast;
                std::cout << "\tpush " << n->number << std::endl;

                return ;
            }
            case ND_TYPE_SYMBOL: {
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

                break;
            }
            default:
                error("??? in gen");
        }
    }
}

