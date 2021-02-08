#ifndef MXC_MLIB_H
#define MXC_MLIB_H

#include "maxc.h"
#include "mlibapi.h"

void std_init(void);
void file_init(void);
void str_init(void);
void list_init(void);
void dir_init(void);

void setup_argv(int argc, char **argv);

extern Vector *gmodule;

#define FTYPE(...) __VA_ARGS__, NULL

#endif
