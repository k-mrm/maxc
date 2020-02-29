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

MxcObject *list_get(MxcIterable *self, size_t idx) {
    MxcList *list = (MxcList *)self;
    if(self->length <= idx) {
        return NULL;
    }

    return list->elem[idx];
}

MxcObject *list_set(MxcIterable *self, size_t idx, MxcObject *a) {
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
        OBJIMPL(l->elem[i])->mark(l->elem[i]);
    }
}

MxcString *list_tostring(MxcObject *ob) {
    MxcList *l = (MxcList *)ob;
    if(ITERABLE(l)->length == 0) {
        return new_string_static("[]", 2);
    }

    MxcString *res = new_string_static("[", 1);
    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        if(i > 0) {
            str_cstr_append(res, ",", 1);
        }

        MxcString *elemstr = OBJIMPL(l->elem[i])->tostring(l->elem[i]);
        str_append(res, elemstr);
    }
    str_cstr_append(res, "]", 1);
    res->str[ITERABLE(res)->length - 1] = '\0';

    return res;
}

MxcObjImpl list_objimpl = {
    "list",
    list_tostring,
    list_dealloc,
    list_copy,
    list_gc_mark,
    list_get,
    list_set,
};
