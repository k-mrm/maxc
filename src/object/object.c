#include <stdlib.h>

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

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

