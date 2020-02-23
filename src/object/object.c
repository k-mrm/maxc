#include <stdlib.h>

#include "object/object.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcError *new_error(const char *msg) {
    MxcError *ob = (MxcError *)Mxc_malloc(sizeof(MxcError));
    ob->errmsg = msg;

    return ob;
}

MxcIStruct *new_struct(int nfield) {
    MxcIStruct *ob = (MxcIStruct *)Mxc_malloc(sizeof(MxcIStruct));
    ob->field = (MxcObject **)malloc(sizeof(MxcObject *) * nfield);
    return ob;
}

