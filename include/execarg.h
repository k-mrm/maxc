#ifndef MXC_EXECARG_H
#define MXC_EXECARG_H

#include "object/object.h"

struct execarg {
  MxcValue top;
  MxcValue second;  /* if ncache == 1, treat as stack top */

  MxcValue *argv;
} __attribute__((__packed__));

#endif
