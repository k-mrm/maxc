#ifndef MAXC_PARSER_H
#define MAXC_PARSER_H

#include "ast.h"
#include "maxc.h"
#include "token.h"

class Parser {
  public:
    Parser(Token &t) : token(t) {}
    Ast_v &run();

  private:
    Token &token;
    Ast_v program;
    bool is_func_call();
    Ast *read_lsmethod(Ast *);
    Ast *read_strmethod(Ast *);
    Ast *read_tuplemethod(Ast *);
    NodeVariable *read_member();

    Ast *var_decl(bool isconst);
    Type *eval_type();
    Type *skip_index(CTYPE);
    Ast *assignment();
    Ast *make_assign(Ast *dst, Ast *src);
    Ast *make_assigneq(std::string, Ast *, Ast *);
    Ast *make_return();
    Ast *make_if(bool);
    Ast *make_for();
    Ast *make_while();
    Ast *make_block();
    Ast *make_struct();
    void make_typedef();
    Ast *func_def();
    Ast *func_call();
    Ast *expr();
    Ast *expr_assign();
    Ast *expr_ternary();
    Ast *expr_logic_or();
    Ast *expr_logic_and();
    Ast *expr_equality();
    Ast *expr_comp();
    Ast *expr_add();
    Ast *expr_mul();
    Ast *expr_unary();
    Ast *expr_unary_postfix();
    Ast *expr_primary();
    Ast *expr_if();
    Ast *expr_bool();
    Ast *expr_num(token_t);
    Ast *expr_char(token_t);
    Ast *expr_string(token_t);
    Ast *expr_var(token_t);
    Ast_v &eval();
    Ast *statement();
    Ast *struct_init();

    void set_global();

    std::unordered_map<std::string, Type *> typemap;
};

#endif
