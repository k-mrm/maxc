#include <string.h>

#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

MxcValue new_integer(char *str, int base) {
    char *s = str;
    MxcInteger *ob = Mxc_malloc(sizeof(MxcInteger));
    ob->sign = 1;
    if(*s == '-') {
        ob->sign = 0;
        s++;
    }
    else if(*s == '+') {
        s++;
    }

    while(*s == '0') s++;
    unsigned int elem = 0;

    return mval_obj(ob);
}
