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
    TOKEN_TYPE_SYMBOL
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

        void to_asm();

    private:
};

//lexer
class Lexer {
    public:
        Token run(std::string src);
};

//parser, ast
enum nd_type {
    ND_TYPE_NUM,
    ND_TYPE_PLUS,
    ND_TYPE_MINUS
};

struct node_t {
    nd_type type;
    node_t *left;
    node_t *right;
    int value;
};

class Ast {
    public:
        //node_t node;
        void make_node();
        void show();
};

class Parser {
    public:
        void run(Token token);
};

/*
class Ast {
    public:
        void gen_asm();
        void show();
};
*/
