#ifndef MAXC_OBJECT_TOSTRING_H
#define MAXC_OBJECT_TOSTRING_H

#include "maxc.h"
#include "object/object.h"

StringObject *int_tostring(MxcObject *);
StringObject *true_tostring(MxcObject *);
StringObject *false_tostring(MxcObject *);
StringObject *string_tostring(MxcObject *);
StringObject *null_tostring(MxcObject *);

#endif
