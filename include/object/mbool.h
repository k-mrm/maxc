#ifndef MXC_BOOLOBJECT_H
#define MXC_BOOLOBJECT_H

#include <stdint.h>
#include "object/object.h"

MxcValue bool_logor(MxcValue, MxcValue);
MxcValue bool_logand(MxcValue, MxcValue);
MxcValue bool_not(MxcValue);

#endif
