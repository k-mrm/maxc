#ifndef MAXC_CONST_H
#define MAXC_CONST_H

#include "function.h"
#include "maxc.h"

enum LITKIND {
    LIT_STR,
    LIT_FNUM,
    LIT_FUNC,
};

typedef struct Literal {
    enum LITKIND kind;
    union {
        char *str; // str
        double fnumber;
        userfunction *func;
    };
} Literal;

Literal *New_Literal();
Literal *New_Literal_With_Str(char *);
Literal *New_Literal_With_Fnumber(double);
Literal *New_Literal_With_Userfn(userfunction *);

int lpool_push_str(Vector *, char *);
int lpool_push_float(Vector *, double);
int lpool_push_userfunc(Vector *, userfunction *);

#endif
