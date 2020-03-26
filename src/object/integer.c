#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "internal.h"
#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

MxcValue cstr2integer(char *, int, int);

#define max_fitin64bit_by(base) _max_fitin64bit_by[(base)-2]

static const uint64_t _max_fitin64bit_by[35] = {
    9223372036854775808, 12157665459056928801, 4611686018427387904,
    7450580596923828125, 4738381338321616896, 3909821048582988049,
    9223372036854775808, 12157665459056928801, 10000000000000000000,
    5559917313492231481, 2218611106740436992, 8650415919381337933,
    2177953337809371136, 6568408355712890625, 1152921504606846976,
    2862423051509815793, 6746640616477458432, 15181127029874798299,
    1638400000000000000, 3243919932521508681, 6221821273427820544,
    11592836324538749809, 876488338465357824, 1490116119384765625,
    2481152873203736576, 4052555153018976267, 6502111422497947648,
    10260628712958602189, 15943230000000000000, 787662783788549761,
    1152921504606846976, 1667889514952984961, 2386420683693101056,
    3379220508056640625, 4738381338321616896,
};

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
    digit_t *digs = ob->digit;
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
            d += (digit2_t)digs[i] * base;
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

static digit2_t digits_to_digit2(digit_t *digs, size_t ndig) {
    if(ndig == 2) return digs[0] | (digit2_t)digs[1] << 32;
    if(ndig == 1) return digs[0];
    return 0;
}

static void integer2str(MxcObject *self, int base, char *res) {
    MxcInteger *i = (MxcInteger *)self;
    char buf[sizeof(digit2_t) * CHAR_BIT + 1];
    char *end = buf + sizeof(buf);
    char *cur = end;
    digit2_t num = digits_to_digit2(i->digit, i->len);

    if(!res) {
        do {
            *--cur = mxc_36digits[num % base];
        } while(num /= base);
        size_t len = end - cur;
        res = malloc(sizeof(char) * len + 1);
        memcpy(res, cur, sizeof(char) * len);
    }
}

MxcValue integer_tostring(MxcObject *ob) {
    ;
}
