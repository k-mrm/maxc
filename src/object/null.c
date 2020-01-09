/* implementation of null object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}

MxcObjImpl null_objimpl = {
    "null",
    null_tostring,
    0,
    0
};

NullObject MxcNull = {
    {
        &null_objimpl,
        1,  /* refcount */
        0
    }
};

