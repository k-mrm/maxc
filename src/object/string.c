/* implementation of string object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

StringObject *new_stringobject(char *s) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ITERABLE(ob)->get = str_index;
    ITERABLE(ob)->set = str_index_set;
    ob->str = s;
    ITERABLE(ob)->length = ob->len = strlen(s);
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

void string_dealloc(MxcObject *s) {
    // TODO: `str` that allocated by malloc 
    Mxc_free(s);
}

MxcObject *str_index(MxcIterable *self, size_t idx) {
    StringObject *str = (StringObject *)self;
    if(str->len <= idx) return NULL;

    return (MxcObject *)new_charobject_ref(&str->str[idx]);
}

MxcObject *str_index_set(MxcIterable *self, size_t idx, MxcObject *a) {
    StringObject *str = (StringObject *)self;
    if(str->len <= idx) return NULL;
    str->str[idx] = a;

    return a;
}

StringObject *str_concat(StringObject *a, StringObject *b) {
    int len = a->len + b->len;
    char *res = malloc(sizeof(char) * len + 1);
    strcpy(res, a->str);
    strcat(res, b->str);

    DECREF(a);
    DECREF(b);

    return new_stringobject(res);
}

StringObject *string_tostring(MxcObject *ob) {
    return (StringObject *)ob;
}

MxcObjImpl string_objimpl = {
    "string",
    string_tostring,
    string_dealloc,
    0,
};
