#include "literalpool.h"
#include "maxc.h"

Literal *New_Literal() {
    Literal *l = malloc(sizeof(Literal));

    return l;
}

Literal *New_Literal_With_Str(char *str) {
    Literal *l = malloc(sizeof(Literal));

    l->kind = LIT_STR;
    l->str = str;

    return l;
}

Literal *New_Literal_With_Fnumber(double f) {
    Literal *l = malloc(sizeof(Literal));

    l->kind = LIT_FNUM;
    l->fnumber = f;

    return l;
}

Literal *New_Literal_With_Userfn(userfunction *u) {
    Literal *l = malloc(sizeof(Literal));

    l->kind = LIT_FUNC;
    l->func = u;

    return l;
}

int lpool_push_str(Vector *table, char *s) {
    for(int i = 0; i < table->len; ++i) {
        if(((Literal *)table->data[i])->kind != LIT_STR)
            continue;
        if(strcmp(((Literal *)table->data[i])->str, s) == 0) {
            return i;
        }

        ++i;
    }

    int key = table->len;
    vec_push(table, New_Literal_With_Str(s));

    return key;
}

int lpool_push_float(Vector *table, double fnum) {
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

void lpooldump(Vector *table) {
    for(int i = 0; i < table->len; ++i) {
        Literal *a = (Literal *)table->data[i];

        switch(a->kind) {
        case LIT_STR:
            printf(" str: %s ", a->str);
            break;
        case LIT_FNUM:
            printf(" fnum: %f ", a->fnumber);
            break;
        case LIT_FUNC:
            printf(" func ");
            break;
        }
    }

    puts("");
}
