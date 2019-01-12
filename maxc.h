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
} token_t;

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string value);
        void push_symbol(std::string value);
        void push_ident(std::string value);
        void push_end();

        void show();

        token_t get();
        token_t get_step();
        bool is_value(std::string tk);
        bool is_type(Token_type ty);
        bool skip(std::string val);
        void step();
        bool step_to(std::string val);
    private:
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
    ND_TYPE_VARDECL,
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
    TYPE_INT
};

struct var_t {
    var_type type;
    std::string name;
};

class Node_var_decl: public Ast {
    public:
        std::vector<var_t> decl_v;
        virtual nd_type get_nd_type() { return ND_TYPE_VARDECL; }

        Node_var_decl(std::vector<var_t> _d): decl_v(_d){}
};

class Node_string: public Ast {
    public:
        std::string string;
        virtual nd_type get_nd_type() { return ND_TYPE_STRING; }

        Node_string(std::string _s): string(_s){}
};

class Parser {
    public:
        Ast *run(Token token);
        Ast_v eval(std::vector<token_t>);
        void show(Ast *ast);

        Ast *var_decl();
        Ast *expr_add();
        Ast *expr_mul();
        Ast *expr_primary();
        Ast *expr_num(token_t token);

        Ast *statement();
    private:
        Token token;
};


//codegen(asm)
class Program {
    public:
        Program();
        ~Program();
        void gen(Ast *ast);
    private:
        std::string src;
        std::string x86_ord;
};

//error
void error(std::string msg);
void noexit_error(std::string msg);
