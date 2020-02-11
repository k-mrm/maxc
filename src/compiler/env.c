#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "ast.h"
#include "error/error.h"
#include "builtins.h"

func_t New_Func_t(bool isgeneric) {
    func_t fn;
    fn.isbuiltin = false;
    fn.isgeneric = isgeneric;

    return fn;
}

func_t New_Func_t_With_Varlist(Varlist *a, bool isgeneric) {
    func_t fn = New_Func_t(isgeneric);
    fn.args = a;

    return fn;
}

func_t New_Func_t_With_Bltin(enum BLTINFN k, bool isgeneric) {
    func_t fn = New_Func_t(isgeneric);
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
    for(int i = 0; i < s->current->vars->vars->len; i++) {
        NodeVariable *v = (NodeVariable *)s->current->vars->vars->data[i]; 

        /*
        if(!v->used && !v->isbuiltin) {
            warn("unused variable: %s", v->name);
        }*/
    }

    if(!s->current->isglb) {
        s->current = s->current->parent;

        return s->current;
    }

    return NULL;
}

bool scope_isglobal(Scope s) { return s.current->isglb; }

int chk_var_conflict(Scope s, NodeVariable *v) {
    Vector *vars = s.current->vars->vars;

    for(int i = 0; i < vars->len; ++i) {
        NodeVariable *cur = (NodeVariable *)vars->data[i];

        if(strcmp(cur->name, v->name) == 0) {
            return 1;
        }
    }

    return 0;
} 

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
        NodeVariable *cur = (NodeVariable *)self->vars->data[i];
        if(!cur) {
            printf("%s ", "null");
            continue;
        }

        do {
            printf("%s ", cur->name);
        } while((cur = cur->next));
    }
    puts("");
}

size_t var_set_number(Varlist *self) {
    size_t id = 0;
    for(size_t i = 0; i < self->vars->len; ++i) {
        NodeVariable *cur = (NodeVariable *)self->vars->data[i];
        do {
            cur->vid = id++;
        } while((cur = cur->next));
    }

    return id;
}
