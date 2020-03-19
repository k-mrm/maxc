#include <string.h>

#include "internal.h"
#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

MxcValue new_integer(char *str, int base) {
    char *s = str;
    int sign = SIGN_PLUS;
    if(*s == '-') {
        sign = SIGN_MINUS;
        s++;
    }
    else if(*s == '+') {
        s++;
    }
    while(*s == '0') s++;
    if(*s == '\0') {
        return mval_int(0);
    }

    return cstr2integer(s, base, sign);
}

MxcValue cstr2integer(char *str, int base, int sign) {
    MxcInteger *ob = Mxc_malloc(sizeof(MxcInteger));

    return mval_obj(ob);
}
