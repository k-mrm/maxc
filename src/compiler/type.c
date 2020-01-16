#include "type.h"
#include "error/error.h"
#include "maxc.h"

static bool is_primitive(Type *);

Type *New_Type_Function(Vector *fnarg, Type *fnret) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = CTYPE_FUNCTION;
    type->fnarg = fnarg;
    type->fnret = fnret;
    type->impl = TIMPL_SHOW;
    type->optional = false;
    type->isprimitive = false;

    size_t sum_len = 0;
    for(size_t i = 0; i < fnarg->len; ++i) {
        sum_len += ((Type *)fnarg->data[i])->tyname_len;
    }
    sum_len += fnret->tyname_len;

    type->tyname_len = sum_len + fnarg->len + 2;
    /*
     *  2 is -1 + 3
     *  fnarg->len - 1 == number of ','
     *  3 == '(', ')', ':'
     */
    type->tyname = malloc(type->tyname_len + 1);    /* fnarg->len - 1 == number of , */
    /*
     *  (int,int,int):int
     */
    strcpy(type->tyname, "(");
    for(size_t i = 0; i < fnarg->len; ++i) {
        if(i > 0) {
            strcat(type->tyname, ",");
        }
        strcat(type->tyname, ((Type *)fnarg->data[i])->tyname);
    }
    strcat(type->tyname, "):");
    strcat(type->tyname, fnret->tyname);

    return type;
}

Type *New_Type(enum CTYPE ty) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = ty;

    if(ty == CTYPE_FUNCTION) {
        type->fnarg = New_Vector();
        type->tyname = "function";
        type->impl = TIMPL_SHOW;
    }
    else if(ty == CTYPE_TUPLE) {
        type->tuple = New_Vector();
        type->tyname = "tuple";
        type->impl = 0;
    }
    else if(ty == CTYPE_ERROR) {
        type->err_msg = "";
        type->tyname = "error";
        type->impl = TIMPL_SHOW;
    }
    else if(ty == CTYPE_UNINFERRED) {
        type->tyname = "uninferred";
        type->impl = 0;
    }

    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_With_Ptr(Type *ty) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_LIST;
    type->tyname = malloc(ty->tyname_len + 3);
    type->tyname_len = sprintf(type->tyname, "[%s]", ty->tyname);
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
    type->name = type->tyname = str;
    type->optional = false;
    type->isprimitive = false;

    return type;
}

Type *New_Type_With_Struct(MxcStruct strct) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_STRUCT;
    type->name = strct.name;
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

bool is_variable(Type *t) {
    return t->type == CTYPE_VARIABLE;
}

bool is_iterable(Type *t) {
    if(!t)  return false;

    return t->impl & TIMPL_ITERABLE; 
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

/* type */

Type TypeNone = {
    CTYPE_NONE,         /* type */
    TIMPL_SHOW,         /* impl */
    "none",             /* tyname */
    4,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

Type TypeBool = {
    CTYPE_BOOL,         /* type */
    TIMPL_SHOW,         /* impl */
    "bool",             /* tyname */
    4,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

Type TypeInt = {
    CTYPE_INT,          /* type */
    TIMPL_SHOW,         /* impl */
    "int",              /* tyname */
    3,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
};

Type TypeFloat = {
    CTYPE_DOUBLE,       /* type */
    TIMPL_SHOW,         /* impl */
    "float",            /* tyname */
    5,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

Type TypeString = {
    CTYPE_STRING,       /* type */
    TIMPL_SHOW,         /* impl */
    "string",           /* tyname */
    6,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

Type TypeAny = {
    CTYPE_ANY,          /* type */
    0,                  /* impl */
    "any",              /* tyname */
    3,                  /* tyname_len */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

Type TypeAnyVararg = {
    CTYPE_ANY_VARARG,   /* type */
    0,                  /* impl */
    "any_vararg",       /* tyname */
    10,                 /* tyname */
    false,              /* optional */
    true,               /* isprimitive */
    {{0}},
}; 

