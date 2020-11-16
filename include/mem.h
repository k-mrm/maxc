#ifndef MAXC_MEM_H
#define MAXC_MEM_H

#include "object/object.h"
#include "object/minteger.h"
#include "object/mbool.h"
#include "object/mchar.h"
#include "object/mfloat.h"
#include "object/mfunc.h"
#include "object/mint.h"
#include "object/mlist.h"
#include "object/mstr.h"
#include "object/mstruct.h"

#define OBJECT_POOL

#ifdef OBJECT_POOL
union obalign {
  MxcInteger i;
  MList l;
  MStrct st;
  MString s;
  MxcFunction fn;
  MxcCFunc cf;
};

typedef struct ObjectPool {
  MxcObject **pool;
  uint16_t len;
  uint16_t reserved;
} ObjectPool;

#define OBPOOL_LAST (obpool.pool[obpool.len - 1])

void New_Objectpool(void);
void obpool_push(MxcObject *);

#endif

#ifdef OBJECT_POOL
#   define Mxc_free(ob) do {                    \
  obpool_push((MxcObject *)(ob));         \
} while(0)
#else
#   define Mxc_free(ob) free(ob)
#endif  /* OBJECT_POOL */

#ifndef USE_MARK_AND_SWEEP
#   define INCREF(ob) (++((MxcObject *)(ob))->refcount)

#   define DECREF(ob)                                                              \
  do {                                                                       \
    if(--((MxcObject *)(ob))->refcount == 0) {                             \
      SYSTEM((MxcObject *)ob)->dealloc((MxcObject *)ob);                \
    }                                                                      \
  } while(0)
#else
#   define INCREF(ob) ((void)0)
#   define DECREF(ob) ((void)0)
#endif  /* USE_MARK_AND_SWEEP */

#define GC_MARK(ob) (SYSTEM(ob)->mark((MxcObject *)(ob)))
#define GC_GUARD(ob) (SYSTEM(ob)->guard((MxcObject *)(ob)))
#define GC_UNGUARD(ob) (SYSTEM(ob)->unguard((MxcObject *)(ob)))

MxcObject *mxc_alloc(size_t);

#endif  /* MAXC_MEM_H */
