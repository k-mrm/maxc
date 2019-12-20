#include "object/tostring.h"

StringObject *int_tostring(MxcObject *ob) {
    int num = ((IntObject *)ob)->inum;
    char *str = malloc(get_digit(num) * sizeof(char));
    sprintf(str, "%d", num);

    return new_stringobject(str);
}

StringObject *true_tostring(MxcObject *ob) {
    return "true";
}

StringObject *false_tostring(MxcObject *ob) {
    return "false";
}

StringObject *string_tostring(MxcObject *ob) {
    return (StringObject *)ob;
}

StringObject *null_tostring(MxcObject *ob) {
    return "null";
}
