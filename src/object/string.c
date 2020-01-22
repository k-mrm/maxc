/* implementation of string object */

#include "object/object.h"
#include "object/tostring.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

StringObject *new_stringobject(const char *s) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ((MxcIterable *)ob)->index = 0;
    ((MxcIterable *)ob)->next = NULL;
    ((MxcIterable *)ob)->get = str_index;
    ob->str = s;
    ((MxcIterable *)ob)->length = ob->len = strlen(s);
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

void string_dealloc(MxcObject *s) {
    // TODO: `str` that allocated by malloc 
    free(s);
}

MxcObject *str_index(MxcObject *self, size_t idx) {
    StringObject *str = (StringObject *)self;

    if(str->len <= idx) return NULL;

    return (MxcObject *)new_charobject(str->str[idx]);
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
