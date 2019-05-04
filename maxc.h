#ifndef MAXC_H
#define MAXC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>

/*
 *  main
 */

class Maxc {
    public:
        int run(std::string src);

        void show_usage();
        std::string version = "0.0.1";
    private:
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
    Print,
    Println,
    Let,
    Fn,
    True,
    False,
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
    int line;
    int col;

    location_t(int l, int c): line(l), col(c) {}
};

typedef struct {
    TKind type;
    std::string value;
    int line; int col;
    //location_t cur;
} token_t;

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string value, int line, int col);
        void push_symbol(std::string value, int line, int col);
        void push_ident(std::string value, int line, int col);
        void push_string(std::string value, int line, int col);
        void push_char(std::string value, int line, int col);
        void push_end(int, int);

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
        Token run(std::string src);
    private:
        Token token;
};

/*
 *  ctype
 */

enum class CTYPE {
    NONE,
    INT,
    UINT,
    INT64,
    UINT64,
    BOOL,
    CHAR,
    STRING,
    LIST,
    TUPLE,
    FUNCTION,
    PTR,
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

struct var_t {
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
struct vmcode_t;
struct func_t {
    std::string name;
    Varlist args;
    Type *ftype;

    func_t() {}
    func_t(std::string n, Type *f): name(n), ftype(f) {}
    func_t(std::string n, Varlist a, Type *f): name(n), args(a), ftype(f) {}
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

        NodeList(Ast_v e, size_t s, Type *b): elem(e), nsize(s) {
            ctype = b;
        }
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

        NodeBinop(std::string _s, Ast *_l, Ast *_r, Type *_t):
            symbol(_s), left(_l), right(_r) { ctype = _t; }
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

        NodeTernop(Ast *c, Ast *t, Ast *e, Type *ty): cond(c), then(t), els(e) {
            ctype = ty;
        }
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
        Varlist var;
        Ast_v init;
        virtual NDTYPE get_nd_type() { return NDTYPE::VARDECL; }

        NodeVardecl(Varlist _v, Ast_v _i): var(_v), init(_i){}
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
        NodeVariable *func;
        Ast_v args;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCCALL; }

        NodeFnCall(NodeVariable *f, Ast_v a): func(f), args(a) {
            ctype = func->finfo.ftype->fnret;
        }
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


struct MxcObject {
    CTYPE type;
    size_t refcount;
};

struct IntObject : MxcObject {
    union {
        int inum32;
        uint32_t unum32;
        int64_t inum64;
        uint64_t unum64;
    };
};

struct BoolObject: MxcObject {
    bool boolean;
};

struct CharObject: MxcObject {
    char ch;
};

struct ListObject : MxcObject {
    MxcObject **elem;
    size_t allocated;
};

struct StringObject: MxcObject {
    const char *str;
};

struct TupleObject: MxcObject {};   //TODO

struct FunctionObject: MxcObject {
    size_t start;
};

struct NullObject: MxcObject {};

class Parser {
    public:
        Ast_v run(Token);
        void show(Ast *ast);
        Env env;

    private:
        Token token;
        bool is_func_call();
        int skip_ptr();
        Type *checktype(Type *, Type *);
        Ast *read_lsmethod(Ast *);
        Ast *read_strmethod(Ast *);
        Ast *read_tuplemethod(Ast *);

        Ast *var_decl();
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
        Ast *func_def();
        Ast *func_call();
        Ast *expr();
        Ast *expr_first();
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
        Ast_v eval();
        Ast *statement();

        bool ensure_hasmethod(Type *);

        Varlist vls;
};


/*
 *  codegen
 */

enum class OPCODE {
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
    LOAD,
    STORE,
    ISTORE,
    LISTSET,
    SUBSCR,
    SUBSCR_STORE,
    STRINGSET,
    TUPLESET,
    FUNCTIONSET,
    RET,
    CALL,
    CALLMethod,
    FNBEGIN,
    FNEND,
    END,
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

struct vmcode_t {
    OPCODE type;
    union {
        int num = 0;
        char ch;
    };
    const char *str;
    NodeVariable *var = nullptr;
    Method obmethod;
    unsigned int nfarg;

    size_t size;                //list
    size_t fnstart, fnend;      //list
    int nline;

    vmcode_t() {}
    vmcode_t(OPCODE t, int l): type(t), nline(l) {}
    vmcode_t(OPCODE t, int v, int l): type(t), num(v), nline(l) {}
    vmcode_t(OPCODE t, char c, int l): type(t), ch(c), nline(l) {}
    vmcode_t(OPCODE t, const char *s, int l): type(t), str(s), nline(l) {}
    vmcode_t(OPCODE t, size_t ls, int l): type(t), size(ls), nline(l) {}
    vmcode_t(OPCODE t, NodeVariable *vr, int l):
        type(t), var(vr), nline(l) {}
    vmcode_t(OPCODE t, const char *s, unsigned int n, int l):
        type(t), str(s), nfarg(n), nline(l) {}  //format
    vmcode_t(OPCODE t, Method m, int l): type(t), obmethod(m), nline(l) {}
    vmcode_t(OPCODE t, size_t fs, size_t fe, int l): type(t), fnstart(fs), fnend(fe), nline(l) {}
};

class Program {
    public:
        void compile(Ast_v, Env);
        void gen(Ast *);
        void show(vmcode_t &);
        std::vector<vmcode_t> vmcodes;
        std::map<const char *, int> lmap;
    private:
        void emit_head();
        void emit_num(Ast *);
        void emit_bool(Ast *);
        void emit_char(Ast *);
        void emit_string(Ast *);
        void emit_list(Ast *);
        void emit_listaccess(Ast *);
        void emit_tuple(Ast *);
        void emit_binop(Ast *);
        void emit_object_oprator(Ast *);
        void emit_dotop(Ast *);
        void emit_ternop(Ast *);
        void emit_pointer(NodeBinop *);
        void emit_addr(Ast *);
        void emit_unaop(Ast *);
        void emit_if(Ast *);
        void emit_exprif(Ast *);
        void emit_for(Ast *);
        void emit_while(Ast *);
        void emit_return(Ast *);
        void emit_block(Ast *);
        void emit_print(Ast *);
        void emit_println(Ast *);
        void emit_format(Ast *);
        void emit_typeof(Ast *);
        void emit_assign(Ast *);
        void emit_store(Ast *);
        void emit_listaccess_store(Ast *);
        void emit_func_def(Ast *);
        void emit_func_call(Ast *);
        void emit_func_head(NodeFunction *);
        void emit_func_end();
        void emit_vardecl(Ast *);
        void emit_load(Ast *);

        //VMcode push
        void vcpush(OPCODE);
        void vcpush(OPCODE, int);
        void vcpush(OPCODE, char);
        void vcpush(OPCODE, const char *);
        void vcpush(OPCODE, NodeVariable *);
        void vcpush(OPCODE, char *, unsigned int);
        void vcpush(OPCODE, size_t);
        void vcpush(OPCODE, Method);
        void vcpush(OPCODE, size_t, size_t);

        void opcode2str(OPCODE);
        char *get_label();
        int get_lvar_size();
        int size;
        int get_var_pos(std::string);
        int nline = 0;
        bool isused_var = false;
        bool isexpr = false;
        bool isinfunction = false;
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
    FunctionObject *alloc_functionobject(size_t);
    ListObject *alloc_listobject(size_t);

    BoolObject *bool_from_int(IntObject *);

    void incref(MxcObject *);
    void decref(MxcObject *);
};


/*
 *  VM
 */

class VMEnv;
class VM {
    public:
        int run(std::vector<vmcode_t> &, std::map<const char *, int> &);
        void exec(std::vector<vmcode_t> &);
    private:
        std::stack<MxcObject *> stk;
        std::stack<unsigned int> locs;
        std::stack<FunctionObject *> fnstk;
        std::map<NodeVariable *, MxcObject *> gvmap;
        std::map<const char *, int> labelmap;
        void print(MxcObject *);
        unsigned int pc = 0;
        VMEnv *env;
        int tcnt;
        size_t cnt;
};

struct vmenv_t {
    std::map<NodeVariable *, MxcObject *> vmap;
    vmenv_t *parent;

    vmenv_t() {}
    vmenv_t(vmenv_t *p): parent(p) {}
};

class VMEnv {
    public:
        vmenv_t *cur;
        vmenv_t *make();
        vmenv_t *escape();
        std::map<NodeVariable *, MxcObject *> getvmap();

        VMEnv() {}
};

/*
 *  error
 */

void error(const char *msg, ...);
void error(int line, int col, const char *msg, ...);
void warning(int line, int col, const char *msg, ...);
void runtime_err(const char *msg, ...);
void debug(const char *msg, ...);
std::string skipln(int n);

#endif
