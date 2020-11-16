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

#define DIRECT_THREADED

int error_flag = 0;

#ifdef DIRECT_THREADED
#define Start() do { goto *optable[*context->pc]; } while(0)
#define Dispatch() do { goto *optable[*context->pc]; } while(0)
#define CASE(op) OP_ ## op:
#define ENDOFVM
#else
#define Start() for(;;) { switch(*context->pc) {
#define Dispatch() break
#define CASE(op) case OP_ ## op:
#define ENDOFVM }}
#endif

#define list_setitem(val, index, item) (olist(val)->elem[(index)] = (item))

#define member_getitem(ob, offset)       (ostrct(ob)->field[(offset)])
#define member_setitem(ob, offset, item) (ostrct(ob)->field[(offset)] = (item))

#define PEEK_i32(pc)  \
  ((uint8_t)(pc)[3]<<24|(uint8_t)(pc)[2]<<16|   \
   (uint8_t)(pc)[1]<<8 |(uint8_t)(pc)[0])
#define READ_i32(pc) (pc += 4, PEEK_i32(pc - 4))
#define PEEK_i8(pc) (*(pc))
#define READ_i8(pc) (PEEK_i8(pc++))

VM gvm;
extern clock_t gc_time;

void vm_open(uint8_t *code, MxcValue *gvars, int ngvars, Vector *ltab, DebugInfo *d) {
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
    ret = vm_exec();
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

int vm_exec() {
#ifdef DIRECT_THREADED
  static const void *optable[] = {
#define OPCODE_DEF(op) &&OP_ ## op,
#include "opcode-def.h"
#undef OPCODE_DEF
  };
#endif

  VM *vm = curvm();
  MContext *context = vm->ctx;

  MxcValue *gvmap = vm->gvars;
  uint8_t *code = context->code;
  Literal **lit_table = (Literal **)vm->ltable->data;
  int key;

  Start();

  CASE(PUSH) {
    context->pc++;
    key = READ_i32(context->pc); 
    MxcValue ob = lit_table[key]->raw;
    PUSH(ob);
    INCREF(ob);

    Dispatch();
  }
  CASE(IPUSH) {
    context->pc++;
    PUSH(mval_int(READ_i32(context->pc)));

    Dispatch();
  }
  CASE(LPUSH) {
    context->pc++;
    key = READ_i32(context->pc);
    PUSH(mval_int(lit_table[key]->lnum));

    Dispatch();
  }
  CASE(PUSHCONST_0) {
    context->pc++;
    MxcValue ob = mval_int(0);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_1) {
    context->pc++;
    MxcValue ob = mval_int(1);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_2) {
    context->pc++;
    MxcValue ob = mval_int(2);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHCONST_3) {
    context->pc++;
    MxcValue ob = mval_int(3);
    PUSH(ob);

    Dispatch();
  }
  CASE(PUSHTRUE) {
    context->pc++;
    PUSH(mval_true);

    Dispatch();
  }
  CASE(PUSHFALSE) {
    context->pc++;
    PUSH(mval_false);

    Dispatch();
  }
  CASE(PUSHNULL) {
    context->pc++;
    PUSH(mval_null);

    Dispatch();
  }
  CASE(FPUSH){
    context->pc++;
    key = READ_i32(context->pc);
    PUSH(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(POP) {
    context->pc++;
    (void)POP();

    Dispatch();
  }
  CASE(ADD) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_add(l, r));

    Dispatch();
  }
  CASE(FADD) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatAdd(l, r));

    Dispatch();
  }
  CASE(STRCAT) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(SUB) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_sub(l, r));

    Dispatch();
  }
  CASE(FSUB) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatSub(l, r));

    Dispatch();
  }
  CASE(MUL) {
    context->pc++; // mul
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_mul(l, r));

    Dispatch();
  }
  CASE(FMUL) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(FloatMul(l, r));

    Dispatch();
  }
  CASE(DIV) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = num_div(l, r);
    SETTOP(res);

    Dispatch();
  }
  CASE(FDIV) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = float_div(l, r);
    SETTOP(res);

    Dispatch();
  }
  CASE(MOD) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    MxcValue res = num_mod(l, r);
    SETTOP(res);

    Dispatch();
  }
  CASE(LOGOR) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGAND) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(bool_logand(l, r));

    Dispatch();
  }
  CASE(BXOR) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(IntXor(l, r));

    Dispatch();
  }
  CASE(EQ) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_eq(l, r));

    Dispatch();
  }
  CASE(FEQ) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_eq(l, r));

    Dispatch();
  }
  CASE(NOTEQ) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(num_noteq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_neq(l, r));

    Dispatch();
  }
  CASE(LT) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_lt(l, r));

    Dispatch();
  }
  CASE(FLT) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_lt(l, r));

    Dispatch();
  }
  CASE(LTE) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_lte(l, r));

    Dispatch();
  }
  CASE(FLTE) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_lt(l, r));

    DECREF(r);
    DECREF(l);

    Dispatch();
  }
  CASE(GT) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_gt(l, r));

    Dispatch();
  }
  CASE(FGT) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(float_gt(l, r));

    Dispatch();
  }
  CASE(GTE) {
    context->pc++;
    MxcValue r = POP();
    MxcValue l = TOP();
    SETTOP(int_gte(l, r));

    Dispatch();
  }
  CASE(INEG) {
    context->pc++;
    MxcValue u = TOP();
    SETTOP(num_neg(u));

    Dispatch();
  }
  CASE(FNEG) {
    context->pc++;
    MxcValue u = TOP();
    SETTOP(mval_float(-(u.fnum)));

    Dispatch();
  }
  CASE(NOT) {
    context->pc++;
    MxcValue b = TOP();
    SETTOP(bool_not(b));

    Dispatch();
  }
  CASE(STORE_GLOBAL) {
    context->pc++;
    key = READ_i32(context->pc);

    gvmap[key] = TOP();

    Dispatch();
  }
  CASE(STORE_LOCAL) {
    context->pc++;
    key = READ_i32(context->pc);

    context->lvars[key] = TOP();

    Dispatch();
  }
  CASE(LOAD_GLOBAL) {
    context->pc++;
    key = READ_i32(context->pc);
    MxcValue ob = gvmap[key];
    INCREF(ob);
    PUSH(ob);

    Dispatch();
  }
  CASE(LOAD_LOCAL) {
    context->pc++;
    key = READ_i32(context->pc);
    MxcValue ob = context->lvars[key];
    INCREF(ob);
    PUSH(ob);

    Dispatch();
  }
  CASE(JMP) {
    context->pc++;
    int c = READ_i32(context->pc);
    context->pc = &code[c];

    Dispatch();
  }
  CASE(JMP_EQ) {
    context->pc++;
    MxcValue a = POP();
    if(a.num) {
      int c = READ_i32(context->pc);
      context->pc = &code[c];
    }
    else {
      context->pc += 4;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ) {
    context->pc++;
    MxcValue a = POP();
    if(!a.num) {
      int c = READ_i32(context->pc);
      context->pc = &code[c];
    }
    else {
      context->pc += 4;
    }

    Dispatch();
  }
  CASE(TRY) {
    context->pc++;
    context->err_handling_enabled++;
    Dispatch();
  }
  CASE(CATCH) {
    context->pc++;
    MxcValue top = TOP();

    if(!check_value(top)) {
      context->pc += 4;
      (void)POP();
    }
    else {
      int p = READ_i32(context->pc);
      context->pc = &code[p];
    }

    context->err_handling_enabled--;

    Dispatch();
  }
  CASE(LISTSET) {
    context->pc++;
    int narg = READ_i32(context->pc);
    MxcValue list = new_list(narg);
    while(--narg >= 0) {
      listadd((MList *)V2O(list), POP());
    }
    PUSH(list);

    Dispatch();
  }
  CASE(LISTSET_SIZE) {
    context->pc++;
    MxcValue n = POP();
    MxcValue init = POP();
    MxcValue ob = new_list_size(n, init);
    PUSH(ob);

    Dispatch();
  }
  CASE(LISTLENGTH) {
    context->pc++;
    MxcValue ls = POP();
    PUSH(mval_int(ITERABLE(olist(ls))->length));

    Dispatch();
  }
  CASE(SUBSCR) {
    context->pc++;
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
    context->pc++;
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
    context->pc++;
    key = READ_i32(context->pc);
    char *str = lit_table[key]->str;
    PUSH(new_string_static(str, strlen(str)));

    Dispatch();
  }
  CASE(TUPLESET) {
    context->pc++;
    Dispatch();
  }
  CASE(FUNCTIONSET) {
    context->pc++;
    key = READ_i32(context->pc);
    PUSH(new_function(lit_table[key]->func, false));

    Dispatch();
  }
  CASE(ITERFN_SET) {
    context->pc++;
    key = READ_i32(context->pc);
    PUSH(new_function(lit_table[key]->func, true));
    Dispatch();
  }
  CASE(STRUCTSET) {
    context->pc++;
    int nfield = READ_i32(context->pc);
    PUSH(new_struct(nfield));

    Dispatch();
  }
  CASE(TABLESET) {
    context->pc++;
    int n = READ_i32(context->pc);
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
    context->pc++;
    int nargs = READ_i32(context->pc);
    MxcValue callee = POP();
    int ret = ocallee(callee)->call(ocallee(callee), context, nargs);
    if(ret)
      goto exit_failure;

    Dispatch();
  }
  CASE(MEMBER_LOAD) {
    context->pc++;
    int offset = READ_i32(context->pc);
    MxcValue strct = POP();
    MxcValue data = member_getitem(strct, offset);

    PUSH(data);

    Dispatch();
  }
  CASE(MEMBER_STORE) {
    context->pc++;
    int offset = READ_i32(context->pc);
    MxcValue strct = POP();
    MxcValue data = TOP();

    member_setitem(strct, offset, data);

    Dispatch();
  }
  CASE(ITER) {
    context->pc++;
    MxcObject *iterable = TOP().obj;
    MxcValue iter = SYSTEM(iterable)->getiter(iterable); 
    SETTOP(iter);
    Dispatch();
  }
  CASE(ITER_NEXT) {
    context->pc++;
    MxcValue iter = POP();
    MxcObject *iter_ob = V2O(iter);

    MxcValue res = SYSTEM(iter_ob)->iter_next(iter_ob);
    if(check_value(res)) {
      PUSH(iter);
      PUSH(res);
      context->pc += 4;
    }
    else {
      int c = READ_i32(context->pc);
      context->pc = &code[c];
    }

    Dispatch();
  }
  CASE(BREAKPOINT) {
    context->pc++;
    Dispatch();
  }
  CASE(ASSERT) {
    context->pc++;
    MxcValue top = POP();
    if(!top.num)
      mxc_raise(EXC_ASSERT, "assertion failed");

    Dispatch();
  }
  CASE(RET) {
    context->pc++;
    return 0;
  }
  CASE(YIELD) {
    context->pc++;
    MxcValue p = TOP();
    MxcValue v = fiber_yield(context, &p, 1);
    return 1; // make a distinction from RET
  }
  CASE(END) {
    /* exit_success */
    return 0;
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
  return 1;
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
