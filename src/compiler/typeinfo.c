#include "maxc.h"
#include "type.h"
#include "error/error.h"

TypeInfo tinfo_none = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_integer = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_boolean = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_float = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_string = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_unsolved = {
    0,
    false
};

TypeInfo tinfo_any = {
    TIMPL_SHOW,
    true
};

TypeInfo tinfo_any_vararg = {
    TIMPL_SHOW,
    true
};
