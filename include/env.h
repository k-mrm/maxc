#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include "maxc.h"
#include "type.h"
#include "util.h"

struct NodeVariable;
enum BLTINFN;

enum VARATTR {
    VARATTR_CONST = 0b0001,
    VARATTR_UNINIT = 0b0010,
};

typedef struct var_t {
    Type *type;
} var_t;

typedef struct Varlist {
    Vector *vars;
} Varlist;

Varlist *New_Varlist(void);
void varlist_push(Varlist *, struct NodeVariable *);
void varlist_mulpush(Varlist *, Varlist *);
void var_set_number(Varlist *);

// Function

typedef struct func_t {
    int fnkind;
    Varlist *args;
    Type *ftype;
    bool isbuiltin;
} func_t;

func_t New_Func_t(Type *);
func_t New_Func_t_With_Varlist(Varlist *, Type *);
func_t New_Func_t_With_Bltin(enum BLTINFN, Type *);

typedef struct Env {
    Varlist *vars;
    Vector *userdef_type;
    struct Env *parent;
    bool isglb;
} Env;

Env *New_Env(void);
Env *New_Env_Global(void);

typedef struct Scope {
    Env *current;
} Scope;

Env *scope_make(Scope *s);
Env *scope_escape(Scope *s);
bool scope_isglobal(Scope s);

typedef struct FuncEnv {
    Env *current;
} FuncEnv;

Env *funcenv_make(FuncEnv *s);
Env *funcenv_escape(FuncEnv *s);
bool funcenv_isglobal(FuncEnv s);

#endif
