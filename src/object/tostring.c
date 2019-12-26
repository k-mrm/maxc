#include "object/tostring.h"
#include "mem.h"

#include <inttypes.h>

StringObject *int_tostring(MxcObject *ob) {
    int64_t num = ((IntObject *)ob)->inum;
    char *str = malloc(get_digit(num) * sizeof(char));
    sprintf(str, "%lld", num);

    return new_stringobject(str);
}

StringObject *float_tostring(MxcObject *ob) {
    double f = ((FloatObject *)ob)->fnum;
    char *str = malloc(sizeof(char) * (get_digit((int)f) + 10));
    sprintf(str, "%lf", f);

    return new_stringobject(str);
} 

StringObject *true_tostring(MxcObject *ob) {
    return new_stringobject("true");
}

StringObject *false_tostring(MxcObject *ob) {
    return new_stringobject("false");
}

StringObject *string_tostring(MxcObject *ob) {
    return (StringObject *)ob;
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

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}

StringObject *userfn_tostring(MxcObject *ob) {
    char *s = malloc(sizeof(char *) * 64);
    sprintf(s, "<user-def function at %p>", ob);

    return new_stringobject(s);
}

StringObject *bltinfn_tostring(MxcObject *ob) {
    return new_stringobject("<builtin function>");
}
