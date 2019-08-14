#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "ast.h"
#include "env.h"
#include "literalpool.h"

class BytecodeGenerator {
  public:
    BytecodeGenerator(LiteralPool &l) : ltable(l) {}

    void compile(Ast_v &, bytecode &);
    void gen(Ast *, bytecode &, bool);
    void show(uint8_t[], size_t &);

  private:
    LiteralPool &ltable;

    void emit_head();
    void emit_num(Ast *, bytecode &, bool);
    void emit_bool(Ast *, bytecode &, bool);
    void emit_char(Ast *, bytecode &, bool);
    void emit_string(Ast *, bytecode &, bool);
    void emit_list(Ast *, bytecode &);
    void emit_listaccess(Ast *, bytecode &);
    void emit_tuple(Ast *, bytecode &);
    void emit_binop(Ast *, bytecode &, bool);
    void emit_member(Ast *, bytecode &, bool);
    void emit_ternop(Ast *, bytecode &);
    void emit_unaop(Ast *, bytecode &, bool);
    void emit_if(Ast *, bytecode &);
    void emit_for(Ast *, bytecode &);
    void emit_while(Ast *, bytecode &);
    void emit_return(Ast *, bytecode &);
    void emit_block(Ast *, bytecode &);
    void emit_print(Ast *, bytecode &);
    void emit_println(Ast *, bytecode &);
    void emit_format(Ast *, bytecode &);
    void emit_typeof(Ast *, bytecode &);
    void emit_assign(Ast *, bytecode &);
    void emit_struct_init(Ast *, bytecode &, bool);
    void emit_store(Ast *, bytecode &);
    void emit_member_store(Ast *, bytecode &);
    void emit_listaccess_store(Ast *, bytecode &);
    void emit_func_def(Ast *, bytecode &);
    void emit_func_call(Ast *, bytecode &, bool);
    void emit_bltinfunc_call(NodeFnCall *, bytecode &, bool);
    void emit_bltinfncall_print(NodeFnCall *, bytecode &, bool);
    void emit_bltinfncall_println(NodeFnCall *, bytecode &, bool);
    void emit_func_head(NodeFunction *);
    void emit_func_end();
    void emit_vardecl(Ast *, bytecode &);
    void emit_load(Ast *, bytecode &, bool);

    std::stack<size_t> fnpc;
};

#endif
