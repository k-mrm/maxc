#include "object/tostring.h"

StringObject *int_tostring(MxcObject *ob) {
    int64_t num = ((IntObject *)ob)->inum;
    char *str = malloc(get_digit(num) * sizeof(char));
    sprintf(str, "%ld", num);

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

StringObject *null_tostring(MxcObject *ob) {
    return new_stringobject("null");
}

StringObject *bltinfn_tostring(MxcObject *ob) {
    return new_stringobject("<builtin function>");
}
