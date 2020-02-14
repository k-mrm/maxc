#include <stdarg.h>

#include "module.h"
#include "util.h"

void set_bltin_func_type(NodeVariable *self, Type *ret, ...) {
    Type *fntype;
    Vector *args = New_Vector();
    va_list argva;
    va_start(argva, ret);

    for(Type *t = va_arg(argva, Type *); t; t = va_arg(argva, Type *)) {
        vec_push(args, t);
    }

    fntype = New_Type_Function(args, ret);
    self->ctype = fntype;
}

void set_bltin_var_type(NodeVariable *self, Type *ty) {
    self->ctype = ty;
}
