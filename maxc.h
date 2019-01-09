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
        void push_end();

        void show();
    private:
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
    ND_TYPE_IDENT
};

class Ast {
    public:
        virtual nd_type get_nd_type() = 0;
};

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

class Parser {
    public:
        Ast *run(Token token);
        void show(Ast *ast);
        Ast *expr_add(std::vector<token_t> tokens);
        Ast *expr_mul(std::vector<token_t> tokens);
        Ast *expr_primary(std::vector<token_t> tokens);
        Ast *expr_num(token_t token);
    private:
        size_t pos = 0;
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
