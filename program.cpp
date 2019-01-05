#include "maxc.h"

Program::Program() {
    //puts("---generated code---");
    printf(".intel_syntax noprefix\n.global main\nmain:\n");
}

Program::~Program() {
    puts("\tpop rax");
    puts("\tret");
}

void Program::gen(ast_t *ast) {
    if(ast->type == ND_TYPE_NUM) {
        std::cout << "\tpush " << ast->value << std::endl;

        return ;
    }

    gen(ast->left);
    gen(ast->right);

    puts("\tpop rdi");
    puts("\tpop rax");

    x86_ord = [&]() -> std::string {
        switch(ast->type) {
            case ND_TYPE_ADD:
                return "add";
            case ND_TYPE_SUB:
                return "sub";
            case ND_TYPE_MUL:
                return "imul";
            case ND_TYPE_DIV:
                puts("\tmov rdx, 0");
                puts("\tdiv rdi");
                return "null_op";
                //sorry coming soon, I want to sleep
            default:
                printf("???");
                exit(1);
        }
    }();

    if(x86_ord != "null_op")
        std::cout << "\t" << x86_ord << " rax, rdi" << std::endl;

    puts("\tpush rax");
}
