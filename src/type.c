#include "type.h"
#include "error.h"
#include "maxc.h"

Type *mxcty_none;
Type *mxcty_bool;
Type *mxcty_string;
Type *mxcty_int;
Type *mxcty_float;

void type_init() {
    mxcty_none = New_Type(CTYPE_NONE);
    mxcty_bool = New_Type(CTYPE_BOOL);
    mxcty_string = New_Type(CTYPE_STRING);
    mxcty_int = New_Type(CTYPE_INT);
    mxcty_float = New_Type(CTYPE_DOUBLE);
}

const char *typedump(Type *self) {
    if(!self) {
        error("nullptr in typedump");
        return "NULL";
    }
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
        return "struct";
    case CTYPE_UNDEFINED:
        return "undefined";
    case CTYPE_ERROR:
        return "error";
    default:
        error("??????: in typedump: %d", self->type);
        return "!UNEXPECTED!";
    }
}

Type *New_Type(enum CTYPE ty) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->type = ty;

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

    return type;
}

Type *New_Type_With_Ptr(Type *ty) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_LIST;
    type->ptr = ty;
    type->optional = false;

    return type;
}

Type *New_Type_With_Str(char *str) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_UNDEFINED;
    type->name = str;
    type->optional = false;

    return type;
}

Type *New_Type_With_Struct(MxcStruct strct) {
    Type *type = malloc(sizeof(Type));
    type->type = CTYPE_STRUCT;
    type->strct = strct;
    type->optional = false;

    return type;
}

bool type_is(Type *self, enum CTYPE ty) { return self->type == ty; }

MxcOptional *New_MxcOptional(Type *base) {
    if(base == NULL) {
        return NULL;
    }

    MxcOptional *new = malloc(sizeof(MxcOptional));

    new->base = *base;

    ((Type *)new)->optional = true;

    new->err = New_Type(CTYPE_ERROR);

    return new;
}
