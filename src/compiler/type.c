#include "type.h"
#include "error/error.h"
#include "maxc.h"

static bool is_primitive(Type *);

Type *New_Type(enum CTYPE ty) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = ty;

    if(ty == CTYPE_FUNCTION) {
        type->fnarg = New_Vector();
        type->tyname = "function";
    }
    else if(ty == CTYPE_TUPLE) {
        type->tuple = New_Vector();
        type->tyname = "tuple";
    }
    else if(ty == CTYPE_ERROR) {
        type->err_msg = "";
        type->tyname = "error";
    }

    type->optional = false;

    return type;
}

Type *New_Type_With_Ptr(Type *ty) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_LIST;
    type->tyname = malloc(strlen(ty->tyname) + 3);
    sprintf(type->tyname, "[%s]", ty->tyname);
    type->info = &tinfo_list;
    type->ptr = ty;
    type->optional = false;

    return type;
}

Type *New_Type_Unsolved(char *str) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_UNDEFINED;
    type->info = &tinfo_unsolved;
    type->name = type->tyname = str;
    type->optional = false;

    return type;
}

Type *New_Type_With_Struct(MxcStruct strct) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_STRUCT;
    type->name = strct.name;
    type->strct = strct;
    type->optional = false;

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

    return t->info->impl & TIMPL_ITERABLE; 
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

    if(t1->info->isprimitive) {
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
    &tinfo_none,        /* info */
    "none",             /* tyname */
    false,              /* optional */
    {{0}},
}; 

Type TypeBool = {
    CTYPE_BOOL,         /* type */
    &tinfo_boolean,     /* info */
    "bool",             /* tyname */
    false,              /* optional */
    {{0}},
}; 

Type TypeInt = {
    CTYPE_INT,          /* type */
    &tinfo_integer,     /* info */
    "int",              /* tyname */
    false,              /* optional */
    {{0}}
};

Type TypeFloat = {
    CTYPE_DOUBLE,       /* type */
    &tinfo_float,       /* info */
    "float",            /* tyname */
    false,              /* optional */
    {{0}},
}; 

Type TypeString = {
    CTYPE_STRING,       /* type */
    &tinfo_string,      /* info */
    "string",           /* tyname */
    false,              /* optional */
    {{0}},
}; 

Type TypeAny = {
    CTYPE_ANY,          /* type */
    &tinfo_any,         /* info */
    "any",              /* tyname */
    false,              /* optional */
    {{0}},
}; 

Type TypeAnyVararg = {
    CTYPE_ANY_VARARG,   /* type */
    &tinfo_any_vararg,  /* info */
    "any_vararg",       /* tyname */
    false,              /* optional */
    {{0}},
}; 

/* type information */

TypeInfo tinfo_none = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_integer = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_boolean = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_float = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_string = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_list = {
    TIMPL_SHOW | TIMPL_ITERABLE,
    false
};

TypeInfo tinfo_function = {
    TIMPL_SHOW,
    false
};

TypeInfo tinfo_struct = {
    0,
    false
};

TypeInfo tinfo_unsolved = {
    0,
    false
};

TypeInfo tinfo_any = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_any_vararg = {
    TIMPL_SHOW,
    true
};
