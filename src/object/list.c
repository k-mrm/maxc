/* implementation of list object */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object/listobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue new_list(size_t size) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcValue) * size);
    ITERABLE(ob)->length = size;

    return mval_obj(ob);
}

MxcValue list_copy(MxcObject *l) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    memcpy(ob, l, sizeof(MxcList));

    MxcValue *old = ob->elem;
    ob->elem = malloc(sizeof(MxcValue) * ITERABLE(ob)->length);
    for(size_t i = 0; i < ITERABLE(ob)->length; ++i) {
        ob->elem[i] = mval_copy(old[i]);
    }

    return mval_obj(ob);
}

MxcValue new_list_with_size(MxcValue size, MxcValue init) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    int64_t len = size.num;
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->length = len;
    OBJIMPL(ob) = &list_objimpl;

    if(len < 0) {
        // error
        return mval_invalid;
    }

    ob->elem = malloc(sizeof(MxcValue) * len);
    MxcValue *ptr = ob->elem;
    while(len--) {
        *ptr++ = init;
    }

    return mval_obj(ob);
}

MxcValue list_get(MxcIterable *self, int64_t idx) {
    MxcList *list = (MxcList *)self;
    if(ITERABLE(list)->length <= idx)
        return mval_invalid;

    return list->elem[idx];
}

MxcValue list_set(MxcIterable *self, int64_t idx, MxcValue a) {
    MxcList *list = (MxcList *)self;
    if(ITERABLE(list)->length <= idx)
        return mval_invalid;
    list->elem[idx] = a;

    return a;
}

void list_dealloc(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;

    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        DECREF(l->elem[i]);
    }
    free(l->elem);
    Mxc_free(ob);
}

void list_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    MxcList *l = (MxcList *)ob;

    ob->marked = 1;
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        mgc_mark(l->elem[i]);
    }
}

void list_guard(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;

    ob->gc_guard = 1;
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        mgc_guard(l->elem[i]);
    }
}

void list_unguard(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;

    ob->gc_guard = 0;
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        mgc_unguard(l->elem[i]);
    }
}

MxcValue list_tostring(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;
    if(ITERABLE(l)->length == 0) {
        return new_string_static("[]", 2);
    }
    GC_GUARD(l);
    MxcValue res = new_string_static("[", 1);
    mgc_guard(res);
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        if(i > 0) {
            str_cstr_append(res, ", ", 2);
        }

        MxcValue elemstr = mval2str(l->elem[i]);
        str_append(res, elemstr);
    }
    GC_UNGUARD(l);
    str_cstr_append(res, "]", 1);

    mgc_unguard(res);
    return res;
}

MxcObjImpl list_objimpl = {
    "list",
    list_tostring,
    list_dealloc,
    list_copy,
    list_gc_mark,
    list_guard,
    list_unguard,
    list_get,
    list_set,
};
