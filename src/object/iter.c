/* implementation of iterator object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

MxcObject *iterable_next(MxcIterable *iter) {
    if(!iter->next) {
        return NULL;
    }

    MxcObject *res = iter->get(iter, iter->index);
    iter->index++;

    return res;
}
