#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/*
 *  main
 */

class Maxc {
    public:
        int run(std::string src);

        void show_usage();
    private:
};

/*
 *  token
 */

enum Token_type {
    TOKEN_TYPE_END,
    TOKEN_TYPE_NUM,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_IDENTIFER
};

typedef struct {
    Token_type type;
    std::string value;
    int line;
} token_t;

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string value, int line);
        void push_symbol(std::string value, int line);
        void push_ident(std::string value, int line);
        void push_string(std::string value, int line);
        void push_end();

        void show();

        token_t get();
        token_t get_step();
        token_t see(int p);
        bool is_value(std::string tk);
        bool is_type(Token_type ty);
        bool is_type();
        bool skip(std::string val);
        bool abs_skip(std::string val);
        void step();
        bool step_to(std::string val);

        void save();
        void rewind();

    private:
        int save_point;
        bool issaved;
        int pos = 0;
};

/*
 *  lexer
 */

class Lexer {
    public:
        Token run(std::string src);
};

/*
 *  ctype
 */

enum c_type {
    TYPE_INT,
    TYPE_VOID,
};

struct type_t {
    c_type type;
    int size;   //array size

    type_t() {}
    type_t(c_type ty): type(ty){}
};

class Type {
    private:
        type_t type;
    public:
        Type *ptr;

        Type() {}
        Type(c_type ty): type(ty) {}
        Type(Type *t): type(t->type), ptr(t->ptr) {}

        std::string show();
        int get_size();
};

typedef std::vector<Type *> Type_v;

/*
 *  AST, parser
 */

enum nd_type {
    ND_TYPE_NUM = 100,
    ND_TYPE_SYMBOL,
    ND_TYPE_IDENT,
    ND_TYPE_RETURN,
    ND_TYPE_FUNCDEF,
    ND_TYPE_FUNCCALL,
    ND_TYPE_FUNCPROTO,
    ND_TYPE_VARDECL,
    ND_TYPE_ASSIGNMENT,
    ND_TYPE_VARIABLE,
    ND_TYPE_BLOCK,
    ND_TYPE_STRING,
    ND_TYPE_BINARY,
    ND_TYPE_UNARY,
    ND_TYPE_IF,
    ND_TYPE_FOR,
    ND_TYPE_WHILE,
};

class Ast {
    public:
        virtual nd_type get_nd_type() = 0;
};

typedef std::vector<Ast *> Ast_v;

class Node_number: public Ast {
    public:
        int number;
        virtual nd_type get_nd_type() { return ND_TYPE_NUM; }

        Node_number(int _n): number(_n){}
};

class Node_binop: public Ast {
    public:
        std::string symbol;
        Ast *left;
        Ast *right;
        virtual nd_type get_nd_type() { return ND_TYPE_BINARY; }

        Node_binop(std::string _s, Ast *_l, Ast *_r):
            symbol(_s), left(_l), right(_r){}
};

class Node_unaop: public Ast {
    public:
        std::string op;
        Ast *expr;
        virtual nd_type get_nd_type() { return ND_TYPE_UNARY; }

        Node_unaop(std::string _o, Ast *_e):
            op(_o), expr(_e){}
};

struct var_t {
    Type *type;
    std::string name;
    Ast *init;
};

struct arg_t {
    Type *type;
    std::string name;
};

class Node_func_def: public Ast {
    public:
        Type *ret_type;
        std::string name;
        std::vector<arg_t> args;
        Ast_v block;
        virtual nd_type get_nd_type() { return ND_TYPE_FUNCDEF; }

        Node_func_def(Type *_r, std::string _n, std::vector<arg_t> _a, Ast_v _b):
            ret_type(_r), name(_n), args(_a), block(_b){}
};

class Node_func_call: public Ast {
    public:
        std::string name;
        Ast_v arg_v;
        virtual nd_type get_nd_type() { return ND_TYPE_FUNCCALL; }

        Node_func_call(std::string _n, Ast_v _a):
            name(_n), arg_v(_a){}
};

class Node_func_proto: public Ast {
    public:
        Type *ret_type;
        std::string name;
        Type_v types;
        virtual nd_type get_nd_type() { return ND_TYPE_FUNCPROTO; }

        Node_func_proto(Type *_r, std::string _n, Type_v _t):
            ret_type(_r), name(_n), types(_t){}
};

class Node_var_decl: public Ast {
    public:
        std::vector<var_t> decl_v;
        virtual nd_type get_nd_type() { return ND_TYPE_VARDECL; }

        Node_var_decl(std::vector<var_t> _d): decl_v(_d){}
};

class Node_assignment: public Ast {
    public:
        Ast *dst;
        Ast *src;
        virtual nd_type get_nd_type() { return ND_TYPE_ASSIGNMENT; }

        Node_assignment(Ast *_d, Ast *_s): dst(_d), src(_s){}
};

class Node_variable: public Ast {
    public:
        std::string name;
        bool isglobal = false;
        virtual nd_type get_nd_type() { return ND_TYPE_VARIABLE; }

        Node_variable(std::string _n): name(_n){}
};

class Node_string: public Ast {
    public:
        std::string string;
        virtual nd_type get_nd_type() { return ND_TYPE_STRING; }

        Node_string(std::string _s): string(_s){}
};

class Node_return: public Ast {
    public:
        Ast *cont;
        virtual nd_type get_nd_type() { return ND_TYPE_RETURN; }

        Node_return(Ast *_a): cont(_a){}
};

class Node_if: public Ast {
    public:
        Ast *cond;
        Ast *then_s, *else_s;
        virtual nd_type get_nd_type() { return ND_TYPE_IF; }

        Node_if(Ast *_c, Ast *_t, Ast *_e):
            cond(_c), then_s(_t), else_s(_e){}
};

class Node_for: public Ast {
    public:
        Ast *init, *cond, *reinit;
        Ast *body;
        virtual nd_type get_nd_type() { return ND_TYPE_FOR; }

        Node_for(Ast *_i, Ast *_c, Ast *_r, Ast *_b):
            init(_i), cond(_c), reinit(_r), body(_b){}
};

class Node_while: public Ast {
    public:
        Ast *cond;
        Ast *body;
        virtual nd_type get_nd_type() { return ND_TYPE_WHILE; }

        Node_while(Ast *_c, Ast *_b): cond(_c), body(_b){}
};

class Node_block: public Ast {
    public:
        Ast_v cont;
        virtual nd_type get_nd_type() { return ND_TYPE_BLOCK; }

        Node_block(Ast_v _c): cont(_c){}
};

class Parser {
    public:
        Ast_v run(Token token);
        void show(Ast *ast);

    private:
        Token token;
        bool is_func_def();
        bool is_var_decl();
        bool is_func_call();
        bool is_func_proto();

        Ast *var_decl();
        Type *eval_type();
        Ast *assignment();
        Ast *make_return();
        Ast *make_if();
        Ast *make_for();
        Ast *make_while();
        Ast *make_block();
        Ast *func_def();
        Ast *func_call();
        Ast *func_proto();
        Ast *expr();
        Ast *expr_first();
        Ast *expr_logic_or();
        Ast *expr_logic_and();
        Ast *expr_equality();
        Ast *expr_comp();
        Ast *expr_add();
        Ast *expr_mul();
        Ast *expr_unary();
        Ast *expr_unary_postfix();
        Ast *expr_primary();
        Ast *expr_num(token_t token);
        Ast *expr_var(token_t token);
        Ast_v eval();
        Ast *statement();

};

/*
 *  codegen
 */

class Program {
    public:
        void out(Ast_v asts);
        void gen(Ast *ast);
    private:
        void emit_head();
        void emit_num(Ast *ast);
        void emit_binop(Ast *ast);
        bool emit_log_andor(Node_binop *b);
        void emit_addr(Ast *ast);
        void emit_unaop(Ast *ast);
        void emit_if(Ast *ast);
        void emit_for(Ast *ast);
        void emit_while(Ast *ast);
        void emit_return(Ast *ast);
        void emit_block(Ast *ast);
        void emit_assign(Ast *ast);
        void emit_store(Ast *ast);
        void emit_func_def(Ast *ast);
        void emit_func_call(Ast *ast);
        void emit_func_head(Node_func_def*);
        void emit_func_end();
        void emit_vardecl(Ast *ast);
        void emit_variable(Ast *ast);
        void emit_cmp(std::string ord, Node_binop *a);
        std::string get_label();
        int get_var_pos(std::string name);
        std::string src;
        std::string x86_ord;
        bool isused_var = false;

        std::vector<std::string> vars;

        int labelnum = 1;
};

/*
 *  error
 */

void error(std::string msg);
void noexit_error(std::string msg);
