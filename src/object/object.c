#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

CharObject *new_charobject(char c) {
    CharObject *ob = (CharObject *)Mxc_malloc(sizeof(CharObject));
    ob->ch = c;

    return ob;
}

ErrorObject *new_errorobject(const char *msg) {
    ErrorObject *ob = (ErrorObject *)Mxc_malloc(sizeof(ErrorObject));

    ob->errmsg = msg;

    return ob;
}

StructObject *new_structobject(int nfield) {
    StructObject *ob = (StructObject *)Mxc_malloc(sizeof(StructObject));
    ob->field = (MxcObject **)malloc(sizeof(MxcObject *) * nfield);

    return ob;
}

