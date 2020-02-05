#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include <stdbool.h>

#include "type.h"
#include "util.h"

struct NodeVariable;
typedef struct NodeVariable NodeVariable;
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
void varlist_push(Varlist *, NodeVariable *);
void varlist_mulpush(Varlist *, Varlist *);
int var_set_number(Varlist *);

/* Function */

typedef struct func_t {
    int fnkind;
    Varlist *args;
    Type *ftype;
    bool isbuiltin;
    bool isgeneric;
} func_t;

func_t New_Func_t(Type *, bool);
func_t New_Func_t_With_Varlist(Varlist *, Type *, bool);
func_t New_Func_t_With_Bltin(enum BLTINFN, Type *, bool);

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

Env *scope_make(Scope *);
Env *scope_escape(Scope *);
bool scope_isglobal(Scope);
int chk_var_conflict(Scope, NodeVariable *);

typedef struct FuncEnv {
    Env *current;
} FuncEnv;

Env *funcenv_make(FuncEnv *s);
Env *funcenv_escape(FuncEnv *s);
bool funcenv_isglobal(FuncEnv s);

#endif
