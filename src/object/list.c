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
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size->inum);
    for(int64_t i = 0; i < size->inum; i++) {
        ob->elem[i] = OBJIMPL(init)->copy(init);
    }
    ITERABLE(ob)->length = size->inum;

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
    size_t new_len;
    MxcList *l = (MxcList *)ob;

    MxcString *res = new_string_static("", 0);

    for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
        if(i > 0) {
            res = str_concat(res, new_string_static(",", 1));
        }

        MxcString *elemstr = OBJIMPL(l->elem[i])->tostring(l->elem[i]);
        res = str_concat(res, elemstr);
    }
    new_len = ITERABLE(res)->length + 3;
    char *result = malloc(sizeof(char *) * new_len);
    sprintf(result, "[%s]", res->str);

    return new_string(result, new_len);
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
