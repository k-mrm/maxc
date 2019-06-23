#ifndef MAXC_H
#define MAXC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>

typedef std::vector<uint8_t> bytecode;

/*
 *  main
 */

class Maxc {
    public:
        int run(std::string src);

        void show_usage();
    private:
        std::string version = "0.0.1";
};

/*
 *  token
 */

enum class TKind {
    End,
    Num,
    String,
    Char,
    Identifer,
    //KeyWord
    TInt,
    TUint,
    TInt64,
    TUint64,
    TBool,
    TChar,
    TString,
    TNone,
    KAnd,
    KOr,
    If,
    Else,
    For,
    While,
    Return,
    Typedef,
    Print,
    Println,
    Let,
    Fn,
    True,
    False,
    Const,
    //Symbol
    Lparen,     // (
    Rparen,     // )
    Lbrace,     // {
    Rbrace,     // }
    Lboxbracket,// [
    Rboxbracket,// ]
    Comma,      // ,
    Colon,      // :
    Dot,        // .
    DotDot,     // ..
    Semicolon,  // ;
    Arrow,      // ->
    Inc,        // ++
    Dec,        // --
    Plus,       // +
    Minus,      // -
    Asterisk,   // *
    Div,        // /
    Mod,        // %
    Eq,         // ==
    Neq,        // !=
    Lt,         // <
    Lte,        // <=
    Gt,         // >
    Gte,        // >=
    LogAnd,     // &&
    LogOr,      // ||
    Assign,     // =
    Question,   // ?
};

struct location_t {
    location_t() {}
    location_t(int l, int c): line(l), col(c) {}

    int line;
    int col;
};

struct token_t {
    TKind type;
    std::string value;

    //for error msg
    location_t start;
    location_t end;
};

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string &, location_t &, location_t &);
        void push_symbol(std::string &, location_t &, location_t &);
        void push_ident(std::string &, location_t &, location_t &);
        void push_string(std::string &, location_t &, location_t &);
        void push_char(std::string &, location_t &, location_t &);
        void push_end(location_t &, location_t &);

        void show();

        token_t get();
        token_t get_step();
        token_t see(int);
        location_t get_location();
        bool is(TKind);
        bool is(std::string);
        bool isctype();
        bool is_stmt();
        bool skip(TKind);
        bool skip(std::string);
        bool skip2(TKind, TKind);
        bool expect(TKind);
        void step();
        bool step_to(TKind);

        void save();
        void rewind();

    private:
        int save_point;
        bool issaved;
        int pos = 0;
        TKind str2ident(std::string);
        TKind str2symbol(std::string);
        const char *tk2str(TKind);
};

/*
 *  lexer
 */

class Lexer {
    public:
        Token &run(std::string src);
    private:
        Token token;
        void save(int, int);
        location_t &fetch();

        location_t saved_loc;
};

/*
 *  ctype
 */

enum class CTYPE {
    NONE        = 0b000000000001,
    INT         = 0b000000000010,
    UINT        = 0b000000000100,
    INT64       = 0b000000001000,
    UINT64      = 0b000000010000,
    BOOL        = 0b000000100000,
    CHAR        = 0b000001000000,
    STRING      = 0b000010000000,
    LIST        = 0b000100000000,
    TUPLE       = 0b001000000000,
    FUNCTION    = 0b010000000000,
};

class Type;
typedef std::vector<Type *> Type_v;

struct type_t {
    CTYPE type;
    int size;   //array size

    type_t() {}
    type_t(CTYPE ty): type(ty){}
    type_t(CTYPE ty, int size): type(ty), size(size) {}
};

class Type {
    private:
        type_t type;
    public:
        Type *ptr = nullptr;                            //list
        Type_v tuple;
        Type_v fnarg;   //function arg
        Type *fnret = nullptr;      //function rettype

        Type() {}
        Type(CTYPE ty): type(ty) {}
        Type(CTYPE ty, int size): type(ty, size) {}     //?
        Type(Type *p): type(CTYPE::LIST), ptr(p) {}     //list
        Type(Type_v a, Type *r): type(CTYPE::FUNCTION), fnarg(a), fnret(r) {}     //function

        std::string show();
        int get_size();
        type_t get();
        bool islist();
        bool istuple();
        bool isobject();
        bool isfunction();
        void tupletype_push(Type *);
};


/*
 *  AST, parser
 */

enum class NDTYPE {
    NUM = 100,
    BOOL,
    CHAR,
    LIST,
    SUBSCR,
    TUPLE,
    SYMBOL,
    IDENT,
    RETURN,
    FUNCDEF,
    FUNCCALL,
    FUNCPROTO,
    VARDECL,
    ASSIGNMENT,
    VARIABLE,
    BLOCK,
    STRING,
    BINARY,
    DOT,
    UNARY,
    TERNARY,
    IF,
    EXPRIF,
    FOR,
    WHILE,
    PRINT,
    PRINTLN,
    FORMAT,
    TYPEOF,
};

class Ast {
    public:
        Type *ctype;
        virtual NDTYPE get_nd_type() = 0;
};

typedef std::vector<Ast *> Ast_v;

//Variable, arguments

enum class VarAttr {
    Const     = 0b0001,
    Uninit    = 0b0010,
};

struct var_t {
    int vattr;
    Type *type;
    std::string name;
};

struct arg_t {
    Type *type;
    std::string name;
};

class NodeVariable;

class Varlist {
    public:
        void push(NodeVariable *v);
        std::vector<NodeVariable *> var_v;
        std::vector<NodeVariable *> get();
        NodeVariable *find(std::string n);
        void show();
        void reset();
    private:
};

//Function
enum class BltinFnKind {
    Println,
    PrintlnInt,
    PrintlnBool,
    PrintlnChar,
    PrintlnString,
    PrintlnList,
};

struct func_t {
    std::string name;
    BltinFnKind fnkind;
    Varlist args;
    Type *ftype;
    bool isbuiltin;

    func_t() {}
    func_t(std::string n, BltinFnKind k, Type *f):
        name(n), fnkind(k), ftype(f), isbuiltin(true) {}
    func_t(std::string n, Type *f): name(n), ftype(f), isbuiltin(false) {}
    func_t(std::string n, Varlist a, Type *f):
        name(n), args(a), ftype(f), isbuiltin(false) {}
};

/*
   class Funclist {
   public:
   std::vector<func_t> funcs;
   void push(func_t f);
   func_t *find(std::string);
   };
   */

//AST
//nodes

class NodeNumber: public Ast {
    public:
        int number;
        virtual NDTYPE get_nd_type() { return NDTYPE::NUM; }

        NodeNumber(int _n): number(_n) {
            ctype = new Type(CTYPE::INT);
        }
};

class NodeBool: public Ast {
    public:
        bool boolean;
        virtual NDTYPE get_nd_type() { return NDTYPE::BOOL; }

        NodeBool(bool b): boolean(b) {
            ctype = new Type(CTYPE::BOOL);
        }
};

class NodeChar: public Ast {
    public:
        char ch;
        virtual NDTYPE get_nd_type() { return NDTYPE::CHAR; }

        NodeChar(char _c): ch(_c) {
            ctype = new Type(CTYPE::CHAR);
        }
};

class NodeString: public Ast {
    public:
        char *string;
        virtual NDTYPE get_nd_type() { return NDTYPE::STRING; }

        NodeString(const char *_s) {
            string = (char *)malloc(sizeof(char) * strlen(_s));
            strncpy(string, _s, strlen(_s) + 1);
            ctype = new Type(CTYPE::STRING);
        }
};

class NodeList: public Ast {
    public:
        Ast_v elem;
        size_t nsize;
        Ast *nindex;
        virtual NDTYPE get_nd_type() { return NDTYPE::LIST; }

        NodeList(Ast_v e, size_t s): elem(e), nsize(s) {}
        NodeList(Ast_v e, Ast *n): elem(e), nindex(n) {
            ctype = new Type(CTYPE::LIST);
        }
};

class NodeTuple: public Ast {
    public:
        Ast_v exprs;
        size_t nsize;
        virtual NDTYPE get_nd_type() { return NDTYPE::TUPLE; }

        NodeTuple(Ast_v e, size_t n, Type *t): exprs(e), nsize(n) {
            ctype = t;
        }
};

class NodeBinop: public Ast {
    public:
        std::string symbol;
        Ast *left;
        Ast *right;
        virtual NDTYPE get_nd_type() { return NDTYPE::BINARY; }

        NodeBinop(std::string _s, Ast *_l, Ast *_r):
            symbol(_s), left(_l), right(_r) {}
};

enum class Method;
class NodeDotop: public Ast {
    public:
        Ast *left;
        Ast *right;
        Method method;
        bool isobj = false;
        virtual NDTYPE get_nd_type() { return NDTYPE::DOT; }

        NodeDotop(Ast *l, Ast *r): left(l), right(r) {}
        NodeDotop(Ast *l, Method m, Type *t):
            left(l), method(m), isobj(true) { ctype = t; }
};

class NodeSubscript: public Ast {
    public:
        Ast *ls;
        Ast *index;
        bool istuple = false; //default -> list
        virtual NDTYPE get_nd_type() { return NDTYPE::SUBSCR; }

        NodeSubscript(Ast *l, Ast *i, Type *t): ls(l), index(i) {
            ctype = t;
        }
        NodeSubscript(Ast *l, Ast *i, Type *t, bool b): ls(l), index(i), istuple(b) {
            ctype = t;
        }
};

class NodeUnaop: public Ast {
    public:
        std::string op;
        Ast *expr;
        virtual NDTYPE get_nd_type() { return NDTYPE::UNARY; }

        NodeUnaop(std::string _o, Ast *_e, Type *_t):
            op(_o), expr(_e){ ctype = _t; }
};

class NodeTernop: public Ast {
    public:
        Ast *cond, *then, *els;
        virtual NDTYPE get_nd_type() { return NDTYPE::TERNARY; }

        NodeTernop(Ast *c, Ast *t, Ast *e):
            cond(c), then(t), els(e) {}
};

class NodeAssignment: public Ast {
    public:
        Ast *dst;
        Ast *src;
        virtual NDTYPE get_nd_type() { return NDTYPE::ASSIGNMENT; }

        NodeAssignment(Ast *_d, Ast *_s): dst(_d), src(_s) {}
};

class NodeFunction;
class NodeVariable: public Ast {
    public:
        var_t vinfo;
        func_t finfo;
        bool isglobal = false;
        NodeVariable *vid;
        virtual NDTYPE get_nd_type() { return NDTYPE::VARIABLE; }

        NodeVariable(var_t _v, bool _b): vinfo(_v), isglobal(_b){
            ctype = _v.type; vid = this;
        }
        NodeVariable(func_t f, bool b): finfo(f), isglobal(b) {
            ctype = f.ftype; vid = this;
        }
};

//Variable list


//Env

struct env_t {
    Varlist vars;
    env_t *parent;
    bool isglb;

    env_t(){}
    env_t(bool i): isglb(i){}
};

class Env {
    public:
        env_t *current = nullptr;
        env_t *make();
        env_t *escape();
        env_t *get();
        bool isglobal();
};

class NodeVardecl: public Ast {
    public:
        NodeVariable *var;
        Ast *init;
        virtual NDTYPE get_nd_type() { return NDTYPE::VARDECL; }

        NodeVardecl(NodeVariable *_v, Ast *_i): var(_v), init(_i){}
};

//Node func

class NodeFunction: public Ast {
    public:
        NodeVariable *fnvar;
        func_t finfo;
        Ast_v block;
        Varlist lvars;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCDEF; }

        NodeFunction(
                NodeVariable *fv,
                func_t f,
                Ast_v b,
                Varlist l
                ): fnvar(fv), finfo(f), block(b), lvars(l) {}
};

class NodeFnCall: public Ast {
    public:
        Ast *func;
        Ast_v args;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCCALL; }

        NodeFnCall(Ast *f, Ast_v a): func(f), args(a) {}
};

class NodeFnProto: public Ast {
    public:
        Type *ret_type;
        std::string name;
        Type_v types;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCPROTO; }

        NodeFnProto(Type *_r, std::string _n, Type_v _t):
            ret_type(_r), name(_n), types(_t){}
};


class NodeReturn: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::RETURN; }

        NodeReturn(Ast *_a): cont(_a){}
};

class NodeIf: public Ast {
    public:
        Ast *cond;
        Ast *then_s, *else_s;
        virtual NDTYPE get_nd_type() { return NDTYPE::IF; }

        NodeIf(Ast *_c, Ast *_t, Ast *_e):
            cond(_c), then_s(_t), else_s(_e){}
};

class NodeExprif: public Ast {
    public:
        Ast *cond;
        Ast *then_s, *else_s;
        virtual NDTYPE get_nd_type() { return NDTYPE::EXPRIF; }

        NodeExprif(Ast *_c, Ast *_t, Ast *_e):
            cond(_c), then_s(_t), else_s(_e){}
};

class NodeFor: public Ast {
    public:
        Ast *init, *cond, *reinit;
        Ast *body;
        virtual NDTYPE get_nd_type() { return NDTYPE::FOR; }

        NodeFor(Ast *_i, Ast *_c, Ast *_r, Ast *_b):
            init(_i), cond(_c), reinit(_r), body(_b){}
};

class NodeWhile: public Ast {
    public:
        Ast *cond;
        Ast *body;
        virtual NDTYPE get_nd_type() { return NDTYPE::WHILE; }

        NodeWhile(Ast *_c, Ast *_b): cond(_c), body(_b){}
};

class NodeBlock: public Ast {
    public:
        Ast_v cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::BLOCK; }

        NodeBlock(Ast_v _c): cont(_c){}
};

class NodePrint: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::PRINT; }

        NodePrint(Ast *c): cont(c) {
            ctype = new Type(CTYPE::NONE);
        }
};

class NodePrintln: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::PRINTLN; }

        NodePrintln(Ast *c): cont(c) {
            ctype = new Type(CTYPE::NONE);
        }
};

class NodeFormat: public Ast {
    public:
        std::string cont;
        int narg;
        Ast_v args;
        virtual NDTYPE get_nd_type() { return NDTYPE::FORMAT; }

        NodeFormat(std::string c, int n, Ast_v a):
            cont(c), narg(n), args(a) {
                ctype = new Type(CTYPE::STRING);
            }
};

class NodeTypeof: public Ast {
    public:
        NodeVariable *var;
        virtual NDTYPE get_nd_type() { return NDTYPE::TYPEOF; }

        NodeTypeof(NodeVariable *v): var(v) {}
};

/*
 * Object
 */
enum class ObjKind {
    List,
    String,
    Tuple,
    Function,
    UserDef,
};

enum class Method {
    ListSize,
    ListAccess,
    StringLength,
    StringAccess,
    StringtoInt,
    TupleAccess,
};

struct userfunction {
    userfunction() {}
    userfunction(bytecode &c, Varlist &v): code(c), vars(v) {}

    bytecode code;
    Varlist vars;
};

struct MxcObject;

typedef MxcObject *(*bltinfn_ty)(MxcObject **, size_t);

struct MxcObject {
    CTYPE type;
    size_t refcount;
};

struct IntObject : MxcObject {
    int inum32;
};

struct BoolObject: MxcObject {
    bool boolean;
};

struct CharObject: MxcObject {
    char ch;
};

struct ListObject: MxcObject {
    MxcObject **elem;
    size_t allocated;
};

struct StringObject: MxcObject {
    const char *str;
};

struct TupleObject: MxcObject {};   //TODO

struct FunctionObject: MxcObject {
    userfunction func;
};

struct BltinFuncObject: MxcObject {
    bltinfn_ty func;
};

struct NullObject: MxcObject {};


class Parser {
    public:
        Parser(Token &t): token(t) {}
        Ast_v &run();
        void show(Ast *ast);
        Env env;

    private:
        Token &token;
        Ast_v program;
        bool is_func_call();
        void expect_type(CTYPE, Ast *);  //1:expected type, 2:real
        Ast *read_lsmethod(Ast *);
        Ast *read_strmethod(Ast *);
        Ast *read_tuplemethod(Ast *);

        Ast *var_decl(bool isconst);
        Type *eval_type();
        Type *skip_index(CTYPE);
        Ast *assignment();
        Ast *make_assign(Ast *dst, Ast *src);
        Ast *make_assigneq(std::string, Ast *, Ast *);
        Ast *make_return();
        Ast *make_if();
        Ast *make_for();
        Ast *make_while();
        Ast *make_block();
        Ast *make_print();
        Ast *make_println();
        Ast *make_format();
        Ast *make_typeof();
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

        void set_global();

        bool ensure_hasmethod(Type *);

        Varlist vls;
        std::unordered_map<std::string, Type *> typemap;
};

/*
 *  semantics checker
 */

class SemaAnalyzer {
    public:
        Ast_v &run(Ast_v &);

    private:
        Ast_v ret_ast;

        Ast *visit(Ast *);
        Ast *visit_binary(Ast *);
        Ast *visit_assign(Ast *);
        Ast *visit_vardecl(Ast *);
        Ast *visit_load(Ast *);
        Ast *visit_fncall(Ast *);
        Ast *visit_bltinfn_call(NodeFnCall *);

        Type *checktype(Type *, Type *);
};


/*
 *  codegen
 */

enum class OpCode : uint8_t {
    END,
    PUSH,
    IPUSH,
    PUSHCONST_1,
    PUSHCONST_2,
    PUSHCONST_3,
    PUSHTRUE,
    PUSHFALSE,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LOGOR,
    LOGAND,
    EQ,
    NOTEQ,
    LT,
    LTE,
    GT,
    GTE,
    LABEL,
    JMP,
    JMP_EQ,
    JMP_NOTEQ,
    INC,
    DEC,
    PRINT,
    PRINTLN,
    FORMAT,
    TYPEOF,
    LOAD_GLOBAL,
    LOAD_LOCAL,
    STORE_GLOBAL,
    STORE_LOCAL,
    LISTSET,
    SUBSCR,
    SUBSCR_STORE,
    STRINGSET,
    TUPLESET,
    FUNCTIONSET,
    BLTINFN_SET,
    RET,
    CALL,
    CALL_BLTIN,
    CALLMethod,
};

enum class ObKind {
    Int,
    Bool,
    Char,
    String,
    List,
    Tuple,
    Function,
    UserDef,
    Null,
};


struct const_t {
    const char *str;    //str
    NodeVariable *var;
    userfunction func;

    const_t(const char *s): str(s), var(nullptr) {}
    const_t(NodeVariable *v): str(nullptr), var(v) {}
    const_t(userfunction u): str(nullptr), var(nullptr), func(u) {}
};

class Constant {
    public:
        std::vector<const_t> table;

        int push_var(NodeVariable *);
        int push_str(const char *);
        int push_userfunc(userfunction &);
        int push_bltinfunc(bltinfn_ty &);
};

namespace Bytecode {
    void push_0arg(bytecode &, OpCode);
    void push_ipush(bytecode &, int32_t);
    void push_jmpneq(bytecode &, size_t);
    void push_jmp(bytecode &, size_t);
    void push_store(bytecode &, int, bool);
    void push_load(bytecode &, int, bool);
    void push_strset(bytecode &, int);
    void push_functionset(bytecode &, int);
    void push_bltinfn_set(bytecode &, BltinFnKind);
    void push_bltinfn_call(bytecode &, int);


    void replace_int32(size_t, bytecode &, size_t);
    void push_int8(bytecode &, int8_t);
    void push_int32(bytecode &, int32_t);
    int32_t read_int32(bytecode &, size_t &);
};

class BytecodeGenerator {
    public:
        BytecodeGenerator(Constant &c): ctable(c) {}

        void compile(Ast_v, Env, bytecode &);
        void gen(Ast *, bytecode &, bool);
        void show(bytecode &, size_t &);
        std::map<const char *, int> lmap;
    private:
        Constant &ctable;

        void emit_head();
        void emit_num(Ast *, bytecode &, bool);
        void emit_bool(Ast *, bytecode &, bool);
        void emit_char(Ast *, bytecode &, bool);
        void emit_string(Ast *, bytecode &, bool);
        void emit_list(Ast *, bytecode &);
        void emit_listaccess(Ast *, bytecode &);
        void emit_tuple(Ast *, bytecode &);
        void emit_binop(Ast *, bytecode &, bool);
        void emit_dotop(Ast *, bytecode &);
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
        void emit_store(Ast *, bytecode &);
        void emit_listaccess_store(Ast *, bytecode &);
        void emit_func_def(Ast *, bytecode &);
        void emit_func_call(Ast *, bytecode &, bool);
        void emit_bltinfunc_call(NodeFnCall *, bytecode &, bool);
        void emit_func_head(NodeFunction *);
        void emit_func_end();
        void emit_vardecl(Ast *, bytecode &);
        void emit_load(Ast *, bytecode &);

        //VMcode push
        void vcpush(OpCode, int);
        void vcpush(OpCode, char);
        void vcpush(OpCode, const char *);
        void vcpush(OpCode, NodeVariable *);
        void vcpush(OpCode, char *, unsigned int);
        void vcpush(OpCode, size_t);
        void vcpush(OpCode, Method);
        void vcpush(OpCode, size_t, size_t);

        int nline = 0;
        std::stack<size_t> fnpc;

        int labelnum = 1;

        Env env;
};

/*
 *  Object
 */

namespace Object {
    MxcObject *Mxc_malloc(size_t);

    IntObject *alloc_intobject(int);
    IntObject *int_add(IntObject *, IntObject *);
    IntObject *int_sub(IntObject *, IntObject *);
    IntObject *int_mul(IntObject *, IntObject *);
    IntObject *int_div(IntObject *, IntObject *);
    IntObject *int_mod(IntObject *, IntObject *);
    BoolObject *int_eq(IntObject *, IntObject *);
    BoolObject *int_noteq(IntObject *, IntObject *);
    BoolObject *int_lt(IntObject *, IntObject *);
    BoolObject *int_lte(IntObject *, IntObject *);
    BoolObject *int_gt(IntObject *, IntObject *);
    BoolObject *int_gte(IntObject *, IntObject *);
    IntObject *int_inc(IntObject *);
    IntObject *int_dec(IntObject *);

    BoolObject *bool_logor(BoolObject *, BoolObject *);
    BoolObject *bool_logand(BoolObject *, BoolObject *);

    BoolObject *alloc_boolobject(bool);
    CharObject *alloc_charobject(char);
    StringObject *alloc_stringobject(const char *);
    FunctionObject *alloc_functionobject(userfunction &);
    BltinFuncObject *alloc_bltinfnobject(bltinfn_ty &);
    ListObject *alloc_listobject(size_t);

    BoolObject *bool_from_int(IntObject *);
};

typedef std::map<NodeVariable *, MxcObject *> localvar;
typedef std::map<NodeVariable *, MxcObject *> globalvar;

class Frame {
    public:
        Frame(bytecode &b): code(b), pc(0) {}

        Frame(userfunction &u): code(u.code), pc(0) {}

        bytecode &code;
        localvar lvars;
        size_t pc;
    private:
};


/*
 *  VM
 */

class VM {
    public:
        VM(Constant *c): ctable(c){}

        int run(bytecode &);
    private:
        MxcObject **stackptr;

        Frame *frame;

        std::stack<unsigned int> locs;
        std::stack<FunctionObject *> fnstk;
        globalvar gvmap;
        Constant *ctable;

        std::stack<Frame *, std::vector<Frame *>> framestack;

        void print(MxcObject *);
        int exec();
};

/*
 *  error
 */

enum class ErrorKind {
};

void error(const char *, ...);
void error(const location_t &, const location_t &, const char *, ...);
void warning(const location_t &, const location_t &, const char *, ...);
void runtime_err(const char *, ...);
void debug(const char *, ...);
void showline(int, int);

#endif
