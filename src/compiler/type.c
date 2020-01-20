#include "type.h"
#include "error/error.h"
#include "maxc.h"

Type *New_Type(enum CTYPE ty) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = ty;

    if(ty == CTYPE_TUPLE) {
        type->tuple = New_Vector();
        // type->tyname = "tuple";
        type->impl = 0;
    }
    else if(ty == CTYPE_ERROR) {
        type->err_msg = "";
        // type->tyname = "error";
        type->impl = TIMPL_SHOW;
    }
    else if(ty == CTYPE_UNINFERRED) {
        type->tostring = uninferty_tostring;
        type->impl = 0;
    }

    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_Function(Vector *fnarg, Type *fnret) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = CTYPE_FUNCTION;
    type->tostring = functy_tostring;
    type->fnarg = fnarg;
    type->fnret = fnret;
    type->impl = TIMPL_SHOW;
    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_With_Ptr(Type *ty) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_LIST;
    type->tostring = listty_tostring;
    type->ptr = ty;
    type->impl = TIMPL_SHOW | TIMPL_ITERABLE; 
    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_Unsolved(char *str) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_UNDEFINED;
    type->impl = 0;
    type->name = str;
    type->tostring = unsolvety_tostring;
    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_With_Struct(MxcStruct strct) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_STRUCT;
    type->name = strct.name;
    type->tostring = structty_tostring;
    type->impl = 0;
    type->strct = strct;
    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_Variable(char *name) {
    Type *type = malloc(sizeof(Type));

    static int id = 0;

    type->type = CTYPE_VARIABLE;
    type->id = id++;
    type->instance = NULL;
    type->optional = false;
    type->type_name = name;

    return type;
}

bool type_is(Type *self, enum CTYPE ty) {
    if(!self) return false;

    return self->type == ty;
}

bool is_number(Type *t) {
    return t && (t->type == CTYPE_INT || 
                 t->type == CTYPE_DOUBLE);
}

bool is_variable(Type *t) {
    return t && (t->type == CTYPE_VARIABLE);
}

bool is_iterable(Type *t) {
    return t && (t->impl & TIMPL_ITERABLE); 
}

Type *instantiate(Type *ty) {
    if(is_variable(ty)) {
        if(ty->instance != NULL) {
            ty->instance = instantiate(ty->instance);
            return ty->instance;
        }
    }

    return ty;
}

bool same_type(Type *t1, Type *t2) {
    if(!t1 || !t2) return false;

    t1 = instantiate(t1);
    t2 = instantiate(t2);

    if(t1->isprimitive) {
        return t1->type == t2->type;
    }
    else if(t1->type == CTYPE_STRUCT &&
            t2->type == CTYPE_STRUCT) {
        if(strncmp(t1->strct.name, t2->strct.name, strlen(t1->strct.name)) == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    return false;
}

MxcOptional *New_MxcOptional(Type *base) {
    if(!base)   return NULL;

    MxcOptional *new = malloc(sizeof(MxcOptional));
    new->parent = *New_Type(CTYPE_OPTIONAL);
    new->base = base;
    ((Type *)new)->optional = true;

    new->err = New_Type(CTYPE_ERROR);

    return new;
}

/* type tostring */

char *nonety_tostring(Type *ty) { (void)ty; return "none"; }

char *boolty_tostring(Type *ty) { (void)ty; return "bool"; } 

char *intty_tostring(Type *ty) { (void)ty; return "int"; } 

char *floatty_tostring(Type *ty) { (void)ty; return "float"; } 

char *stringty_tostring(Type *ty) { (void)ty; return "string"; }

char *anyty_tostring(Type *ty) { (void)ty; return "any"; }

char *any_varargty_tostring(Type *ty) { (void)ty; return "any_vararg"; }

char *structty_tostring(Type *ty) { return ty->name; }

char *unsolvety_tostring(Type *ty) { return ty->name; }

char *listty_tostring(Type *ty) {
    char *name = malloc(strlen(ty->ptr->tostring(ty->ptr)) + 3);
    sprintf(name, "[%s]", ty->ptr->tostring(ty->ptr));

    return name;
}

char *functy_tostring(Type *ty) {
    size_t sum_len = 0;

    if(!ty->fnarg || !ty->fnret) {
        return "";
    }

    for(size_t i = 0; i < ty->fnarg->len; ++i) {
        sum_len += strlen(((Type *)ty->fnarg->data[i])->tostring((Type *)ty->fnarg->data[i]));
    }
    sum_len += strlen(ty->fnret->tostring(ty->fnret));

    sum_len += ty->fnarg->len + 2;
    /*
     *  2 is -1 + 3
     *  fnarg->len - 1 == number of ','
     *  3 == '(', ')', ':'
     */
    char *name = malloc(sum_len + 1);
    /*
     *  (int,int,int):int
     */
    strcpy(name, "(");
    for(size_t i = 0; i < ty->fnarg->len; ++i) {
        if(i > 0) {
            strcat(name, ",");
        }
        strcat(name, ((Type *)ty->fnarg->data[i])->tostring((Type *)ty->fnarg->data[i]));
    }
    strcat(name, "):");
    strcat(name, ty->fnret->tostring(ty->fnret));
    
    return name;
}

char *uninferty_tostring(Type *ty) {
    INTERN_UNUSE(ty);
    return "uninferred";
}

/* type */

Type TypeNone = {
    .type = CTYPE_NONE,
    .impl = TIMPL_SHOW,
    .tostring = nonety_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

Type TypeBool = {
    .type = CTYPE_BOOL,
    .impl = TIMPL_SHOW,
    .tostring = boolty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

Type TypeInt = {
    .type = CTYPE_INT,
    .impl = TIMPL_SHOW,
    .tostring = intty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
};

Type TypeFloat = {
    .type = CTYPE_DOUBLE,
    .impl = TIMPL_SHOW,
    .tostring = floatty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

Type TypeString = {
    .type = CTYPE_STRING,
    .impl = TIMPL_SHOW,
    .tostring = stringty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

Type TypeAny = {
    .type = CTYPE_ANY,
    .impl = 0,
    .tostring = anyty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

Type TypeAnyVararg = {
    .type = CTYPE_ANY_VARARG,
    .impl = 0,
    .tostring = any_varargty_tostring,
    .optional = false,
    .isprimitive = true,
    {{0}},
}; 

