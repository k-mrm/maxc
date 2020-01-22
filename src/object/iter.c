/* implementation of iterator object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcObject *iterable_next(MxcIterable *iter) {
    if(!iter->next) {
        return NULL;
    }

    MxcObject *res = iter->get((MxcObject *)iter, iter->index);
    iter->index++;

    return res;
}
