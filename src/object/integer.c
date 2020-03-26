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
    size_t dslen = 1;
    ob->digit = malloc(sizeof(unsigned int) * 50);/* TODO: really 50? */
    unsigned int *digs = ob->digit;
    uint64_t d;
    int i = 0;
    
    while(*s) {
        d = intern_ascii_to_numtable[(int)*s++];
        if(d < 0 || d >= base) {
            continue;
            // TODO: raise error
        }
        i = 0;

redo:
        while(i < dslen) {
            d += (uint64_t)digs[i] * base;
            digs[i++] = d & DIGIT_MAX;
            d = d >> DIGIT_POW;
        }
        if(d) {   /* carry occurs */
            ++dslen;
            goto redo;
        }
    }

    ob->len = dslen;

    for(int j = 0; j < dslen; ++j) {
        printf("%d is %lu\n", j, digs[j]);
    }

    return mval_obj(ob);
}

void integer2str(MxcObject *self, int base) {
    ;
}

MxcValue integer_tostring(MxcObject *ob) {
    MxcInteger *i = (MxcInteger *)ob;
    size_t ilen = i->len;

    for(int i = 0; i < ilen - 1; i++) {
        ;
    }
}
