#include "env.h"
#include "ast.h"
#include "error.h"

func_t New_Func_t(Type *f) {
    func_t fn;
    fn.ftype = f;
    fn.isbuiltin = false;

    return fn;
}

func_t New_Func_t_With_Varlist(Varlist *a, Type *f) {
    func_t fn = New_Func_t(f);

    fn.args = a;

    return fn;
}

func_t New_Func_t_With_Bltin(enum BLTINFN k, Type *f) {
    func_t fn = New_Func_t(f);

    fn.fnkind = k;
    fn.isbuiltin = true;

    return fn;
}

Env *New_Env() {
    Env *self = malloc(sizeof(Env));

    self->vars = New_Varlist();
    self->userdef_type = New_Vector();

    return self;
}

Env *New_Env_Global() {
    Env *self = New_Env();

    self->isglb = true;

    return self;
}

Env *scope_make(Scope *s) {
    Env *e = New_Env();
    e->parent = s->current;
    e->isglb = false;

    s->current = e;
    return s->current;
}

Env *scope_escape(Scope *s) {
    if(!s->current->isglb) {
        s->current = s->current->parent;
        return s->current;
    }

    return NULL;
}

bool scope_isglobal(Scope s) { return s.current->isglb; }

Env *funcenv_make(FuncEnv *s) {
    Env *e = New_Env();
    e->parent = s->current;
    e->isglb = false;

    s->current = e;
    return s->current;
}

Env *funcenv_escape(FuncEnv *s) {
    if(!s->current->isglb) {
        s->current = s->current->parent;
        return s->current;
    }

    return NULL;
}

bool funcenv_isglobal(FuncEnv s) { return s.current->isglb; }

Varlist *New_Varlist() {
    Varlist *self = malloc(sizeof(Varlist));

    self->vars = New_Vector();

    return self;
}

void varlist_push(Varlist *self, NodeVariable *v) { vec_push(self->vars, v); }

void varlist_mulpush(Varlist *self, Varlist *v) {
    for(int i = 0; i < v->vars->len; ++i) {
        vec_push(self->vars, (NodeVariable *)v->vars->data[i]);
    }
}

void varlist_show(Varlist *self) {
    debug("varlist show: ");
    for(int i = 0; i < self->vars->len; ++i) {
        printf("%s ", ((NodeVariable *)self->vars->data[i])->name);
    }
    puts("");
}

void var_set_number(Varlist *self) {
    for(int i = 0; i < self->vars->len; ++i) {
        ((NodeVariable *)self->vars->data[i])->vid = i;
    }
}