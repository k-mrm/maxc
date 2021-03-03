#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "vm.h"
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
#define CASE(op) OP_ ## op:
#define ENDOFVM
#else
#define Start() for(;;) { switch(*pc) {
#define Dispatch() break
#define CASE(op) case OP_ ## op:
#define ENDOFVM }}
#endif

#define list_setitem(val, index, item) (olist(val)->elem[(index)] = (item))

#define member_getitem(ob, offset)       (ostrct(ob)->field[(offset)])
#define member_setitem(ob, offset, item) (ostrct(ob)->field[(offset)] = (item))

#define PEEKARG(pc) ((smptr_t)*(pc))
#define READARG(pc) (PEEKARG(pc++))

VM gvm;
extern clock_t gc_time;

mptr_t **pcsaver;

/* state of stack cache */
enum scstate {
  SCXX,
  SCAX,
  SCBX,
  SCBA,
  SCAB,
};

enum scstate scstate = SCXX;

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

#define SCXX_WW_W(ob) \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAX_WW_W(ob) \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBX_WW_W(ob) \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCBA_WW_W(ob) \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)
#define SCAB_WW_W(ob) \
  do {  \
    screg_a = (ob); \
    scstate = SCAX; \
  } while(0)

void vm_open(mptr_t *code, MxcValue *gvars, int ngvars, Vector *ltab, DebugInfo *d) {
  VM *vm = curvm();
  vm->ctx = new_econtext(code, 0, d, NULL);
  vm->gvars = gvars;
  vm->ngvars = ngvars;
  vm->stackptr = calloc(1, sizeof(MxcValue) * 1024);
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

void *vm_exec(VM *vm) {
#ifdef DIRECT_THREADED
  static void *optable[] = {
#define OPCODE_DEF(op) &&OP_ ## op,
#include "opcode-def.h"
#undef OPCODE_DEF
  };
#endif

  if(UNLIKELY(!vm)) {
    /* get optable */
    return (void *)optable;
  }

  MContext *context = vm->ctx;

  MxcValue *gvmap = vm->gvars;
  mptr_t *code = context->code;
  mptr_t *pc = context->pc;
  pcsaver = &pc;
  Literal **lit_table = (Literal **)vm->ltable->data;
  int key;

  register MxcValue screg_a;
  register MxcValue screg_b;

  Start();

  CASE(PUSH_SCXX) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCXX_X_W(ob);

    Dispatch();
  }
  CASE(PUSH_SCAX) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;
    SCAX_X_W(ob);

    Dispatch();
  }
  CASE(PUSH_SCBX) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;
    
    SCBX_X_W(ob);

    Dispatch();
  }
  CASE(PUSH_SCBA) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCBA_X_W(ob);

    Dispatch();
  }
  CASE(PUSH_SCAB) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;

    SCAB_X_W(ob);

    Dispatch();
  }
  CASE(IPUSH_SCXX) {
    pc++;
    SCXX_X_W(mval_int(READARG(pc)));

    Dispatch();
  }
  CASE(IPUSH_SCAX) {
    pc++;
    SCAX_X_W(mval_int(READARG(pc)));

    Dispatch();
  }
  CASE(IPUSH_SCBX) {
    pc++;
    SCBX_X_W(mval_int(READARG(pc)));

    Dispatch();
  }
  CASE(IPUSH_SCBA) {
    pc++;
    SCBA_X_W(mval_int(READARG(pc)));

    Dispatch();
  }
  CASE(IPUSH_SCAB) {
    pc++;
    SCAB_X_W(mval_int(READARG(pc)));

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
    pc++;
    key = (int)READARG(pc);
    SCXX_X_W(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(FPUSH_SCAX){
    pc++;
    key = (int)READARG(pc);
    SCAX_X_W(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(FPUSH_SCBX){
    pc++;
    key = (int)READARG(pc);
    SCBX_X_W(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(FPUSH_SCBA){
    pc++;
    key = (int)READARG(pc);
    SCBA_X_W(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(FPUSH_SCAB){
    pc++;
    key = (int)READARG(pc);
    SCAB_X_W(mval_float(lit_table[key]->fnumber));

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
    MxcValue r = POP();
    MxcValue l = POP();
    SCXX_WW_W(num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCAX) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = POP();
    SCAX_WW_W(num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCBX) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = POP();
    SCBX_WW_W(num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCBA) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = screg_b;
    SCBA_WW_W(num_add(l, r));

    Dispatch();
  }
  CASE(ADD_SCAB) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = screg_a;
    SCAB_WW_W(num_add(l, r));

    Dispatch();
  }
  CASE(FADD) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatAdd(l, r));

    Dispatch();
  }
  CASE(STRCAT) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(SUB_SCXX) {
    pc++;
    MxcValue r = POP();
    MxcValue l = POP();
    SCXX_WW_W(num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCAX) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = POP();
    SCAX_WW_W(num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCBX) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = POP();
    SCBX_WW_W(num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCBA) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = screg_b;
    SCBA_WW_W(num_sub(l, r));

    Dispatch();
  }
  CASE(SUB_SCAB) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = screg_a;
    SCAB_WW_W(num_sub(l, r));

    Dispatch();
  }
  CASE(FSUB) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatSub(l, r));

    Dispatch();
  }
  CASE(MUL_SCXX) {
    pc++;
    MxcValue r = POP();
    MxcValue l = POP();
    SCXX_WW_W(num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCAX) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = POP();
    SCAX_WW_W(num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCBX) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = POP();
    SCBX_WW_W(num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCBA) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = screg_b;
    SCBA_WW_W(num_mul(l, r));

    Dispatch();
  }
  CASE(MUL_SCAB) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = screg_a;
    SCAB_WW_W(num_mul(l, r));

    Dispatch();
  }
  CASE(FMUL) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatMul(l, r));

    Dispatch();
  }
  CASE(DIV_SCXX) {
    pc++;
    MxcValue r = POP();
    MxcValue l = POP();
    SCXX_WW_W(num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCAX) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = POP();
    SCAX_WW_W(num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCBX) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = POP();
    SCBX_WW_W(num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCBA) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = screg_b;
    SCBA_WW_W(num_div(l, r));

    Dispatch();
  }
  CASE(DIV_SCAB) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = screg_a;
    SCAB_WW_W(num_div(l, r));

    Dispatch();
  }
  CASE(FDIV) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = float_div(l, r);
    SETTOP(res);

    Dispatch();
  }
  CASE(MOD_SCXX) {
    pc++;
    MxcValue r = POP();
    MxcValue l = POP();
    SCXX_WW_W(num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCAX) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = POP();
    SCAX_WW_W(num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCBX) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = POP();
    SCBX_WW_W(num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCBA) {
    pc++;
    MxcValue r = screg_a;
    MxcValue l = screg_b;
    SCBA_WW_W(num_mod(l, r));

    Dispatch();
  }
  CASE(MOD_SCAB) {
    pc++;
    MxcValue r = screg_b;
    MxcValue l = screg_a;
    SCAB_WW_W(num_mod(l, r));

    Dispatch();
  }
  CASE(LOGOR) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGAND) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(bool_logand(l, r));

    Dispatch();
  }
  CASE(BXOR) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_xor(l, r));

    Dispatch();
  }
  CASE(EQ) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_eq(l, r));

    Dispatch();
  }
  CASE(FEQ) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_eq(l, r));

    Dispatch();
  }
  CASE(NOTEQ) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_noteq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_neq(l, r));

    Dispatch();
  }
  CASE(LT) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_lt(l, r));

    Dispatch();
  }
  CASE(FLT) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_lt(l, r));

    Dispatch();
  }
  CASE(LTE) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_lte(l, r));

    Dispatch();
  }
  CASE(FLTE) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_lt(l, r));

    DECREF(r);
    DECREF(l);

    Dispatch();
  }
  CASE(GT) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_gt(l, r));

    Dispatch();
  }
  CASE(FGT) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_gt(l, r));

    Dispatch();
  }
  CASE(GTE) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_gte(l, r));

    Dispatch();
  }
  CASE(INEG) {
    pc++;
    MxcValue u = TOP();
    SETTOP(num_neg(u));

    Dispatch();
  }
  CASE(FNEG) {
    pc++;
    MxcValue u = TOP();
    SETTOP(mval_float(-(V2F(u))));

    Dispatch();
  }
  CASE(NOT) {
    pc++;
    MxcValue b = TOP();
    SETTOP(bool_not(b));

    Dispatch();
  }
  CASE(STORE_GLOBAL) {
    pc++;
    key = (int)READARG(pc);

    gvmap[key] = TOP();

    Dispatch();
  }
  CASE(STORE_LOCAL) {
    pc++;
    key = (int)READARG(pc);

    context->lvars[key] = TOP();

    Dispatch();
  }
  CASE(LOAD_GLOBAL) {
    pc++;
    key = (int)READARG(pc);
    MxcValue ob = gvmap[key];
    INCREF(ob);
    PUSH(ob);

    Dispatch();
  }
  CASE(LOAD_LOCAL) {
    pc++;
    key = (int)READARG(pc);
    MxcValue ob = context->lvars[key];
    INCREF(ob);
    PUSH(ob);

    Dispatch();
  }
  CASE(JMP) {
    pc++;
    int c = (int)READARG(pc);
    pc = &code[c];

    Dispatch();
  }
  CASE(JMP_EQ) {
    pc++;
    MxcValue a = POP();
    if(V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc++;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ) {
    pc++;
    MxcValue a = POP();
    if(!V2I(a)) {
      int c = (int)READARG(pc);
      pc = &code[c];
    }
    else {
      pc++;
    }

    Dispatch();
  }
  CASE(TRY) {
    pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(CATCH) {
    pc++;
    MxcValue top = TOP();

    if(!check_value(top)) {
      pc++;
      (void)POP();
    }
    else {
      int p = (int)READARG(pc);
      pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(LISTSET) {
    pc++;
    int narg = (int)READARG(pc);
    MxcValue list = new_list(narg);
    while(--narg >= 0) {
      listadd((MList *)V2O(list), POP());
    }
    PUSH(list);

    Dispatch();
  }
  CASE(LISTSET_SIZE) {
    pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTLENGTH) {
    pc++;
    MxcValue ls = POP();
    PUSH(mval_int(ITERABLE(olist(ls))->length));

    Dispatch();
  }
  CASE(SUBSCR) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(POP());
    MxcValue idx = TOP();
    MxcValue ob = SYSTEM(ls)->get(ls, idx);
    if(!check_value(ob)) {
      goto exit_failure;
    }
    SETTOP(ob);

    Dispatch();
  }
  CASE(SUBSCR_STORE) {
    pc++;
    MxcIterable *ls = (MxcIterable *)olist(POP());
    MxcValue idx = POP();
    MxcValue top = TOP();
    MxcValue res = SYSTEM(ls)->set(ls, idx, top);
    if(!check_value(res)) {
      goto exit_failure;
    }

    Dispatch();
  }
  CASE(STRINGSET) {
    pc++;
    key = (int)READARG(pc);
    char *str = lit_table[key]->str;
    PUSH(new_string_static(str, strlen(str)));

    Dispatch();
  }
  CASE(TUPLESET) {
    pc++;
    Dispatch();
  }
  CASE(FUNCTIONSET) {
    pc++;
    key = (int)READARG(pc);
    PUSH(new_function(lit_table[key]->func, false));

    Dispatch();
  }
  CASE(ITERFN_SET) {
    pc++;
    key = (int)READARG(pc);
    PUSH(new_function(lit_table[key]->func, true));
    Dispatch();
  }
  CASE(STRUCTSET) {
    pc++;
    int nfield = (int)READARG(pc);
    PUSH(new_struct(nfield));

    Dispatch();
  }
  CASE(TABLESET) {
    pc++;
    int n = (int)READARG(pc);
    MxcValue t = new_table_capa(n);
    for(int i = 0; i < n; i++) {
      MxcValue v = POP();
      MxcValue k = POP();
      mtable_add((MTable *)V2O(t), k, v);
    }

    PUSH(t);

    Dispatch();
  }
  CASE(CALL) {
    pc++;
    int nargs = (int)READARG(pc);
    MxcValue callee = POP();
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    Dispatch();
  }
  CASE(MEMBER_LOAD) {
    pc++;
    int offset = (int)READARG(pc);
    MxcValue strct = POP();
    MxcValue data = member_getitem(strct, offset);

    PUSH(data);

    Dispatch();
  }
  CASE(MEMBER_STORE) {
    pc++;
    int offset = (int)READARG(pc);
    MxcValue strct = POP();
    MxcValue data = TOP();

    member_setitem(strct, offset, data);

    Dispatch();
  }
  CASE(ITER) {
    pc++;
    MxcObject *iterable = V2O(TOP());
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    SETTOP(iter);
    Dispatch();
  }
  CASE(ITER_NEXT) {
    pc++;
    MxcValue iter = POP();
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      PUSH(iter);
      PUSH(res);
      pc++;
    }
    else {
      int c = (int)READARG(pc);
      pc = &code[c];
    }

    Dispatch();
  }
  CASE(BREAKPOINT) {
    pc++;
    Dispatch();
  }
  CASE(SWITCH_DISPATCH) {
    pc++;
    MTable *dispatch_tab = (MTable *)READARG(pc);
    MxcValue v = POP();
    MxcValue jmp = SYSTEM(dispatch_tab)->get((MxcIterable *)dispatch_tab, v);
    pc = &code[V2I(jmp)];

    Dispatch();
  }
  CASE(OBJATTR_READ) {
    pc++;

    MxcObject *ob = V2O(POP());
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc);

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

    PUSH(res);

    Dispatch();
  }
  CASE(OBJATTR_WRITE) {
    pc++;

    MxcObject *ob = V2O(POP());
    char *baseaddr = (char *)ob;
    size_t offset = READARG(pc);
    enum attr_type ty = READARG(pc);

    MxcValue v = TOP();
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

    Dispatch();
  }
  CASE(ASSERT) {
    pc++;
    MxcValue top = POP();
    if(!V2I(top))
      mxc_raise(EXC_ASSERT, "assertion failed");

    Dispatch();
  }
  CASE(RET) {
    pc++;
    return (void *)(intptr_t)0;
  }
  CASE(YIELD) {
    pc++;
    MxcValue p = TOP();
    MxcValue v = fiber_yield(context, &p, 1);
    context->pc = pc;
    return (void *)(intptr_t)1; // make a distinction from RET
  }
  CASE(END) {
    /* exit_success */
    return (void *)(intptr_t)0;
  }
  // TODO
  CASE(FLOGOR)
  CASE(FLOGAND)
  CASE(FMOD)
  CASE(FGTE) {
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
