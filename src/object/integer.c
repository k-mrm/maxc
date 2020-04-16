#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>

#include "internal.h"
#include "object/object.h"
#include "object/integerobject.h"
#include "mem.h"

static digit_t darylshift(digit_t *, digit_t *, size_t, int);
static void daryrshift(digit_t *, digit_t *, size_t, int);
static MxcValue cstr2integer(char *, int, int);
static void digit2_t_to_dary(digit_t *, digit2_t);
static MxcValue integer_norm(MxcInteger *);
static digit_t integer_divrem1(MxcInteger *, digit_t, MxcValue *);

#define PLUS(v) (obig(v)->sign)
#define MINUS(v) (!obig(v)->sign)
#define log2to32power(base) _log2to32power[(base)-2]
#define log2to32power_inv(base) _log2to32power_inv[(base)-2]
#define maxpow_fitin64bit_by(base) _maxpow_fitin64bit_by[(base)-2]

/* log_2**32(base) */
static const double _log2to32power[35] = {
    0.03125, 0.049530078147536134, 0.0625, 0.07256025296523007,
    0.08078007814753613, 0.08772984131430013, 0.09375, 0.09906015629507227,
    0.10381025296523008, 0.10810723808241555, 0.11203007814753614,
    0.11563874119190913, 0.11897984131430012, 0.12209033111276621,
    0.125, 0.12773321378907312, 0.13031015629507225, 0.13274773479511204,
    0.13506025296523008, 0.13725991946183627, 0.13935723808241554,
    0.14136131112678166, 0.14328007814753616, 0.14512050593046014,
    0.14688874119190914, 0.1485902344426084, 0.15022984131430012,
    0.15181190609773665, 0.1533403311127662, 0.15481863469958987,
    0.15625, 0.15763731622995167, 0.15898321378907312, 0.1602900942795302,
    0.16156015629507225,
};

static const double _log2to32power_inv[35] = {
    32.0, 20.189752114286637, 16.0, 13.781649858348578,
    12.379289831505332, 11.39862998745671, 10.666666666666666,
    10.094876057143319, 9.632959861247397, 9.25007444217241,
    8.926174260836154, 8.647620941674232, 8.404785121190194,
    8.190656793914096, 8.0, 7.8288173477832315, 7.673998930180207,
    7.533085227732424, 7.404102821112293, 7.285447958302496,
    7.175802374962414, 7.07407134264012, 6.979337343537009,
    6.890824929174289, 6.80787371370762, 6.729917371428878,
    6.6564671256483035, 6.5870986387339014, 6.521441506896198,
    6.459170770627194, 6.4, 6.343675621457937, 6.289972231450323,
    6.238688700601162, 6.189644915752666
};

static const uint64_t _maxpow_fitin64bit_by[35] = {
    9223372036854775808u, 12157665459056928801u, 4611686018427387904u,
    7450580596923828125u, 4738381338321616896u, 3909821048582988049u,
    9223372036854775808u, 12157665459056928801u, 10000000000000000000u,
    5559917313492231481u, 2218611106740436992u, 8650415919381337933u,
    2177953337809371136u, 6568408355712890625u, 1152921504606846976u,
    2862423051509815793u, 6746640616477458432u, 15181127029874798299u,
    1638400000000000000u, 3243919932521508681u, 6221821273427820544u,
    11592836324538749809u, 876488338465357824u, 1490116119384765625u,
    2481152873203736576u, 4052555153018976267u, 6502111422497947648u,
    10260628712958602189u, 15943230000000000000u, 787662783788549761u,
    1152921504606846976u, 1667889514952984961u, 2386420683693101056u,
    3379220508056640625u, 4738381338321616896u,
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

static MxcInteger *new_integer_capa(size_t capa, int sign) {
    MxcInteger *ob = Mxc_malloc(sizeof(MxcInteger));
    ob->digit = malloc(sizeof(digit_t) * capa);
    memset(ob->digit, 0, sizeof(digit_t) * capa);
    ob->len = capa;
    ob->sign = sign;
    OBJIMPL(ob) = &integer_objimpl; 

    return ob;
}

MxcValue int_to_integer(int64_t n) {
    int sign = n >= 0;
    MxcInteger *a = new_integer_capa(2, sign);
    uint64_t un = sign ? n : (uint64_t)(-(n + 1)) + 1; 
    digit2_t_to_dary(a->digit, un);
    return integer_norm(a);
}

MxcValue uint_to_integer(uint64_t n) {
    MxcInteger *a = new_integer_capa(2, SIGN_PLUS);
    digit2_t_to_dary(a->digit, n);
    return integer_norm(a);
}

static MxcValue cstr2integer(char *str, int base, int sign) {
    char *s = str;
    size_t slen = strlen(str);
    size_t n = (size_t)ceil(slen * log2to32power(base));
    MxcInteger *ob = new_integer_capa(n, sign);
    size_t dslen = 1;
    digit_t *digs = ob->digit;
    digit2_t d;
    unsigned int i = 0;
    
    while(*s) {
        d = intern_ascii_to_numtable[(int)*s++];
        if(d >= (unsigned int)base) {
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

    return integer_norm(ob);
}

void integer_dealloc(MxcObject *ob) {
    MxcInteger *i = (MxcInteger *)ob;
    free(i->digit);
    Mxc_free(ob);
}

MxcValue integer_copy(MxcObject *ob) {
    MxcInteger *n = (MxcInteger *)Mxc_malloc(sizeof(MxcInteger));
    MxcInteger *old = (MxcInteger *)ob;
    *n = *old; 

    digit_t *olds = n->digit;
    n->digit = malloc(sizeof(digit_t) * (n->len));
    memcpy(n->digit, olds, sizeof(digit_t) * n->len);

    return mval_obj(n);
}

void integer_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

void integer_guard(MxcObject *ob) {
    ob->gc_guard = 1;
}

void integer_unguard(MxcObject *ob) {
    ob->gc_guard = 0;
}

static MxcValue integer_norm(MxcInteger *x) {
    size_t i = x->len;
    size_t tmp = i;
    while(i > 0 && x->digit[i - 1] == 0) {
        --i;
    }
    if(i != tmp) {
        x->len = i;
    }

    return mval_obj(x);
}


static MxcValue iadd_intern(MxcValue a, MxcValue b) {
    size_t alen = obig(a)->len, blen = obig(b)->len;
    /* always alen >= blen */
    if(alen < blen) {
        MxcValue tv = a; a = b; b = tv;
        size_t tl = alen; alen = blen; blen = tl;
    }

    MxcInteger *x = obig(a),
               *y = obig(b),
               *r = new_integer_capa(alen + 1, SIGN_PLUS);
    digit2_t carry = 0;
    size_t i = 0;
    for(; i < blen; i++) {
        carry += (digit2_t)x->digit[i] + y->digit[i];
        r->digit[i] = carry & DIGIT_MAX;
        carry >>= DIGIT_POW;
    }
    for(; i < alen; i++) {
        carry += x->digit[i];
        r->digit[i] = carry & DIGIT_MAX;
        carry >>= DIGIT_POW;
    }
    r->digit[i] = carry;

    return integer_norm(r);
}

static MxcValue isub_intern(MxcValue a, MxcValue b) {
    size_t alen = obig(a)->len, blen = obig(b)->len;
    int sign = SIGN_PLUS;
    /* always alen >= blen */
    if(alen < blen) {
        sign = SIGN_MINUS;
        MxcValue tv = a; a = b; b = tv;
        size_t tl = alen; alen = blen; blen = tl;
    }
    MxcInteger *x = obig(a),
               *y = obig(b);
    if(alen == blen) {
        ssize_t i = alen - 1;
        while(i >= 0 && x->digit[i] == y->digit[i]) {
            --i;
        }
        if(i < 0) return mval_int(0); /* a == b */
        if(x->digit[i] < y->digit[i]) {
            sign = SIGN_MINUS;
            MxcInteger *tmp = x; x = y; y = tmp;
        }
        alen = blen = i + 1;
    }
    MxcInteger *r = new_integer_capa(alen, sign);
    size_t i = 0;
    sdigit2_t borrow = 0;
    for(; i < blen; i++) {
        borrow = (sdigit2_t)x->digit[i] - y->digit[i] - borrow;
        r->digit[i] = borrow & DIGIT_MAX;
        borrow >>= DIGIT_POW;
        borrow &= 1;
    }
    for(; i < alen; i++) {
        borrow = (sdigit2_t)x->digit[i] - borrow;
        r->digit[i] = borrow & DIGIT_MAX;
        borrow >>= DIGIT_POW;
        borrow &= 1;
    }

    return integer_norm(r);
}

MxcValue integer_add(MxcValue a, MxcValue b) {
    MxcValue r;
    /*
     *  (+a) + (+b) = a + b
     *  (+a) + (-b) = a - b
     *  (-a) + (+b) = b - a
     *  (-a) + (-b) = -(a + b)
     */
    if(PLUS(a) && PLUS(b)) {
        r = iadd_intern(a, b);
    }
    else if(PLUS(a) && MINUS(b)) {
        r = isub_intern(a, b);
    }
    else if(MINUS(a) && PLUS(b)) {
        r = isub_intern(b, a);
    }
    else if(MINUS(a) && MINUS(b)) {
        r = iadd_intern(a, b);
        obig(r)->sign = SIGN_MINUS;
    }

    return r;
}

MxcValue integer_sub(MxcValue a, MxcValue b) {
    MxcValue r;
    /*
     *  (+a) - (+b) = a - b
     *  (+a) - (-b) = a + b
     *  (-a) - (+b) = -a - b = -(a + b)
     *  (-a) - (-b) = -a + b = b - a
     */
    if(PLUS(a) && PLUS(b)) {
        r = isub_intern(a, b);
    }
    else if(PLUS(a) && MINUS(b)) {
        r = iadd_intern(a, b);
    }
    else if(MINUS(a) && PLUS(b)) {
        r = iadd_intern(a, b);
        obig(r)->sign = SIGN_MINUS;
    }
    else if(MINUS(a) && MINUS(b)) {
        r = isub_intern(b, a);
    }

    return r;
}

/* r += a * b */
static void imuladd_digit_t(digit_t *rd, size_t rlen, MxcInteger *a, digit_t b) {
    if(b == 0) return;
    digit2_t carry = 0;
    digit2_t n = 0;
    digit2_t db = b;
    size_t i = 0;
    size_t alen = a->len;
    digit_t *ad = a->digit;
    for(; i < alen; i++) {
        n = carry + db * ad[i];
        carry = rd[i] + n;
        rd[i] = carry & DIGIT_MAX;
        carry >>= DIGIT_POW;
    }
    for(; i < rlen; i++) {
        if(carry == 0) break;
        carry += rd[i];
        rd[i] = carry & DIGIT_MAX;
        carry >>= DIGIT_POW;
    }
}

static MxcValue imul_intern(MxcInteger *a, MxcInteger *b) {
    size_t alen = a->len, blen = b->len;
    digit_t *bd = b->digit;
    MxcInteger *r = new_integer_capa(alen + blen, a->sign == b->sign);
    for(size_t i = 0; i < blen; i++) {
        imuladd_digit_t(r->digit + i, r->len - i, a, bd[i]);
    }

    return integer_norm(r);
}

MxcValue integer_mul(MxcValue a, MxcValue b) {
    return imul_intern(obig(a), obig(b));
}

// only GCC and sizeof(int)*CHAR_BIT == 32
unsigned int nlz_int(unsigned int n) {
    return n ? __builtin_clz(n) : 32;
}

/* Knuth algorithm D */
static void idivrem_knuthd(MxcInteger *a1,
                           MxcInteger *b1,
                           MxcValue *quo,
                           MxcValue *rem) {
    size_t alen = a1->len;
    size_t blen = b1->len;
    MxcInteger *a = new_integer_capa(alen + 1, SIGN_PLUS);
    GC_GUARD(a);
    MxcInteger *b = new_integer_capa(blen, SIGN_PLUS);
    GC_GUARD(b);
    unsigned int shift = nlz_int(b1->digit[blen - 1]);

    darylshift(b->digit, b1->digit, blen, shift);
    size_t tcarry = darylshift(a->digit, a1->digit, alen, shift);
    if(tcarry || a->digit[alen - 1] >= b->digit[blen - 1]) {
        a->digit[alen] = tcarry;
        alen++;
    }
    int k = alen - blen;
    MxcInteger *qo = new_integer_capa(k, SIGN_PLUS);
    GC_GUARD(qo);

    digit_t *adig = a->digit;
    digit_t *bdig = b->digit;

    for(int j = k; j >= 0; --j) {
        digit2_t ttx = (digit2_t)adig[j + blen] << DIGIT_POW | adig[j + blen - 1];
        digit2_t q = ttx / bdig[blen - 1];
        digit2_t r = ttx % bdig[blen - 1];
        while(q >= DIGIT_BASE || 
              q * bdig[blen - 2] > (r << DIGIT_POW | adig[j + blen - 2])) {
            --q;
            r += bdig[blen - 1];
            if(r >= DIGIT_BASE) break;
        }

        sdigit2_t carry = 0;
        for(size_t i = 0; i < blen; ++i) {
            carry += (sdigit2_t)adig[j + i] - (sdigit2_t)q * (sdigit2_t)bdig[i];
            adig[j + i] = carry & DIGIT_MAX;
            carry >>= DIGIT_POW;
        }

        if(carry + (sdigit2_t)adig[j + blen] < 0) {
            --q;
            digit2_t carry2 = 0;
            for(size_t i = 0; i < blen; i++) {
                carry2 += adig[j + i] + bdig[i];
                adig[j + i] = carry2 & DIGIT_MAX;
                carry2 >>= DIGIT_POW;
            }
        }

        qo->digit[j] = q;
    }

    daryrshift(bdig, adig, blen, shift);

    if(quo) *quo = integer_norm(qo);
    if(rem) *rem = integer_norm(b);

    GC_GUARD(a);
    GC_GUARD(b);
    GC_GUARD(qo);
}

static void idivrem_intern(MxcInteger *a,
                           MxcInteger *b,
                           MxcValue *quo,
                           MxcValue *rem) {
    size_t alen = a->len;
    size_t blen = b->len;
    if(blen == 0) {
        // error
        return;
    }
    else if(alen < blen || 
            (alen == blen &&
             a->digit[alen - 1] < b->digit[blen - 1])) {
        if(quo) *quo = mval_int(0);
        if(rem) *rem = mval_obj(a);
        return;
    }
    else if(blen == 1) {
        MxcInteger *q = new_integer_capa(a->len, a->sign == b->sign);
        MxcValue qq = mval_obj(q);
        digit_t r = integer_divrem1(a, b->digit[0], &qq);
        if(quo) *quo = qq;
        if(rem) *rem = mval_int(r);
        return;
    }
    else {
        idivrem_knuthd(a, b, quo, rem);
    }
}

static digit_t integer_divrem1(MxcInteger *a, digit_t b, MxcValue *q) {
    digit2_t t = 0;
    MxcInteger *qb = obig(*q);
    for(size_t i = 0; i < a->len; i++) {
        t = t << DIGIT_POW | a->digit[a->len - i - 1];
        qb->digit[a->len - i - 1] = (digit_t)(t / b);
        t %= b;
    }
    *q = integer_norm(qb);

    return t;
}

MxcValue integer_divrem(MxcValue a, MxcValue b, MxcValue *rem) {
    MxcValue quo;
    idivrem_intern(obig(a), obig(b), &quo, rem);

    return quo;
}

MxcValue integer_eq(MxcValue a1, MxcValue b1) {
    MxcInteger *a = obig(a1);
    MxcInteger *b = obig(b1);
    if(a->sign != b->sign) return mval_false;
    if(a->len != b->len) return mval_false;
    if(memcmp(a->digit, b->digit, sizeof(digit_t) * a->len))
        return mval_false;
    return mval_true;
}

static void digit2_t_to_dary(digit_t *digs, digit2_t a) {
    digs[0] = a & DIGIT_MAX;
    digs[1] = a >> DIGIT_POW;
}

static digit2_t digits_to_digit2(digit_t *digs, size_t ndig) {
    if(ndig == 2) return digs[0] | (digit2_t)digs[1] << 32;
    if(ndig == 1) return digs[0];
    return 0;
}

static digit_t darylshift(digit_t *r, digit_t *a, size_t n, int shift) {
    digit2_t carry = 0;
    for(size_t i = 0; i < n; i++) {
        carry |= (digit2_t)a[i] << shift;
        r[i] = carry & DIGIT_MAX;
        carry >>= DIGIT_POW;
    }
    return (digit_t)(carry & DIGIT_MAX);
}

static void daryrshift(digit_t *r, digit_t *a, size_t n, int shift) {
    digit2_t carry = 0;
    for(size_t i = 0; i < n; i++) {
        digit_t x = a[n - i - 1];
        carry = (carry | x) >> shift;
        r[n - i - 1] = carry & DIGIT_MAX;
        carry = (digit2_t)x << DIGIT_POW;
    }
}

static MxcValue integer2str(MxcInteger *self, int base) {
    int neg = !self->sign;
    MxcInteger *iquo = new_integer_capa(self->len, SIGN_PLUS);
    MxcValue quo = mval_obj(iquo);
    size_t nbuf = self->len * log2to32power_inv(base);
    char buf[nbuf];
    char *end = buf + sizeof(buf);
    char *cur = end;

    do {
        digit_t rem = integer_divrem1(self, base, &quo);
        *--cur = mxc_36digits[rem];
        self = obig(quo);
    } while(self->len != 0);
    if(neg) {
        *--cur = '-';
    }

    return new_string_copy(cur, end - cur);
}

MxcValue integer_tostring(MxcObject *ob) {
    return integer2str((MxcInteger *)ob, 10);
}

MxcObjImpl integer_objimpl = {
    "integer",
    integer_tostring,
    integer_dealloc,
    integer_copy,
    integer_gc_mark,
    integer_guard,
    integer_unguard,
    0,
    0,
};
