#ifndef MXC_FILEOBJECT_H
#define MXC_FILEOBJECT_H

#include <stdio.h>
#include "object/object.h"
#include "object/mstr.h"

typedef struct MFile MFile;
struct MFile {
  OBJECT_HEAD;
  MString *path;
  FILE *file;
};

#endif
