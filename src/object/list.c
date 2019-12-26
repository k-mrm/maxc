/* implementation of list object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

MxcObject *list_get(MxcObject *self, size_t idx) {
    ListObject *list = (ListObject *)self;

    if(list->size <= idx) {
        runtime_err("Index out of range");
    }

    return list->elem[idx];
}

ListObject *new_listobject(size_t size) {
    ListObject *ob = (ListObject *)Mxc_malloc(sizeof(ListObject));
    ((MxcIterable *)ob)->index = 0;
    ((MxcIterable *)ob)->next = NULL;
    ((MxcIterable *)ob)->get = list_get;
    ((MxcObject *)ob)->tostring = list_tostring;

    ob->elem = malloc(sizeof(MxcObject *) * size);
    ob->size = size;

    return ob;
}

StringObject *list_tostring(MxcObject *ob) {
    ListObject *l = (ListObject *)ob;

    StringObject *res = new_stringobject("");

    for(int i = 0; i < l->size; ++i) {
        if(i > 0) {
            res = str_concat(res, new_stringobject(","));
        }
        res = str_concat(res, l->elem[i]->tostring(l->elem[i]));
    }
    char *result = malloc(sizeof(char *) * (strlen(res->str) + 3));
    sprintf(result, "[%s]", res->str);
    DECREF(res);

    return new_stringobject(result);
}
