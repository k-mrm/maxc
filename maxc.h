#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

//main
class Maxc {
    public:
        int run(std::string src);

        void show_usage();
    private:
};

//token
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
        void push_end();

        void show();

        token_t get();
        token_t get_step();
        token_t see(int p);
        bool is_value(std::string tk);
        bool is_type(Token_type ty);
        bool is_type();
        bool skip(std::string val);
        void step();
        bool step_to(std::string val);

        void save();
        void rewind();

    private:
        int save_point;
        bool issaved;
        int pos = 0;
};

//lexer
class Lexer {
    public:
        Token run(std::string src);
};

//parser, ast
enum nd_type {
    ND_TYPE_NUM = 100,
    ND_TYPE_SYMBOL,
    ND_TYPE_IDENT,
    ND_TYPE_RETURN,
    ND_TYPE_FUNCDEF,
    ND_TYPE_FUNCCALL,
    ND_TYPE_VARDECL,
    ND_TYPE_ASSIGNMENT,
    ND_TYPE_VARIABLE,
    ND_TYPE_STRING
};

class Ast {
    public:
        virtual nd_type get_nd_type() = 0;
};

typedef std::vector<Ast *> Ast_v;

class Node_number: public Ast {
    public:
        std::string number;
        virtual nd_type get_nd_type() { return ND_TYPE_NUM; }

        Node_number(std::string _n): number(_n){}
};

class Node_binop: public Ast {
    public:
        std::string symbol;
        Ast *left;
        Ast *right;
        virtual nd_type get_nd_type() { return ND_TYPE_SYMBOL; }

        Node_binop(std::string _s, Ast *_l, Ast *_r):
            symbol(_s), left(_l), right(_r){}
};

enum var_type {
    TYPE_INT,
    TYPE_VOID,
};

struct var_t {
    var_type type;
    std::string name;
};

struct arg_t {
    var_type type;
    std::string name;
};

class Node_func_def: public Ast {
    public:
        var_type ret_type;
        std::string name;
        std::vector<arg_t> arg_v;
        Ast_v block;
        virtual nd_type get_nd_type() { return ND_TYPE_FUNCDEF; }

        Node_func_def(var_type _r, std::string _n, std::vector<arg_t> _a, Ast_v _b):
            ret_type(_r), name(_n), arg_v(_a), block(_b){}
};

class Node_func_call: public Ast {
    public:
        std::string name;
        Ast_v arg_v;
        virtual nd_type get_nd_type() { return ND_TYPE_FUNCCALL; }

        Node_func_call(std::string _n, Ast_v _a):
            name(_n), arg_v(_a){}
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

class Parser {
    public:
        Ast_v run(Token token);
        Ast_v eval();
        void show(Ast *ast);

        Ast *var_decl();
        var_type eval_type();
        Ast *assignment();
        Ast *make_return();
        Ast *func_def();
        Ast *func_call();
        Ast *expr();
        Ast *expr_add();
        Ast *expr_mul();
        Ast *expr_primary();
        Ast *expr_num(token_t token);
        Ast *expr_var(token_t token);

        Ast *statement();
    private:
        Token token;
        std::string show_type(var_type ty);
        bool is_func_def();
        bool is_var_decl();
        bool is_func_call();
};


//codegen(asm)
class Program {
    public:
        void out(Ast_v asts);
        void gen(Ast *ast);
    private:
        void emit_head();
        void emit_num(Ast *ast);
        void emit_binop(Ast *ast);
        void emit_return(Ast *ast);
        void emit_assign(Ast *ast);
        void emit_assign_left(Ast *ast);
        void emit_func_def(Ast *ast);
        void emit_func_call(Ast *ast);
        void emit_func_head(Node_func_def*);
        void emit_func_end();
        void emit_vardecl(Ast *ast);
        void emit_variable(Ast *ast);
        int get_var_pos(std::string name);
        int get_type_size(var_type ty);
        std::string src;
        std::string x86_ord;
        bool isused_var = false;

        std::vector<var_t> vars;
};

//error
void error(std::string msg);
void noexit_error(std::string msg);
