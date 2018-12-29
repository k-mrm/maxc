#include"maxc.h"

void Parser::run(Token token) {
    Ast ast;
    for(token_t tk: token.token_v) {
        if(tk.type == TOKEN_TYPE_NUM) {
            ast.node = ast.make_num_node(tk);
            std::cout << ast.node->value << std::endl;
        }
        else if(tk.type == TOKEN_TYPE_SYMBOL) {
            ;
        }
        else if(tk.type == TOKEN_TYPE_END) {
            break;
        }
    }
}

void Parser::eval() {
}
