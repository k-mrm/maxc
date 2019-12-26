/* implementation of string object */

#include "object/object.h"
#include "object/tostring.h"
#include "error.h"
#include "mem.h"
#include "vm.h"

StringObject *new_stringobject(const char *s) {
    StringObject *ob = (StringObject *)Mxc_malloc(sizeof(StringObject));
    ob->str = s;
    ((MxcObject *)ob)->tostring = string_tostring;

    return ob;
}

StringObject *str_concat(StringObject *a, StringObject *b) {
    int len = strlen(a->str) + strlen(b->str);

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
