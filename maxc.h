#pragma once

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

enum class TOKEN_TYPE {
    END,
    NUM,
    SYMBOL,
    STRING,
    CHAR,
    IDENTIFER
};

struct location_t {
    int line;
    int col;

    location_t(int l, int c): line(l), col(c) {}
};

typedef struct {
    TOKEN_TYPE type;
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
        token_t see(int p);
        location_t get_location();
        bool is_value(std::string tk);
        bool is_type(TOKEN_TYPE ty);
        bool isctype();
        bool is_stmt();
        bool skip(std::string val);
        bool skip2(std::string, std::string);
        bool expect(std::string val);
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
    CHAR,
    LIST,
    ACCESS,
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
    std::vector<vmcode_t> codes;

    func_t() {}
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
        std::string string;
        virtual NDTYPE get_nd_type() { return NDTYPE::STRING; }

        NodeString(std::string _s): string(_s){
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

class NodeAccess: public Ast {
    public:
        Ast *ls;
        Ast *index;
        bool istuple = false; //default -> list
        virtual NDTYPE get_nd_type() { return NDTYPE::ACCESS; }

        NodeAccess(Ast *l, Ast *i, Type *t): ls(l), index(i) {
            ctype = t;
        }
        NodeAccess(Ast *l, Ast *i, Type *t, bool b): ls(l), index(i), istuple(b) {
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
        func_t finfo;
        Ast_v block;
        Varlist lvars;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCDEF; }

        NodeFunction(
                func_t f,
                Ast_v b,
                Varlist l
                ): finfo(f), block(b), lvars(l) {}
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

struct value_t;

class Object {
    public:
};

class ListObject : public Object {
    public:
        std::vector<value_t> lselem;

        size_t get_size();
        value_t get_item(int);
        void set_item(std::vector<value_t>);
};

class StringObject: public Object {
    public:
        std::string str;
        int get_length();
        int to_int();
};

class TupleObject: public Object {
    public:
        std::vector<value_t> tup;

        void set_tup(std::vector<value_t>);
};

class FunctionObject: public Object {
    public:
        NodeFunction *func;
        std::vector<vmcode_t> proc;
};

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
        Ast *expr_num(token_t token);
        Ast *expr_char(token_t token);
        Ast *expr_string(token_t token);
        Ast *expr_var(token_t token);
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
    PRINT_INT,
    PRINT_CHAR,
    PRINT_STR,
    PRINTLN,
    PRINTLN_INT,
    PRINTLN_CHAR,
    PRINTLN_STR,
    FORMAT,
    TYPEOF,
    LOAD,
    STORE,
    ISTORE,
    LISTSET,
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

enum class VALUE {
    Number,
    Char,
    String,
    Array,
    Bool,
    Null,
    Object,
    NONE,   //defalut
};

struct value_t {
    VALUE type;
    CTYPE ctype;
    union {
        int num;
        uint32_t unum;
        int64_t inum64;
        uint64_t unum64;
        char ch;
    };
    std::string str;
    ListObject listob;
    StringObject strob;
    TupleObject tupleob;
    FunctionObject funcob;

    value_t() {}
    value_t(int n): type(VALUE::Number), ctype(CTYPE::INT), num(n) {}
    value_t(char c): type(VALUE::Char), ctype(CTYPE::CHAR), ch(c) {}
    value_t(std::string s): type(VALUE::String), ctype(CTYPE::STRING), str(s) {}
    value_t(ListObject lo): type(VALUE::Object), ctype(CTYPE::LIST), listob(lo) {}
    value_t(StringObject so): type(VALUE::Object), ctype(CTYPE::STRING), strob(so) {}
    value_t(TupleObject to): type(VALUE::Object), ctype(CTYPE::TUPLE), tupleob(to) {}
    value_t(FunctionObject fo): type(VALUE::Object), ctype(CTYPE::FUNCTION), funcob(fo) {}
};

class Value {
    public:
        value_t value;
};

struct variable_t {
    NodeVariable *var;
    value_t val;

    variable_t(NodeVariable *_var): var(_var) {}
    variable_t(NodeVariable *_vr, value_t _val): var(_vr), val(_val) {}
};

struct vmcode_t {
    OPCODE type;
    VALUE vtype = VALUE::NONE;
    union {
        int num = 0;
        char ch;
    };
    std::string str;
    variable_t *var = nullptr;
    Method obmethod;
    unsigned int nfarg;
    std::vector<vmcode_t> proc;     //function

    size_t listsize;                //list
    int nline;

    vmcode_t() {}
    vmcode_t(OPCODE t, int l): type(t), nline(l) {}
    vmcode_t(OPCODE t, int v, int l): type(t), vtype(VALUE::Number), num(v), nline(l) {}
    vmcode_t(OPCODE t, char c, int l): type(t), vtype(VALUE::Char), ch(c), nline(l) {}
    vmcode_t(OPCODE t, std::string s, int l): type(t), vtype(VALUE::String), str(s), nline(l) {}
    vmcode_t(OPCODE t, size_t ls, int l): type(t), vtype(VALUE::Object), listsize(ls), nline(l) {}
    vmcode_t(OPCODE t, NodeVariable *vr, int l):
        type(t), var(new variable_t(vr)), nline(l) {}
    vmcode_t(OPCODE t, std::string s, unsigned int n, int l):
        type(t), str(s), nfarg(n), nline(l) {}  //format
    vmcode_t(OPCODE t, Method m, int l): type(t), obmethod(m), nline(l) {}
    vmcode_t(OPCODE t, std::vector<vmcode_t> p, int l): type(t), proc(p), nline(l) {}
};

class Program {
    public:
        void compile(Ast_v asts, Env e);
        void gen(Ast *ast);
        void show(vmcode_t &);
        std::vector<vmcode_t> vmcodes;
        std::map<std::string, int> lmap;
    private:
        void emit_head();
        void emit_num(Ast *ast);
        void emit_char(Ast *ast);
        void emit_string(Ast *ast);
        void emit_list(Ast *ast);
        void emit_listaccess(Ast *ast);
        void emit_tuple(Ast *ast);
        void emit_binop(Ast *ast);
        void emit_object_oprator(Ast *ast);
        void emit_dotop(Ast *ast);
        void emit_ternop(Ast *ast);
        void emit_pointer(NodeBinop *b);
        void emit_addr(Ast *ast);
        void emit_unaop(Ast *ast);
        void emit_if(Ast *ast);
        void emit_exprif(Ast *ast);
        void emit_for(Ast *ast);
        void emit_while(Ast *ast);
        void emit_return(Ast *ast);
        void emit_block(Ast *ast);
        void emit_print(Ast *ast);
        void emit_println(Ast *ast);
        void emit_format(Ast *ast);
        void emit_typeof(Ast *ast);
        void emit_assign(Ast *ast);
        void emit_store(Ast *ast);
        void emit_listaccess_store(Ast *ast);
        void emit_func_def(Ast *ast);
        void emit_func_call(Ast *ast);
        void emit_func_head(NodeFunction*);
        void emit_func_end();
        void emit_vardecl(Ast *ast);
        void emit_load(Ast *ast);
        void emit_cmp(std::string ord, NodeBinop *a);

        //VMcode push
        void vcpush(OPCODE);
        void vcpush(OPCODE, int);
        void vcpush(OPCODE, char);
        void vcpush(OPCODE, std::string);
        void vcpush(OPCODE, NodeVariable *);
        void vcpush(OPCODE, std::string, unsigned int);
        void vcpush(OPCODE, size_t);
        void vcpush(OPCODE, Method);
        void vcpush(OPCODE, std::vector<vmcode_t>);

        void opcode2str(OPCODE);
        std::string get_label();
        int get_lvar_size();
        int size;
        int get_var_pos(std::string name);
        int align(int n, int base);
        int nline = 0;
        std::string src;
        std::string endlabel;
        bool isused_var = false;
        bool isexpr = false;
        bool isinfunction = false;
        std::vector<vmcode_t> proc;     //function

        int labelnum = 1;

        Env env;
};

/*
 *  VM
 */

struct vmenv_t {
    std::map<NodeVariable *, value_t> vmap;
    vmenv_t *parent;

    vmenv_t() {}
};

class VMEnv {
    public:
        vmenv_t *cur;
        vmenv_t *make();
        vmenv_t *escape();
        std::map<NodeVariable *, value_t> getvmap();
    private:
};

class VM {
    public:
        int run(std::vector<vmcode_t> &code, std::map<std::string, int> &lmap);
        void exec(std::vector<vmcode_t> &);
    private:
        std::stack<value_t> s;
        std::stack<unsigned int> locs;
        std::map<NodeVariable *, value_t> gvmap;
        std::map<std::string, int> labelmap;
        void print(value_t &);
        unsigned int pc = 0;
        VMEnv env;
        //vmcode_t c = vmcode_t();
        int r, l, u;    //binary, unary
        value_t valstr;     //variable store
        std::string _format; int fpos; std::string bs; std::string ftop; //format
        std::string tyname;
        ListObject lsob; size_t lfcnt;    //list
        StringObject strob; //string
        FunctionObject tfuncob;     //function
        ListObject cmlsob; StringObject cmstob; TupleObject cmtupob; //call method
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
