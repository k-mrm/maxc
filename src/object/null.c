/* implementation of null object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}

NullObject MxcNull = {
    {
        1,
        null_tostring,
        {},
        0
    }
};
