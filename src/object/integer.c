#include <string.h>

#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

MxcValue new_integer(char *n, int base) {
    MxcInteger *ob = Mxc_malloc(sizeof(MxcInteger));
    ob->size = strlen(n);

    return mval_obj(ob);
}
