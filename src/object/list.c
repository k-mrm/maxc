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
    OBJIMPL(ob) = &list_objimpl;

    ob->elem = malloc(sizeof(MxcObject *) * size);
    ITERABLE(ob)->length = ob->size = size;

    return ob;
}

MxcObject *list_get(MxcIterable *self, size_t idx) {
    ListObject *list = (ListObject *)self;

    if(list->size <= idx) {
        return NULL;
    }

    return list->elem[idx];
}

void list_dealloc(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    free(l->elem);
    free(l);
}

StringObject *list_tostring(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    StringObject *res = new_stringobject("");

    for(int i = 0; i < l->size; ++i) {
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
    0,
};
