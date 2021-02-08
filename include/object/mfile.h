#ifndef MXC_FILEOBJECT_H
#define MXC_FILEOBJECT_H

#include <stdio.h>
#include <sys/stat.h>
#include "object/object.h"
#include "object/mstr.h"

typedef struct MStat MStat;
struct MStat {
  OBJECT_HEAD;
  struct stat st;
};

typedef struct MFile MFile;
struct MFile {
  OBJECT_HEAD;
  MString *path;
  FILE *file;
};

#endif
