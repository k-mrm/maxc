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

ListObject *new_listobject_size(IntObject *size, MxcObject *init) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->get = list_get;
    ITERABLE(ob)->set = list_set;
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size->inum);
    for(int64_t i = 0; i < size->inum; i++) {
        MxcObject *n = Mxc_malloc(OBJIMPL(init)->size_of);
        memcpy(n, init, OBJIMPL(init)->size_of);
        ob->elem[i] = n;
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

    free(l->elem);
    free(l);
}

StringObject *list_tostring(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    StringObject *res = new_stringobject("");

    for(size_t i = 0; i < l->size; ++i) {
        if(i > 0) {
            res = str_concat(res, new_stringobject(","));
        }
        res = str_concat(res, OBJIMPL(l->elem[i])->tostring(l->elem[i]));
    }
    char *result = malloc(sizeof(char *) * (res->len + 3));
    sprintf(result, "[%s]", res->str);
    DECREF(res);

    return new_stringobject(result);
}

MxcObjImpl list_objimpl = {
    "list",
    list_tostring,
    list_dealloc,
    sizeof(ListObject),
    0,
};
