/* implementation of null object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

NullObject MxcNull = {
    {
        1,
        null_tostring
    }
};

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}
