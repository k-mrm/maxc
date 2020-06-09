#ifndef MXC_FILEOBJECT_H
#define MXC_FILEOBJECT_H

#include <stdio.h>
#include "object/object.h"
#include "object/strobject.h"

typedef struct MFile MFile;
struct MFile {
  OBJECT_HEAD;
  MxcString *path;
  FILE *file;
};

#endif
