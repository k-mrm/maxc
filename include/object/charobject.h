#ifndef MXC_CHAROBJECT_H
#define MXC_CHAROBJECT_H

#include "object/object.h"

typedef struct MxcChar {
    OBJECT_HEAD;
    char ch;
} MxcChar;

MxcValue new_char(char);
MxcValue new_char_ref(char *c);

#endif
