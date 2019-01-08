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
    ND_TYPE_ADD,
    ND_TYPE_SUB,
    ND_TYPE_MUL,
    ND_TYPE_DIV,
    ND_TYPE_MOD,
    ND_TYPE_IDENT
};

struct ast_t {
    nd_type type;
    std::string value;
    ast_t *left;
    ast_t *right;
};

class Ast {
    public:
        void make(Token token);
        ast_t *node;
    private:
        int pos;
        ast_t *expr_add(std::vector<token_t> tokens);
        ast_t *expr_num(token_t token);
        ast_t *expr_mul(std::vector<token_t> tokens);
        ast_t *expr_primary(std::vector<token_t> tokens);
        ast_t *make_node(std::string value, ast_t *left, ast_t *right);
        ast_t *make_num_node(token_t token);
        void show();
        void show(ast_t *current);
        void print_pos(std::string msg);
        std::string ret_type(nd_type ty);
};

class Parser {
    public:
        ast_t *run(Token token);
};

//codegen(asm)
class Program {
    public:
        Program();
        ~Program();
        void gen(ast_t *ast);
    private:
        std::string src;
        std::string x86_ord;
};
