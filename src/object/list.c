/* implementation of list object */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object/listobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcList *new_list(size_t size) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size);
    ITERABLE(ob)->length = size;

    return ob;
}

MxcObject *list_copy(MxcObject *l) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    memcpy(ob, l, sizeof(MxcList));

    MxcObject **old = ob->elem;
    ob->elem = malloc(sizeof(MxcObject *) * ITERABLE(ob)->length);
    for(size_t i = 0; i < ITERABLE(ob)->length; ++i) {
        ob->elem[i] = OBJIMPL(old[i])->copy(old[i]);
    }

    return (MxcObject *)ob;
}

MxcList *new_list_with_size(MxcInteger *size, MxcObject *init) {
    MxcList *ob = (MxcList *)Mxc_malloc(sizeof(MxcList));
    int64_t len = size->inum;
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->length = len;
    OBJIMPL(ob) = &list_objimpl;

    if(len < 0) {
        // error
        return NULL;
    }

    ob->elem = malloc(sizeof(MxcObject *) * size->inum);
    MxcObject **ptr = ob->elem;
    while(len--) {
        *ptr++ = init;
    }

    return ob;
}

MxcObject *list_get(MxcIterable *self, int64_t idx) {
    MxcList *list = (MxcList *)self;
    if(self->length <= idx) {
        return NULL;
    }

    return list->elem[idx];
}

MxcObject *list_set(MxcIterable *self, int64_t idx, MxcObject *a) {
    MxcList *list = (MxcList *)self;
    if(self->length <= idx) return NULL;
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
        GC_MARK(l->elem[i]);
    }
}

void list_guard(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;

    ob->gc_guard = 1;
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        GC_GUARD(l->elem[i]);
    }
}

void list_unguard(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;

    ob->gc_guard = 0;
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        GC_UNGUARD(l->elem[i]);
    }
}

MxcString *list_tostring(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;
    GC_GUARD(l);
    if(ITERABLE(l)->length == 0) {
        return new_string_static("[]", 2);
    }
    MxcString *res = new_string_static("[", 1);
    GC_GUARD(res);
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        if(i > 0) {
            str_cstr_append(res, ", ", 2);
        }

        MxcString *elemstr = OBJIMPL(l->elem[i])->tostring(l->elem[i]);
        str_append(res, elemstr);
    }
    GC_UNGUARD(l);
    str_cstr_append(res, "]", 1);

    GC_UNGUARD(res);
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
