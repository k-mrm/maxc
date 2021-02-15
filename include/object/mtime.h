#ifndef MXC_OBJECT_TIME_H
#define MXC_OBJECT_TIME_H

#include <stdio.h>
#include <time.h>
#include "object/object.h"

typedef struct MTime MTime;
struct MTime {
  OBJECT_HEAD;
  struct tm time;
};

MxcValue time_from_utime(time_t utime);

#endif
