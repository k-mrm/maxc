#ifndef MXC_OBJECT_MDIR_H
#define MXC_OBJECT_MDIR_H

#include <stdio.h>
#include <dirent.h>
#include "object/object.h"

typedef struct MDir MDir;
struct MDir {
  OBJECT_HEAD;
  MString *path;
  DIR *dir;
};

#endif
