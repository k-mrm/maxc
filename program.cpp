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
            case ND_TYPE_PLUS:
                return "add";
            case ND_TYPE_MINUS:
                return "sub";
            default:
                printf("???");
                exit(1);
        }
    }();

    std::cout << "\t" << x86_ord << " rax, rdi" << std::endl;

    puts("\tpush rax");
}
