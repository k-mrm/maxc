/* implementation of list object */
#include <stdio.h>

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

ListObject *new_listobject(size_t size) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->get = list_get;
    ITERABLE(ob)->set = list_set;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size);
    ITERABLE(ob)->length = ob->size = size;

    return ob;
}

MxcObject *list_copy(MxcObject *l) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    memcpy(ob, l, sizeof(ListObject));

    MxcObject **old = ob->elem;
    ob->elem = malloc(sizeof(MxcObject *) * ob->size);
    for(size_t i = 0; i < ob->size; ++i) {
        ob->elem[i] = OBJIMPL(old[i])->copy(old[i]);
    }

    return ob;
}

ListObject *new_listobject_size(IntObject *size, MxcObject *init) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->get = list_get;
    ITERABLE(ob)->set = list_set;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size->inum);
    for(int64_t i = 0; i < size->inum; i++) {
        ob->elem[i] = OBJIMPL(init)->copy(init);
    }
    ITERABLE(ob)->length = ob->size = size->inum;

    return ob;
}

MxcObject *list_get(MxcIterable *self, size_t idx) {
    ListObject *list = (ListObject *)self;
    if(list->size <= idx) {
        return NULL;
    }

    return list->elem[idx];
}

MxcObject *list_set(MxcIterable *self, size_t idx, MxcObject *a) {
    ListObject *list = (ListObject *)self;
    if(list->size <= idx) return NULL;
    list->elem[idx] = a;

    return a;
}

void list_dealloc(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    for(size_t i = 0; i < l->size; ++i) {
        Mxc_free(l->elem[i]);
    }
    free(l->elem);
    Mxc_free(ob);
}

StringObject *list_tostring(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    StringObject *res = new_stringobject("");

    for(size_t i = 0; i < l->size; ++i) {
        if(i > 0) {
            res = str_concat(res, new_stringobject(","));
        }
        StringObject *elemstr = OBJIMPL(l->elem[i])->tostring(l->elem[i]);
        res = str_concat(res, OBJIMPL(elemstr)->copy(elemstr));
    }
    char *result = malloc(sizeof(char *) * (res->len + 3));
    sprintf(result, "[%s]", res->str);
    DECREF((MxcObject *)res);

    return new_stringobject(result);
}

MxcObjImpl list_objimpl = {
    "list",
    list_tostring,
    list_dealloc,
    list_copy,
    0,
};
