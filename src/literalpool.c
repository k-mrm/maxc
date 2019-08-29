#include "literalpool.h"
#include "maxc.h"

Literal *New_Literal() {
    Literal *l = malloc(sizeof(Literal));

    return l;
}

Literal *New_Literal_With_Str(char *str) {
    Literal *l = New_Literal();

    l->kind = LIT_STR;
    l->str = str;

    return l;
}

Literal *New_Literal_With_Fnumber(double f) {
    Literal *l = New_Literal();

    l->kind = LIT_FNUM;
    l->fnumber = f;

    return l;
}

Literal *New_Literal_With_Userfn(userfunction *u) {
    Literal *l = New_Literal();

    l->kind = LIT_FUNC;
    l->func = u;

    return l;
}

int lpool_push_str(Vector *table, char *s) {
    for(int i = 0; i < table->len; ++i) {
        if(((Literal *)table->data[i])->kind != LIT_STR)
            continue;
        if(strncmp(((Literal *)table->data[i])->str,
                   s,
                   strlen(((Literal *)table->data[i])->str)) == 0) {
            return i;
        }
        ++i;
    }

    int key = table->len;
    vec_push(table, New_Literal_With_Str(s));

    return key;
}

int lpool_push_float(Vector *table, double fnum) {
    int i = 0;
    for(int i = 0; i < table->len; ++i) {
        if(((Literal *)table->data[i])->kind != LIT_FNUM)
            continue;
        if(((Literal *)table->data[i])->fnumber == fnum) {
            return i;
        }

        ++i;
    }

    int key = table->len;
    vec_push(table, New_Literal_With_Fnumber(fnum));

    return key;
}

int lpool_push_userfunc(Vector *table, userfunction *func) {
    int key = table->len;
    vec_push(table, New_Literal_With_Userfn(func));

    return key;
}
