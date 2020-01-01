/* implementation of null object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}

MxcObjImpl null_objimpl = {
    null_tostring,
    0,
    0
};

NullObject MxcNull = {
    {
        1,  /* refcount */
        &null_objimpl,
        0
    }
};

