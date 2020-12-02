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

  Start();

  CASE(PUSH) {
    pc++;
    key = (int)READARG(pc); 
    MxcValue ob = lit_table[key]->raw;
    PUSH(ob);
    INCREF(ob);

    Dispatch();
  }
  CASE(IPUSH) {
    pc++;
    PUSH(mval_int(READARG(pc)));

    Dispatch();
  }
  CASE(LPUSH) {
    pc++;
    key = (int)READARG(pc);
    PUSH(mval_int(lit_table[key]->lnum));

    Dispatch();
  }
  CASE(PUSHCONST_0) {
    pc++;
    MxcValue ob = mval_int(0);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_1) {
    pc++;
    MxcValue ob = mval_int(1);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_2) {
    pc++;
    MxcValue ob = mval_int(2);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_3) {
    pc++;
    MxcValue ob = mval_int(3);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHTRUE) {
    pc++;
    PUSH(mval_true);

    Dispatch();
  }
  CASE(PUSHFALSE) {
    pc++;
    PUSH(mval_false);

    Dispatch();
  }
  CASE(PUSHNULL) {
    pc++;
    PUSH(mval_null);

    Dispatch();
  }
  CASE(FPUSH){
    pc++;
    key = (int)READARG(pc);
    PUSH(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(POP) {
    pc++;
    (void)POP();

    Dispatch();
  }
  CASE(ADD) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_add(l, r));

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
  CASE(SUB) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_sub(l, r));

    Dispatch();
  }
  CASE(FSUB) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatSub(l, r));

    Dispatch();
  }
  CASE(MUL) {
    pc++; // mul
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_mul(l, r));

    Dispatch();
  }
  CASE(FMUL) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatMul(l, r));

    Dispatch();
  }
  CASE(DIV) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = num_div(l, r);
    SETTOP(res);

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
  CASE(MOD) {
    pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = num_mod(l, r);
    SETTOP(res);

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
    SETTOP(IntXor(l, r));

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
    SETTOP(mval_float(-(u.fnum)));

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
    if(a.num) {
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
    if(!a.num) {
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
    MxcObject *iterable = TOP().obj;
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
  CASE(ASSERT) {
    pc++;
    MxcValue top = POP();
    if(!top.num)
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
