#ifndef MAXC_CONST_H
#define MAXC_CONST_H

#include "function.h"
#include "util.h"
#include "object/object.h"

enum LITKIND {
    LIT_STR,
    LIT_FNUM,
    LIT_LONG,
    LIT_FUNC,
    LIT_RAWOBJ,
};

typedef struct Literal {
    enum LITKIND kind;
    union {
        char *str; // str
        double fnumber;
        int64_t lnum;
        userfunction *func;
        MxcValue raw;
    };
} Literal;

Literal *New_Literal();
Literal *New_Literal_With_Str(char *);
Literal *New_Literal_Long(int64_t);
Literal *New_Literal_With_Fnumber(double);
Literal *New_Literal_With_Userfn(userfunction *);
Literal *New_Literal_Object(MxcValue);
int lpool_push_str(Vector *, char *);
int lpool_push_long(Vector *, int64_t);
int lpool_push_float(Vector *, double);
int lpool_push_userfunc(Vector *, userfunction *);
int lpool_push_object(Vector *, MxcValue);

void lpooldump(Vector *);

#endif
