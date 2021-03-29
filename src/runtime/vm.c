#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "vm.h"
#include "execarg.h"
#include "ast.h"
#include "bytecode.h"
#include "error/error.h"
#include "error/runtime-err.h"
#include "literalpool.h"
#include "mem.h"
#include "gc.h"
#include "object/object.h"
#include "object/system.h"
#include "object/mbool.h"
#include "object/mfloat.h"
#include "object/mfunc.h"
#include "object/mint.h"
#include "object/num.h"
#include "object/mlist.h"
#include "object/mstr.h"
#include "object/mfiber.h"
#include "object/mtable.h"

int error_flag = 0;

#ifdef DIRECT_THREADED
#define Start() do { goto **pc; } while(0)
#define Dispatch() do { goto **pc; } while(0)
#define CASE(op) OP_ ## op: printf("enter %s\n", #op);
#define ENDOFVM
#else
#define Start() for(;;) { printf("%d ", scstate); switch(*pc + scstate) {
#define Dispatch() break
#define CASE(op) case OP_ ## op: printf("enter %s\n", #op);
#define ENDOFVM }}
#endif

#define list_setitem(val, index, item) (olist(val)->elem[(index)] = (item))

#define member_getitem(ob, offset)       (ostrct(ob)->field[(offset)])
#define member_setitem(ob, offset, item) (ostrct(ob)->field[(offset)] = (item))

#define PEEKARG(pc) ((smptr_t)*(pc))
#define READARG(pc) (PEEKARG((pc)+1))

VM gvm;
extern clock_t gc_time;

mptr_t **pcsaver;

/*
 * POP:
 *  scstate = (scstate - 2) < 0? 0 : scstate - 2;
 *
 */

enum stack_cache_state scstate = SCXX;

/*
 *  [scstate]_[before stack]_[after stack]
 *  
 *  X_W: 0->1
 *  W_X: 1->0
 *  WW_W: 2->1
 */

#define SCXX_X_W(ob)  \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAX_X_W(ob)  \
  do {  \
    screg_b = (ob); \
    scstate = SCAB; \
  } while(0)
#define SCBX_X_W(ob)  \
  do {  \
    screg_a = (ob); \
    scstate = SCBA; \
  } while(0)
#define SCBA_X_W(ob)  \
  do {  \
    PUSH(screg_b);  \
    screg_b = (ob); \
    scstate = SCAB; \
  } while(0)
#define SCAB_X_W(ob)  \
  do {  \
    PUSH(screg_a);  \
    screg_a = (ob); \
    scstate = SCBA; \
  } while(0)

#define SCXX_W_X()  \
  do {  \
    (void)POP();  \
    scstate = SCXX; \
  } while(0)
#define SCAX_W_X()  \
  do {  \
    scstate = SCXX; \
  } while(0)
#define SCBX_W_X()  \
  do {  \
    scstate = SCXX; \
  } while(0)
#define SCBA_W_X()  \
  do {  \
    scstate = SCBX; \
  } while(0)
#define SCAB_W_X()  \
  do {  \
    scstate = SCAX; \
  } while(0)

#define SCXX_W_W(top, ob) \
  do {  \
    top = POP();  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAX_W_W(top, ob) \
  do {  \
    top = screg_a;  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBX_W_W(top, ob) \
  do {  \
    top = screg_b;  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBA_W_W(top, ob) \
  do {  \
    top = screg_a;  \
    screg_a = (ob); \
    scstate = SCBA; \
  } while(0)
#define SCAB_W_W(top, ob) \
  do {  \
    top = screg_b;  \
    screg_b = (ob); \
    scstate = SCAB; \
  } while(0)

#define SCXX_WW_W(top, snd, ob) \
  do {  \
    top = POP();  \
    snd = POP();  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAX_WW_W(top, snd, ob) \
  do {  \
    top = screg_a;  \
    snd = POP();  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBX_WW_W(top, snd, ob) \
  do {  \
    top = screg_b;  \
    snd = POP();  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBA_WW_W(top, snd, ob) \
  do {  \
    top = screg_a;  \
    snd = screg_b;  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAB_WW_W(top, snd, ob) \
  do {  \
    top = screg_b;  \
    snd = screg_a;  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)


#define NON_DESTRUCTIVE_CASE(op)  \
  CASE(op ## _SCXX) \
  CASE(op ## _SCAX) \
  CASE(op ## _SCBX) \
  CASE(op ## _SCBA) \
  CASE(op ## _SCAB)


void vm_open(mptr_t *code, MxcValue *gvars, int ngvars, Vector *ltab, DebugInfo *d) {
  VM *vm = curvm();
  vm->ctx = new_econtext(code, 0, d, NULL);
  vm->gvars = gvars;
  vm->ngvars = ngvars;
  vm->stackptr = (MxcValue *)calloc(1, sizeof(MxcValue) * 1024);
  vm->stackbase = vm->stackptr;
  vm->ltable = ltab;
}

void vm_force_exit(int status) {
  longjmp(curvm()->vm_end_jb, status);
}

int vm_run() {
  VM *vm = curvm();
  int ret;
  int c;

#ifdef MXC_DEBUG
  printf(MUTED("ptr: %p")"\n", vm->stackptr);
#endif

  if((c = setjmp(vm->vm_end_jb)) == 0) {
    ret = (int)(intptr_t)vm_exec(vm);
  }
  else {
    ret = c;
  }

#ifdef MXC_DEBUG
  printf("GC time: %.5lf\n", (double)(gc_time) / CLOCKS_PER_SEC);
  printf(MUTED("ptr: %p")"\n", vm->stackptr);
#endif

  return ret;
}

MxcValue screg_a;
MxcValue screg_b;

void *vm_exec(VM *vm) {
#ifdef DIRECT_THREADED
  static void *optable[] = {
#define OPCODE_DEF(op)  \
    &&OP_ ## op ## _SCXX,  \
    &&OP_ ## op ## _SCAX,  \
    &&OP_ ## op ## _SCBX,  \
    &&OP_ ## op ## _SCBA,  \
    &&OP_ ## op ## _SCAB,
#include "opcode-def.h"
#undef OPCODE_DEF
  };

  if(UNLIKELY(!vm)) {
    /* get optable */
    return (void *)optable;
  }
#endif

  MContext *context = vm->ctx;

  MxcValue *gvmap = vm->gvars;
  mptr_t *code = context->code;
  mptr_t *pc = context->pc;
  pcsaver = &pc;
  Literal **lit_table = (Literal **)vm->ltable->data;
  int key;

  Start();

  CASE(PUSH_SCXX) {
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCXX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(PUSH_SCAX) {
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;
    SCAX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(PUSH_SCBX) {
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;
    
    SCBX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(PUSH_SCBA) {
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCBA_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(PUSH_SCAB) {
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCAB_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(IPUSH_SCXX) {
    SCXX_X_W(mval_int(READARG(pc)));

    pc += 2;
    Dispatch();
  }
  CASE(IPUSH_SCAX) {
    SCAX_X_W(mval_int(READARG(pc)));

    pc += 2;
    Dispatch();
  }
  CASE(IPUSH_SCBX) {
    SCBX_X_W(mval_int(READARG(pc)));

    pc += 2;
    Dispatch();
  }
  CASE(IPUSH_SCBA) {
    SCBA_X_W(mval_int(READARG(pc)));

    pc += 2;
    Dispatch();
  }
  CASE(IPUSH_SCAB) {
    SCAB_X_W(mval_int(READARG(pc)));

    pc += 2;
    Dispatch();
  }
  CASE(PUSHCONST_0_SCXX) {
    pc++;
    SCXX_X_W(mval_int(0));

    Dispatch();
  }
  CASE(PUSHCONST_0_SCAX) {
    pc++;
    SCAX_X_W(mval_int(0));

    Dispatch();
  }
  CASE(PUSHCONST_0_SCBX) {
    pc++;
    SCBX_X_W(mval_int(0));

    Dispatch();
  }
  CASE(PUSHCONST_0_SCBA) {
    pc++;
    SCBA_X_W(mval_int(0));

    Dispatch();
  }
  CASE(PUSHCONST_0_SCAB) {
    pc++;
    SCAB_X_W(mval_int(0));

    Dispatch();
  }
  CASE(PUSHCONST_1_SCXX) {
    pc++;
    SCXX_X_W(mval_int(1));

    Dispatch();
  }
  CASE(PUSHCONST_1_SCAX) {
    pc++;
    SCAX_X_W(mval_int(1));

    Dispatch();
  }
  CASE(PUSHCONST_1_SCBX) {
    pc++;
    SCBX_X_W(mval_int(1));

    Dispatch();
  }
  CASE(PUSHCONST_1_SCBA) {
    pc++;
    SCBA_X_W(mval_int(1));

    Dispatch();
  }
  CASE(PUSHCONST_1_SCAB) {
    pc++;
    SCAB_X_W(mval_int(1));

    Dispatch();
  }
  CASE(PUSHCONST_2_SCXX) {
    pc++;
    SCXX_X_W(mval_int(2));

    Dispatch();
  }
  CASE(PUSHCONST_2_SCAX) {
    pc++;
    SCAX_X_W(mval_int(2));

    Dispatch();
  }
  CASE(PUSHCONST_2_SCBX) {
    pc++;
    SCBX_X_W(mval_int(2));

    Dispatch();
  }
  CASE(PUSHCONST_2_SCBA) {
    pc++;
    SCBA_X_W(mval_int(2));

    Dispatch();
  }
  CASE(PUSHCONST_2_SCAB) {
    pc++;
    SCAB_X_W(mval_int(2));

    Dispatch();
  }
  CASE(PUSHCONST_3_SCXX) {
    pc++;
    SCXX_X_W(mval_int(3));

    Dispatch();
  }
  CASE(PUSHCONST_3_SCAX) {
    pc++;
    SCAX_X_W(mval_int(3));

    Dispatch();
  }
  CASE(PUSHCONST_3_SCBX) {
    pc++;
    SCBX_X_W(mval_int(3));

    Dispatch();
  }
  CASE(PUSHCONST_3_SCBA) {
    pc++;
    SCBA_X_W(mval_int(3));

    Dispatch();
  }
  CASE(PUSHCONST_3_SCAB) {
    pc++;
    SCAB_X_W(mval_int(3));

    Dispatch();
  }
  CASE(PUSHTRUE_SCXX) {
    pc++;
    SCXX_X_W(mval_true);

    Dispatch();
  }
  CASE(PUSHTRUE_SCAX) {
    pc++;
    SCAX_X_W(mval_true);

    Dispatch();
  }
  CASE(PUSHTRUE_SCBX) {
    pc++;
    SCBX_X_W(mval_true);

    Dispatch();
  }
  CASE(PUSHTRUE_SCBA) {
    pc++;
    SCBA_X_W(mval_true);

    Dispatch();
  }
  CASE(PUSHTRUE_SCAB) {
    pc++;
    SCAB_X_W(mval_true);

    Dispatch();
  }
  CASE(PUSHFALSE_SCXX) {
    pc++;
    SCXX_X_W(mval_false);

    Dispatch();
  }
  CASE(PUSHFALSE_SCAX) {
    pc++;
    SCAX_X_W(mval_false);

    Dispatch();
  }
  CASE(PUSHFALSE_SCBX) {
    pc++;
    SCBX_X_W(mval_false);

    Dispatch();
  }
  CASE(PUSHFALSE_SCBA) {
    pc++;
    SCBA_X_W(mval_false);

    Dispatch();
  }
  CASE(PUSHFALSE_SCAB) {
    pc++;
    SCAB_X_W(mval_false);

    Dispatch();
  }
  CASE(PUSHNULL_SCXX) {
    pc++;
    SCXX_X_W(mval_null);

    Dispatch();
  }
  CASE(PUSHNULL_SCAX) {
    pc++;
    SCAX_X_W(mval_null);

    Dispatch();
  }
  CASE(PUSHNULL_SCBX) {
    pc++;
    SCBX_X_W(mval_null);

    Dispatch();
  }
  CASE(PUSHNULL_SCBA) {
    pc++;
    SCBA_X_W(mval_null);

    Dispatch();
  }
  CASE(PUSHNULL_SCAB) {
    pc++;
    SCAB_X_W(mval_null);

    Dispatch();
  }
  CASE(FPUSH_SCXX){
    key = (int)READARG(pc);
    SCXX_X_W(mval_float(lit_table[key]->fnumber));

    pc += 2;
    Dispatch();
  }
  CASE(FPUSH_SCAX){
    key = (int)READARG(pc);
    SCAX_X_W(mval_float(lit_table[key]->fnumber));

    pc += 2;
    Dispatch();
  }
  CASE(FPUSH_SCBX){
    key = (int)READARG(pc);
    SCBX_X_W(mval_float(lit_table[key]->fnumber));

    pc += 2;
    Dispatch();
  }
  CASE(FPUSH_SCBA){
    key = (int)READARG(pc);
    SCBA_X_W(mval_float(lit_table[key]->fnumber));

    pc += 2;
    Dispatch();
  }
  CASE(FPUSH_SCAB){
    key = (int)READARG(pc);
    SCAB_X_W(mval_float(lit_table[key]->fnumber));

    pc += 2;
    Dispatch();
  }
  CASE(POP_SCXX) {
    pc++;
    SCXX_W_X();

    Dispatch();
  }
  CASE(POP_SCAX) {
    pc++;
    SCAX_W_X();

    Dispatch();
  }
  CASE(POP_SCBX) {
    pc++;
    SCBX_W_X();

    Dispatch();
  }
  CASE(POP_SCBA) {
    pc++;
    SCBA_W_X();

    Dispatch();
  }
  CASE(POP_SCAB) {
    pc++;
    SCAB_W_X();

    Dispatch();
  }
  CASE(ADD_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_add(l, r));

    Dispatch();
  }
  CASE(FADD_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, FloatAdd(l, r));

    Dispatch();
  }
  CASE(FADD_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, FloatAdd(l, r));

    Dispatch();
  }
  CASE(FADD_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, FloatAdd(l, r));

    Dispatch();
  }
  CASE(FADD_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, FloatAdd(l, r));

    Dispatch();
  }
  CASE(FADD_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, FloatAdd(l, r));

    Dispatch();
  }
  CASE(STRCAT_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(STRCAT_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(STRCAT_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(STRCAT_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(STRCAT_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(SUB_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_sub(l, r));

    Dispatch();
  }
  CASE(FSUB_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, FloatSub(l, r));

    Dispatch();
  }
  CASE(FSUB_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, FloatSub(l, r));

    Dispatch();
  }
  CASE(FSUB_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, FloatSub(l, r));

    Dispatch();
  }
  CASE(FSUB_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, FloatSub(l, r));

    Dispatch();
  }
  CASE(FSUB_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, FloatSub(l, r));

    Dispatch();
  }
  CASE(MUL_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_mul(l, r));

    Dispatch();
  }
  CASE(FMUL_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, FloatMul(l, r));

    Dispatch();
  }
  CASE(FMUL_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, FloatMul(l, r));

    Dispatch();
  }
  CASE(FMUL_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, FloatMul(l, r));

    Dispatch();
  }
  CASE(FMUL_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, FloatMul(l, r));

    Dispatch();
  }
  CASE(FMUL_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, FloatMul(l, r));

    Dispatch();
  }
  CASE(DIV_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_div(l, r));

    Dispatch();
  }
  CASE(FDIV_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_div(l, r));

    Dispatch();
  }
  CASE(FDIV_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_div(l, r));

    Dispatch();
  }
  CASE(FDIV_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_div(l, r));

    Dispatch();
  }
  CASE(FDIV_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_div(l, r));

    Dispatch();
  }
  CASE(FDIV_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_div(l, r));

    Dispatch();
  }
  CASE(MOD_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_mod(l, r));

    Dispatch();
  }
  CASE(LOGOR_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGOR_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGOR_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGOR_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGOR_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGAND_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, bool_logand(l, r));

    Dispatch();
  }
  CASE(LOGAND_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, bool_logand(l, r));

    Dispatch();
  }
  CASE(LOGAND_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, bool_logand(l, r));

    Dispatch();
  }
  CASE(LOGAND_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, bool_logand(l, r));

    Dispatch();
  }
  CASE(LOGAND_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, bool_logand(l, r));

    Dispatch();
  }
  CASE(BXOR_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, int_xor(l, r));

    Dispatch();
  }
  CASE(BXOR_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, int_xor(l, r));

    Dispatch();
  }
  CASE(BXOR_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, int_xor(l, r));

    Dispatch();
  }
  CASE(BXOR_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, int_xor(l, r));

    Dispatch();
  }
  CASE(BXOR_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, int_xor(l, r));

    Dispatch();
  }
  CASE(EQ_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_eq(l, r));

    Dispatch();
  }
  CASE(EQ_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_eq(l, r));

    Dispatch();
  }
  CASE(EQ_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_eq(l, r));

    Dispatch();
  }
  CASE(EQ_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_eq(l, r));

    Dispatch();
  }
  CASE(EQ_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_eq(l, r));

    Dispatch();
  }
  CASE(FEQ_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_eq(l, r));

    Dispatch();
  }
  CASE(FEQ_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_eq(l, r));

    Dispatch();
  }
  CASE(FEQ_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_eq(l, r));

    Dispatch();
  }
  CASE(FEQ_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_eq(l, r));

    Dispatch();
  }
  CASE(FEQ_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_eq(l, r));

    Dispatch();
  }
  CASE(NOTEQ_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, num_noteq(l, r));

    Dispatch();
  }
  CASE(NOTEQ_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, num_noteq(l, r));

    Dispatch();
  }
  CASE(NOTEQ_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, num_noteq(l, r));

    Dispatch();
  }
  CASE(NOTEQ_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, num_noteq(l, r));

    Dispatch();
  }
  CASE(NOTEQ_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, num_noteq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_neq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_neq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_neq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_neq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_neq(l, r));

    Dispatch();
  }
  CASE(LT_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, int_lt(l, r));

    Dispatch();
  }
  CASE(LT_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, int_lt(l, r));

    Dispatch();
  }
  CASE(LT_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, int_lt(l, r));

    Dispatch();
  }
  CASE(LT_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, int_lt(l, r));

    Dispatch();
  }
  CASE(LT_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, int_lt(l, r));

    Dispatch();
  }
  CASE(FLT_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLT_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLT_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLT_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLT_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(LTE_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, int_lte(l, r));

    Dispatch();
  }
  CASE(LTE_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, int_lte(l, r));

    Dispatch();
  }
  CASE(LTE_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, int_lte(l, r));

    Dispatch();
  }
  CASE(LTE_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, int_lte(l, r));

    Dispatch();
  }
  CASE(LTE_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, int_lte(l, r));

    Dispatch();
  }
  CASE(FLTE_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLTE_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLTE_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLTE_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(FLTE_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_lt(l, r));

    Dispatch();
  }
  CASE(GT_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, int_gt(l, r));

    Dispatch();
  }
  CASE(GT_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, int_gt(l, r));

    Dispatch();
  }
  CASE(GT_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, int_gt(l, r));

    Dispatch();
  }
  CASE(GT_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, int_gt(l, r));

    Dispatch();
  }
  CASE(GT_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, int_gt(l, r));

    Dispatch();
  }
  CASE(FGT_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, float_gt(l, r));

    Dispatch();
  }
  CASE(FGT_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, float_gt(l, r));

    Dispatch();
  }
  CASE(FGT_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, float_gt(l, r));

    Dispatch();
  }
  CASE(FGT_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, float_gt(l, r));

    Dispatch();
  }
  CASE(FGT_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, float_gt(l, r));

    Dispatch();
  }
  CASE(GTE_SCXX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCXX_WW_W(r, l, int_gte(l, r));

    Dispatch();
  }
  CASE(GTE_SCAX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAX_WW_W(r, l, int_gte(l, r));

    Dispatch();
  }
  CASE(GTE_SCBX) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBX_WW_W(r, l, int_gte(l, r));

    Dispatch();
  }
  CASE(GTE_SCBA) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCBA_WW_W(r, l, int_gte(l, r));

    Dispatch();
  }
  CASE(GTE_SCAB) {
    pc++;
    MxcValue r;
    MxcValue l;
    SCAB_WW_W(r, l, int_gte(l, r));

    Dispatch();
  }
  CASE(INEG_SCXX) {
    pc++;
    MxcValue u;
    SCXX_W_W(u, num_neg(u));

    Dispatch();
  }
  CASE(INEG_SCAX) {
    pc++;
    MxcValue u;
    SCAX_W_W(u, num_neg(u));

    Dispatch();
  }
  CASE(INEG_SCBX) {
    pc++;
    MxcValue u;
    SCBX_W_W(u, num_neg(u));

    Dispatch();
  }
  CASE(INEG_SCBA) {
    pc++;
    MxcValue u;
    SCBA_W_W(u, num_neg(u));

    Dispatch();
  }
  CASE(INEG_SCAB) {
    pc++;
    MxcValue u;
    SCAB_W_W(u, num_neg(u));

    Dispatch();
  }
  CASE(FNEG_SCXX) {
    pc++;
    MxcValue u;
    SCXX_W_W(u, mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(FNEG_SCAX) {
    pc++;
    MxcValue u;
    SCAX_W_W(u, mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(FNEG_SCBX) {
    pc++;
    MxcValue u;
    SCBX_W_W(u, mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(FNEG_SCBA) {
    pc++;
    MxcValue u;
    SCBA_W_W(u, mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(FNEG_SCAB) {
    pc++;
    MxcValue u;
    SCAB_W_W(u, mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(NOT_SCXX) {
    pc++;
    MxcValue b;
    SCXX_W_W(b, bool_not(b));

    Dispatch();
  }
  CASE(NOT_SCAX) {
    pc++;
    MxcValue b;
    SCAX_W_W(b, bool_not(b));

    Dispatch();
  }
  CASE(NOT_SCBX) {
    pc++;
    MxcValue b;
    SCBX_W_W(b, bool_not(b));

    Dispatch();
  }
  CASE(NOT_SCBA) {
    pc++;
    MxcValue b;
    SCBA_W_W(b, bool_not(b));

    Dispatch();
  }
  CASE(NOT_SCAB) {
    pc++;
    MxcValue b;
    SCAB_W_W(b, bool_not(b));

    Dispatch();
  }
  CASE(STORE_GLOBAL_SCXX) {
    key = (int)READARG(pc);

    gvmap[key] = TOP();

    pc += 2;
    Dispatch();
  }
  CASE(STORE_GLOBAL_SCAX) {
    key = (int)READARG(pc);

    gvmap[key] = screg_a;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_GLOBAL_SCBX) {
    key = (int)READARG(pc);

    gvmap[key] = screg_b;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_GLOBAL_SCBA) {
    key = (int)READARG(pc);

    gvmap[key] = screg_a;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_GLOBAL_SCAB) {
    key = (int)READARG(pc);

    gvmap[key] = screg_b;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_LOCAL_SCXX) {
    key = (int)READARG(pc);

    context->lvars[key] = TOP();

    pc += 2;
    Dispatch();
  }
  CASE(STORE_LOCAL_SCAX) {
    key = (int)READARG(pc);

    context->lvars[key] = screg_a;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_LOCAL_SCBX) {
    key = (int)READARG(pc);

    context->lvars[key] = screg_b;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_LOCAL_SCBA) {
    key = (int)READARG(pc);

    context->lvars[key] = screg_a;

    pc += 2;
    Dispatch();
  }
  CASE(STORE_LOCAL_SCAB) {
    key = (int)READARG(pc);

    context->lvars[key] = screg_b;

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_GLOBAL_SCXX) {
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    SCXX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_GLOBAL_SCAX) {
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    SCAX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_GLOBAL_SCBX) {
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    SCBX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_GLOBAL_SCBA) {
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    SCBA_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_GLOBAL_SCAB) {
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    SCAB_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_LOCAL_SCXX) {
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    SCXX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_LOCAL_SCAX) {
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    SCAX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_LOCAL_SCBX) {
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    SCBX_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_LOCAL_SCBA) {
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    SCBA_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(LOAD_LOCAL_SCAB) {
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    SCAB_X_W(ob);

    pc += 2;
    Dispatch();
  }
  CASE(JMP_SCXX)
  CASE(JMP_SCAX)
  CASE(JMP_SCBX)
  CASE(JMP_SCBA)
  CASE(JMP_SCAB) {
    int c = (int)READARG(pc);
    pc = &code[c];

    Dispatch();
  }
  CASE(JMP_EQ_SCXX) {
    MxcValue a = POP();
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_EQ_SCAX) {
    MxcValue a = screg_a;
    scstate = SCXX;
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_EQ_SCBX) {
    MxcValue a = screg_b;
    scstate = SCXX;
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_EQ_SCBA) {
    MxcValue a = screg_a;
    scstate = SCBX;
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_EQ_SCAB) {
    MxcValue a = screg_b;
    scstate = SCAX;
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ_SCXX) {
    MxcValue a = POP();
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ_SCAX) {
    MxcValue a = screg_a;
    scstate = SCXX;
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ_SCBX) {
    MxcValue a = screg_b;
    scstate = SCXX;
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ_SCBA) {
    MxcValue a = screg_a;
    scstate = SCBX;
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ_SCAB) {
    MxcValue a = screg_b;
    scstate = SCAX;
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc += 2;
    }

    Dispatch();
  }
  CASE(TRY_SCXX) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(TRY_SCAX) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(TRY_SCBX) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(TRY_SCBA) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(TRY_SCAB) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(CATCH_SCXX) {
    MxcValue top = TOP();

    if(!check_value(top)) {
      pc += 2;
      (void)POP();
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(CATCH_SCAX) {
    MxcValue top = screg_a;

    if(!check_value(top)) {
      pc += 2;
      (void)POP();
      scstate = SCXX;
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(CATCH_SCBX) {
    MxcValue top = screg_b;

    if(!check_value(top)) {
      pc += 2;
      (void)POP();
      scstate = SCXX;
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(CATCH_SCBA) {
    MxcValue top = screg_a;

    if(!check_value(top)) {
      pc += 2;
      (void)POP();
      scstate = SCBX;
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(CATCH_SCAB) {
    MxcValue top = screg_b;

    if(!check_value(top)) {
      pc += 2;
      (void)POP();
      scstate = SCAX;
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(LISTSET_SCXX) {
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    for(int i = 0; i < narg; i++) {
      listadd((MList *)V2O(list), POP());
    }
    PUSH(list);

    pc += 2;
    Dispatch();
  }
  CASE(LISTSET_SCAX) {
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    for(int i = 0; i < narg; i++) {
      if(UNLIKELY(i == 0)) {
        listadd((MList *)V2O(list), screg_a);
        scstate = SCXX;
      }
      else {
        listadd((MList *)V2O(list), POP());
      }
    }

    if(scstate == SCAX) {
      screg_b = list;
      scstate = SCAB;
    }
    else {
      PUSH(list);
      scstate = SCAX;
    }

    pc += 2;
    Dispatch();
  }
  CASE(LISTSET_SCBX) {
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    for(int i = 0; i < narg; i++) {
      if(UNLIKELY(i == 0)) {
        listadd((MList *)V2O(list), screg_b);
        scstate = SCXX;
      }
      else {
        listadd((MList *)V2O(list), POP());
      }
    }

    if(scstate == SCBX) {
      screg_a = list;
      scstate = SCBA;
    }
    else {
      PUSH(list);
      scstate = SCAX;
    }

    pc += 2;
    Dispatch();
  }
  CASE(LISTSET_SCBA) {
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    for(int i = 0; i < narg; i++) {
      if(UNLIKELY(i == 0)) {
        listadd((MList *)V2O(list), screg_a);
        scstate = SCBX;
      }
      if(UNLIKELY(i == 1)) {
        listadd((MList *)V2O(list), screg_b);
        scstate = SCXX;
      }
      else {
        listadd((MList *)V2O(list), POP());
      }
    }

    if(scstate == SCBA) {
      PUSH(screg_b);
      screg_b = list;
      scstate = SCAB;
    }
    else if(scstate == SCBX) {
      screg_a = list;
      scstate = SCBA;
    }
    else {
      PUSH(list);
      scstate = SCAX;
    }

    pc += 2;
    Dispatch();
  }
  CASE(LISTSET_SCAB) {
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    for(int i = 0; i < narg; i++) {
      if(UNLIKELY(i == 0)) {
        listadd((MList *)V2O(list), screg_b);
        scstate = SCAX;
      }
      if(UNLIKELY(i == 1)) {
        listadd((MList *)V2O(list), screg_a);
        scstate = SCXX;
      }
      else {
        listadd((MList *)V2O(list), POP());
      }
    }

    if(scstate == SCAB) {
      PUSH(screg_a);
      screg_a = list;
      scstate = SCBA;
    }
    else if(scstate == SCAX) {
      screg_b = list;
      scstate = SCAB;
    }
    else {
      PUSH(list);
      scstate = SCAX;
    }

    pc += 2;
    Dispatch();
  }
  CASE(LISTSET_SIZE_SCXX) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTSET_SIZE_SCAX) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTSET_SIZE_SCBX) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTSET_SIZE_SCBA) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTSET_SIZE_SCAB) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(SUBSCR_SCXX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(POP());
    MxcValue idx = POP();
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    screg_a = ob;
    scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_SCAX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_a);
    MxcValue idx = POP();
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    screg_a = ob;
    // scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_SCBX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_b);
    MxcValue idx = POP();
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    screg_b = ob;
    // scstate = SCBX;

    Dispatch();
  }
  CASE(SUBSCR_SCBA) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_a);
    MxcValue idx = screg_b;
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    screg_a = ob;
    scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_SCAB) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_b);
    MxcValue idx = screg_a;
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    screg_a = ob;
    scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_STORE_SCXX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(POP());
    MxcValue idx = POP();
    MxcValue top = POP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }
    screg_a = top;
    scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_STORE_SCAX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_a);
    MxcValue idx = POP();
    MxcValue top = POP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }
    screg_a = top;
    // scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_STORE_SCBX) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_b);
    MxcValue idx = POP();
    MxcValue top = POP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }
    screg_b = top;
    // scstate = SCBX;

    Dispatch();
  }
  CASE(SUBSCR_STORE_SCBA) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_a);
    MxcValue idx = screg_b;
    MxcValue top = POP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }
    screg_a = top;
    scstate = SCAX;

    Dispatch();
  }
  CASE(SUBSCR_STORE_SCAB) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(screg_b);
    MxcValue idx = screg_a;
    MxcValue top = POP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }
    screg_a = top;
    scstate = SCAX;

    Dispatch();
  }
  CASE(STRINGSET_SCXX) {
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    SCXX_X_W(new_string_static(str, strlen(str)));

    pc += 2;
    Dispatch();
  }
  CASE(STRINGSET_SCAX) {
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    SCAX_X_W(new_string_static(str, strlen(str)));

    pc += 2;
    Dispatch();
  }
  CASE(STRINGSET_SCBX) {
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    SCBX_X_W(new_string_static(str, strlen(str)));

    pc += 2;
    Dispatch();
  }
  CASE(STRINGSET_SCBA) {
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    SCBA_X_W(new_string_static(str, strlen(str)));

    pc += 2;
    Dispatch();
  }
  CASE(STRINGSET_SCAB) {
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    SCAB_X_W(new_string_static(str, strlen(str)));

    pc += 2;
    Dispatch();
  }
  CASE(FUNCTIONSET_SCXX) {
    key = (int)READARG(pc);
    SCXX_X_W(new_function(lit_table[key]->func, false));

    pc += 2;
    Dispatch();
  }
  CASE(FUNCTIONSET_SCAX) {
    key = (int)READARG(pc);
    SCAX_X_W(new_function(lit_table[key]->func, false));

    pc += 2;
    Dispatch();
  }
  CASE(FUNCTIONSET_SCBX) {
    key = (int)READARG(pc);
    SCBX_X_W(new_function(lit_table[key]->func, false));

    pc += 2;
    Dispatch();
  }
  CASE(FUNCTIONSET_SCBA) {
    key = (int)READARG(pc);
    SCBA_X_W(new_function(lit_table[key]->func, false));

    pc += 2;
    Dispatch();
  }
  CASE(FUNCTIONSET_SCAB) {
    key = (int)READARG(pc);
    SCAB_X_W(new_function(lit_table[key]->func, false));

    pc += 2;
    Dispatch();
  }
  CASE(ITERFN_SET_SCXX) {
    key = (int)READARG(pc);
    SCXX_X_W(new_function(lit_table[key]->func, true));

    pc += 2;
    Dispatch();
  }
  CASE(ITERFN_SET_SCAX) {
    key = (int)READARG(pc);
    SCAX_X_W(new_function(lit_table[key]->func, true));

    pc += 2;
    Dispatch();
  }
  CASE(ITERFN_SET_SCBX) {
    key = (int)READARG(pc);
    SCBX_X_W(new_function(lit_table[key]->func, true));

    pc += 2;
    Dispatch();
  }
  CASE(ITERFN_SET_SCBA) {
    key = (int)READARG(pc);
    SCBA_X_W(new_function(lit_table[key]->func, true));

    pc += 2;
    Dispatch();
  }
  CASE(ITERFN_SET_SCAB) {
    key = (int)READARG(pc);
    SCAB_X_W(new_function(lit_table[key]->func, true));

    pc += 2;
    Dispatch();
  }
  CASE(STRUCTSET_SCXX) {
    int nfield = (int)READARG(pc);
    SCXX_X_W(new_struct(nfield));

    pc += 2;
    Dispatch();
  }
  CASE(STRUCTSET_SCAX) {
    int nfield = (int)READARG(pc);
    SCAX_X_W(new_struct(nfield));

    pc += 2;
    Dispatch();
  }
  CASE(STRUCTSET_SCBX) {
    int nfield = (int)READARG(pc);
    SCBX_X_W(new_struct(nfield));

    pc += 2;
    Dispatch();
  }
  CASE(STRUCTSET_SCBA) {
    int nfield = (int)READARG(pc);
    SCBA_X_W(new_struct(nfield));

    pc += 2;
    Dispatch();
  }
  CASE(STRUCTSET_SCAB) {
    int nfield = (int)READARG(pc);
    SCAB_X_W(new_struct(nfield));

    pc += 2;
    Dispatch();
  }
  CASE(TABLESET_SCXX) {
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    for(int i = 0; i < n; i++) {
      MxcValue v = POP();
      MxcValue k = POP();
      mtable_add((MTable *)V2O(t), k, v);
    }

    screg_a = t;
    scstate = SCAX;

    pc += 2;
    Dispatch();
  }
  CASE(TABLESET_SCAX) {
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    MxcValue v, k;
    for(int i = 0; i < n; i++) {
      if(i == 0) {
        v = screg_a;
        scstate = SCXX;
      }
      else {
        v = POP();
      }
      k = POP();
      mtable_add((MTable *)V2O(t), k, v);
    }

    if(scstate == SCXX) {
      screg_a = t;
      scstate = SCAX;
    }
    else {
      screg_b = t;
      scstate = SCAB;
    }

    pc += 2;
    Dispatch();
  }
  CASE(TABLESET_SCBX) {
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    MxcValue v, k;
    for(int i = 0; i < n; i++) {
      if(i == 0) {
        v = screg_b;
        scstate = SCXX;
      }
      else {
        v = POP();
      }
      k = POP();
      mtable_add((MTable *)V2O(t), k, v);
    }

    if(scstate == SCXX) {
      screg_a = t;
      scstate = SCAX;
    }
    else {
      screg_a = t;
      scstate = SCBA;
    }

    pc += 2;
    Dispatch();
  }
  CASE(TABLESET_SCBA) {
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    MxcValue v, k;
    for(int i = 0; i < n; i++) {
      if(i == 0) {
        v = screg_a;
        k = screg_b;
        scstate = SCXX;
      }
      else {
        v = POP();
        k = POP();
      }
      mtable_add((MTable *)V2O(t), k, v);
    }

    if(scstate == SCXX) {
      screg_a = t;
      scstate = SCAX;
    }
    else {
      PUSH(screg_b);
      screg_b = t;
      scstate = SCAB;
    }

    pc += 2;
    Dispatch();
  }
  CASE(TABLESET_SCAB) {
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    MxcValue v, k;
    for(int i = 0; i < n; i++) {
      if(i == 0) {
        v = screg_b;
        k = screg_a;
        scstate = SCXX;
      }
      else {
        v = POP();
        k = POP();
      }
      mtable_add((MTable *)V2O(t), k, v);
    }

    if(scstate == SCXX) {
      screg_a = t;
      scstate = SCAX;
    }
    else {
      PUSH(screg_a);
      screg_a = t;
      scstate = SCBA;
    }

    pc += 2;
    Dispatch();
  }
  CASE(CALL_SCXX) {
    int nargs = (int)READARG(pc);
    MxcValue callee = POP();
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    pc += 2;
    Dispatch();
  }
  CASE(CALL_SCAX) {
    int nargs = (int)READARG(pc);
    MxcValue callee = screg_a;
    scstate = SCXX;
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    pc += 2;
    Dispatch();
  }
  CASE(CALL_SCBX) {
    int nargs = (int)READARG(pc);
    MxcValue callee = screg_b;
    scstate = SCXX;
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    pc += 2;
    Dispatch();
  }
  CASE(CALL_SCBA) {
    int nargs = (int)READARG(pc);
    MxcValue callee = screg_a;
    scstate = SCBX;
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    pc += 2;
    Dispatch();
  }
  CASE(CALL_SCAB) {
    int nargs = (int)READARG(pc);
    MxcValue callee = screg_b;
    scstate = SCAX;
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_LOAD_SCXX) {
    int offset = (int)READARG(pc);
    MxcValue strct;

    SCXX_W_W(strct, member_getitem(strct, offset));

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_LOAD_SCAX) {
    int offset = (int)READARG(pc);
    MxcValue strct;

    SCAX_W_W(strct, member_getitem(strct, offset));

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_LOAD_SCBX) {
    int offset = (int)READARG(pc);
    MxcValue strct;

    SCBX_W_W(strct, member_getitem(strct, offset));

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_LOAD_SCBA) {
    int offset = (int)READARG(pc);
    MxcValue strct;

    SCBA_W_W(strct, member_getitem(strct, offset));

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_LOAD_SCAB) {
    int offset = (int)READARG(pc);
    MxcValue strct;

    SCAB_W_W(strct, member_getitem(strct, offset));

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_STORE_SCXX) {
    int offset = (int)READARG(pc);
    MxcValue strct = POP();
    MxcValue data = TOP();

    member_setitem(strct, offset, data);

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_STORE_SCAX) {
    int offset = (int)READARG(pc);
    MxcValue strct = screg_a;
    MxcValue data = POP();

    member_setitem(strct, offset, data);

    screg_a = data;
    // scstate = SCAX;

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_STORE_SCBX) {
    int offset = (int)READARG(pc);
    MxcValue strct = screg_b;
    MxcValue data = POP();

    member_setitem(strct, offset, data);

    screg_b = data;
    // scstate = SCBX;

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_STORE_SCBA) {
    int offset = (int)READARG(pc);
    MxcValue strct = screg_a;
    MxcValue data = screg_b;

    member_setitem(strct, offset, data);

    scstate = SCBX;

    pc += 2;
    Dispatch();
  }
  CASE(MEMBER_STORE_SCAB) {
    int offset = (int)READARG(pc);
    MxcValue strct = screg_b;
    MxcValue data = screg_a;

    member_setitem(strct, offset, data);

    scstate = SCAX;

    pc += 2;
    Dispatch();
  }
  CASE(ITER_SCXX) {
    pc++;
    MxcObject *iterable = V2O(POP());
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 

    screg_a = iter;
    scstate = SCAX;

    Dispatch();
  }
  CASE(ITER_SCAX) {
    pc++;
    MxcObject *iterable = V2O(screg_a);
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    screg_a = iter;
    Dispatch();
  }
  CASE(ITER_SCBX) {
    pc++;
    MxcObject *iterable = V2O(screg_b);
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    screg_b = iter;
    Dispatch();
  }
  CASE(ITER_SCBA) {
    pc++;
    MxcObject *iterable = V2O(screg_a);
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    screg_a = iter;
    Dispatch();
  }
  CASE(ITER_SCAB) {
    pc++;
    MxcObject *iterable = V2O(screg_b);
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    screg_b = iter;
    Dispatch();
  }
  CASE(ITER_NEXT_SCXX) {
    MxcValue iter = POP();
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      screg_b = iter;
      screg_a = res;
      scstate = SCBA;
      pc += 2;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  CASE(ITER_NEXT_SCAX) {
    MxcValue iter = screg_a;
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      screg_b = iter;
      screg_a = res;
      scstate = SCBA;
      pc += 2;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  CASE(ITER_NEXT_SCBX) {
    MxcValue iter = screg_b;
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      screg_b = iter;
      screg_a = res;
      scstate = SCBA;
      pc += 2;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  CASE(ITER_NEXT_SCBA) {
    MxcValue iter = screg_a;
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      PUSH(screg_b);
      screg_b = iter;
      screg_a = res;
      scstate = SCBA;
      pc += 2;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  CASE(ITER_NEXT_SCAB) {
    MxcValue iter = screg_b;
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      PUSH(screg_a);
      screg_b = iter;
      screg_a = res;
      scstate = SCBA;
      pc += 2;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  NON_DESTRUCTIVE_CASE(BREAKPOINT) {
    pc++;
    Dispatch();
  }
  CASE(SWITCH_DISPATCH_SCXX) {
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = POP();
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(SWITCH_DISPATCH_SCAX) {
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = screg_a;
    scstate = SCXX;
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(SWITCH_DISPATCH_SCBX) {
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = screg_b;
    scstate = SCXX;
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(SWITCH_DISPATCH_SCBA) {
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = screg_a;
    scstate = SCBX;
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(SWITCH_DISPATCH_SCAB) {
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = screg_b;
    scstate = SCAX;
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(OBJATTR_READ_SCXX) {
    MxcObject *ob = V2O(POP());
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue res;
    switch(ty) {
      case ATTY_CINT: {
        int i = *(int *)(baseaddr + offset);
        res = mval_int(i);
        break;
      }
      case ATTY_CFLOAT: {
        double d = *(double *)(baseaddr + offset);
        res = mval_float(d);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        res = *(MxcValue *)(baseaddr + offset);
        break;
      }
      default:
        unreachable();
    }
    
    screg_a = res;
    scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_READ_SCAX) {
    MxcObject *ob = V2O(screg_a);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue res;
    switch(ty) {
      case ATTY_CINT: {
        int i = *(int *)(baseaddr + offset);
        res = mval_int(i);
        break;
      }
      case ATTY_CFLOAT: {
        double d = *(double *)(baseaddr + offset);
        res = mval_float(d);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        res = *(MxcValue *)(baseaddr + offset);
        break;
      }
      default:
        unreachable();
    }

    screg_a = res;
    // scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_READ_SCBX) {
    MxcObject *ob = V2O(screg_b);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue res;
    switch(ty) {
      case ATTY_CINT: {
        int i = *(int *)(baseaddr + offset);
        res = mval_int(i);
        break;
      }
      case ATTY_CFLOAT: {
        double d = *(double *)(baseaddr + offset);
        res = mval_float(d);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        res = *(MxcValue *)(baseaddr + offset);
        break;
      }
      default:
        unreachable();
    }

    screg_b = res;
    // scstate = SCBX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_READ_SCBA) {
    MxcObject *ob = V2O(screg_a);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue res;
    switch(ty) {
      case ATTY_CINT: {
        int i = *(int *)(baseaddr + offset);
        res = mval_int(i);
        break;
      }
      case ATTY_CFLOAT: {
        double d = *(double *)(baseaddr + offset);
        res = mval_float(d);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        unreachable();
        break;
      }
      case ATTY_MVALUE: {
        res = *(MxcValue *)(baseaddr + offset);
        break;
      }
      default:
        unreachable();
    }

    screg_a = res;
    // scstate = SCBA;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_READ_SCAB) {
    MxcObject *ob = V2O(screg_b);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue res;
    switch(ty) {
      case ATTY_CINT: {
        int i = *(int *)(baseaddr + offset);
        res = mval_int(i);
        break;
      }
      case ATTY_CFLOAT: {
        double d = *(double *)(baseaddr + offset);
        res = mval_float(d);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        unreachable();
        break;
      }
      case ATTY_MVALUE: {
        res = *(MxcValue *)(baseaddr + offset);
        break;
      }
      default:
        unreachable();
    }

    screg_b = res;
    // scstate = SCAB;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_WRITE_SCXX) {
    MxcObject *ob = V2O(POP());
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue v = POP();
    switch(ty) {
      case ATTY_CINT: {
        *(int *)(baseaddr + offset) = V2I(v);
        break;
      }
      case ATTY_CFLOAT: {
        *(double *)(baseaddr + offset) = V2F(v);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        unreachable();
        break;
      }
      case ATTY_MVALUE: {
        *(MxcValue *)(baseaddr + offset) = v;
        break;
      }
      default:
        unreachable();
    }

    screg_a = v;
    scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_WRITE_SCAX) {
    MxcObject *ob = V2O(screg_a);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue v = POP();
    switch(ty) {
      case ATTY_CINT: {
        *(int *)(baseaddr + offset) = V2I(v);
        break;
      }
      case ATTY_CFLOAT: {
        *(double *)(baseaddr + offset) = V2F(v);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        *(MxcValue *)(baseaddr + offset) = v;
        break;
      }
      default:
        unreachable();
    }

    screg_a = v;
    // scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_WRITE_SCBX) {
    MxcObject *ob = V2O(screg_b);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue v = POP();
    switch(ty) {
      case ATTY_CINT: {
        *(int *)(baseaddr + offset) = V2I(v);
        break;
      }
      case ATTY_CFLOAT: {
        *(double *)(baseaddr + offset) = V2F(v);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        *(MxcValue *)(baseaddr + offset) = v;
        break;
      }
      default:
        unreachable();
    }

    screg_b = v;
    // scstate = SCBX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_WRITE_SCBA) {
    MxcObject *ob = V2O(screg_a);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue v = screg_b;
    switch(ty) {
      case ATTY_CINT: {
        *(int *)(baseaddr + offset) = V2I(v);
        break;
      }
      case ATTY_CFLOAT: {
        *(double *)(baseaddr + offset) = V2F(v);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        *(MxcValue *)(baseaddr + offset) = v;
        break;
      }
      default:
        unreachable();
    }

    screg_a = v;
    // scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(OBJATTR_WRITE_SCAB) {
    MxcObject *ob = V2O(screg_b);
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc+1);

    MxcValue v = screg_a;
    switch(ty) {
      case ATTY_CINT: {
        *(int *)(baseaddr + offset) = V2I(v);
        break;
      }
      case ATTY_CFLOAT: {
        *(double *)(baseaddr + offset) = V2F(v);
        break;
      }
      case ATTY_CBOOL: {
        /* TODO */
        break;
      }
      case ATTY_MVALUE: {
        *(MxcValue *)(baseaddr + offset) = v;
        break;
      }
      default:
        unreachable();
    }

    scstate = SCAX;

    pc += 3;
    Dispatch();
  }
  CASE(ASSERT_SCXX) {
    pc++;
    MxcValue top = POP();
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");

    Dispatch();
  }
  CASE(ASSERT_SCAX) {
    pc++;
    MxcValue top = screg_a;
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");
    scstate = SCXX;

    Dispatch();
  }
  CASE(ASSERT_SCBX) {
    pc++;
    MxcValue top = screg_b;
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");
    scstate = SCXX;

    Dispatch();
  }
  CASE(ASSERT_SCBA) {
    pc++;
    MxcValue top = screg_a;
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");
    scstate = SCBX;

    Dispatch();
  }
  CASE(ASSERT_SCAB) {
    pc++;
    MxcValue top = screg_b;
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");
    scstate = SCAX;

    Dispatch();
  }
  NON_DESTRUCTIVE_CASE(RET) {
    pc++;
    return (void *)(intptr_t)0;
  }
  CASE(YIELD_SCXX) {
    pc++;
    MxcValue p = TOP();
    MxcValue v = myield(context, p);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  CASE(YIELD_SCAX) {
    pc++;
    MxcValue p = screg_a;
    MxcValue v = myield(context, p);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  CASE(YIELD_SCBX) {
    pc++;
    MxcValue p = screg_b;
    MxcValue v = myield(context, p);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  CASE(YIELD_SCBA) {
    pc++;
    MxcValue p = screg_a;
    MxcValue v = myield(context, p);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  CASE(YIELD_SCAB) {
    pc++;
    MxcValue p = screg_b;
    MxcValue v = myield(context, p);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  NON_DESTRUCTIVE_CASE(END) {
    /* exit_success */
    return (void *)(intptr_t)0;
  }
  // TODO
  NON_DESTRUCTIVE_CASE(FLOGOR)
  NON_DESTRUCTIVE_CASE(FLOGAND)
  NON_DESTRUCTIVE_CASE(FMOD)
  NON_DESTRUCTIVE_CASE(FGTE) {
    panic("unimplemented instruction");
  }

  ENDOFVM

exit_failure:
  return (void *)(intptr_t)1;
}

void stack_dump(char *label) {
  VM *vm = curvm();
  MxcValue *base = vm->stackbase;
  MxcValue *cur = vm->stackptr;
  MxcValue ob;
  printf("---%s stack---\n", label);
  while(base < cur) {
    ob = *--cur;
    printf("%s\n", ostr(mval2str(ob))->str);
  }
  puts("-----------");
}
