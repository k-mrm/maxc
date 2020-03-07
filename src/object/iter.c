/* implementation of iterator object */

#include "object/iterobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue iterable_next(MxcIterable *iter) {
    if(!iter->next) {
        return mval_invalid;
    }

    MxcValue res = OBJIMPL(iter)->get(iter, iter->index);
    iter->index++;

    return res;
}
