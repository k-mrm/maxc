#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include "ast.h"
#include "env.h"
#include "maxc.h"
#include "method.h"
#include "type.h"

int sema_analysis(Vector *);
void setup_bltin(void);
void sema_init(void);

#endif
