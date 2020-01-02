#include "type.h"
#include "error/error.h"
#include "maxc.h"

Type *mxcty_none;
Type *mxcty_bool;
Type *mxcty_string;
Type *mxcty_int;
Type *mxcty_float;

static bool is_primitive(Type *);

void type_init() {
    mxcty_none = New_Type(CTYPE_NONE);
    mxcty_bool = New_Type(CTYPE_BOOL);
    mxcty_string = New_Type(CTYPE_STRING);
    mxcty_int = New_Type(CTYPE_INT);
    mxcty_float = New_Type(CTYPE_DOUBLE);
}

const char *typedump(Type *self) {
    if(!self) return "NULL";

    switch(self->type) {
    case CTYPE_INT:
        return "int";
    case CTYPE_BOOL:
        return "bool";
    case CTYPE_CHAR:
        return "char";
    case CTYPE_STRING:
        return "string";
    case CTYPE_DOUBLE:
        return "float";
    case CTYPE_LIST:
        return "list";
    case CTYPE_TUPLE:
        return "tuple";
    case CTYPE_FUNCTION:
        return "function";
    case CTYPE_UNINFERRED:
        return "uninferred type";
    case CTYPE_NONE:
        return "none";
    case CTYPE_ANY_VARARG:
        return "any_arg";
    case CTYPE_ANY:
        return "any";
    case CTYPE_STRUCT:
        return self->strct.name;
    case CTYPE_UNDEFINED:
        return "undefined";
    case CTYPE_ERROR:
        return "error";
    case CTYPE_OPTIONAL:
        return "optional";
    default:
        error("??????: in typedump: %d", self->type);
        return "!UNEXPECTED!";
    }
}

Type *New_Type(enum CTYPE ty) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = ty;
    type->impl = 0;

    if(ty == CTYPE_FUNCTION) {
        type->fnarg = New_Vector();
    }
    else if(ty == CTYPE_TUPLE) {
        type->tuple = New_Vector();
    }
    else if(ty == CTYPE_ERROR) {
        type->err_msg = "";
    }

    type->optional = false;

    if(is_primitive(type)) {
        type->impl |= TIMPL_SHOW; 
    }

    return type;
}

Type *New_Type_With_Ptr(Type *ty) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_LIST;
    type->ptr = ty;
    type->optional = false;
    type->impl |= TIMPL_ITERABLE | TIMPL_SHOW;

    return type;
}

Type *New_Type_With_Str(char *str) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_UNDEFINED;
    type->name = str;
    type->optional = false;
    type->impl = 0;

    return type;
}

Type *New_Type_With_Struct(MxcStruct strct) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_STRUCT;
    type->strct = strct;
    type->optional = false;
    type->impl = 0;

    return type;
}

bool type_is(Type *self, enum CTYPE ty) {
    if(!self) return false;

    return self->type == ty;
}

static bool is_primitive(Type *t) {
    if(!t)  return false;

    switch(t->type) {
    case CTYPE_NONE:
    case CTYPE_INT:
    case CTYPE_DOUBLE:
    case CTYPE_BOOL:
    case CTYPE_STRING:
        return true;
    default:
        return false;
    }
}

bool is_iterable(Type *t) {
    if(!t)  return false;

    return t->impl & TIMPL_ITERABLE; 
}

bool same_type(Type *t1, Type *t2) {
    if(!t1 || !t2) return false;

    if(is_primitive(t1)) {
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
