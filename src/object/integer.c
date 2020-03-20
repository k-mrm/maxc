#include <string.h>
#include <stdlib.h>

#include "internal.h"
#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

MxcValue cstr2integer(char *, int, int);

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
    char *s = str;
    MxcInteger *ob = Mxc_malloc(sizeof(MxcInteger));
    ob->sign = sign;
    size_t slen = strlen(str);
    size_t didx = 1;
    // unsigned int *digs = ob->digit = malloc();
    int d;
    
    while(*s) {
        d = intern_ascii_to_numtable[(int)*s++];
        if(d < 0 || d >= base) {
            continue;
            // TODO: raise error
        }
        int i = 0;
    }

    return mval_obj(ob);
}
