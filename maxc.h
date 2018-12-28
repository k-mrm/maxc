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
        Token lex(std::string src);
};

//parser
class Parser {
    public:
        void parse(Token token);
};

//ast
class Ast {
    public:
};
