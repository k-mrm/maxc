#include <string.h>
#include <time.h>

#include "vm.h"
#include "ast.h"
#include "bytecode.h"
#include "error/error.h"
#include "error/runtime-err.h"
#include "literalpool.h"
#include "debug.h"
#include "mem.h"
#include "gc.h"
#include "object/object.h"
#include "object/boolobject.h"
#include "object/charobject.h"
#include "object/floatobject.h"
#include "object/funcobject.h"
#include "object/intobject.h"
#include "object/num.h"
#include "object/listobject.h"
#include "object/strobject.h"
#include "object/mfiber.h"

// #define DPTEST

int error_flag = 0;

#ifndef DPTEST
#define Dispatch() do { goto *optable[*pc]; } while(0)
#else
#define DISPATCH_CASE(name, smallname)                                         \
  case OP_##name:                                                            \
goto code_##smallname;

#define Dispatch()                                                             \
  do {                                                                       \
    switch(*pc) {                                       \
      DISPATCH_CASE(END, end)                                            \
      DISPATCH_CASE(IPUSH, ipush)                                        \
      DISPATCH_CASE(CPUSH, cpush)                                        \
      DISPATCH_CASE(FPUSH, fpush)                                        \
      DISPATCH_CASE(LPUSH, lpush)                                        \
      DISPATCH_CASE(LOAD_GLOBAL, load_global)                            \
      DISPATCH_CASE(LOAD_LOCAL, load_local)                              \
      DISPATCH_CASE(RET, ret)                                            \
      DISPATCH_CASE(STORE_LOCAL, store_local)                            \
      DISPATCH_CASE(STORE_GLOBAL, store_global)                          \
      DISPATCH_CASE(CALL, call)                                          \
      DISPATCH_CASE(PUSHCONST_0, pushconst_0)                            \
      DISPATCH_CASE(PUSHCONST_1, pushconst_1)                            \
      DISPATCH_CASE(PUSHCONST_2, pushconst_2)                            \
      DISPATCH_CASE(PUSHCONST_3, pushconst_3)                            \
      DISPATCH_CASE(PUSHTRUE, pushtrue)                                  \
      DISPATCH_CASE(PUSHFALSE, pushfalse)                                \
      DISPATCH_CASE(PUSHNULL, pushnull)                                  \
      DISPATCH_CASE(LTE, lte)                                            \
      DISPATCH_CASE(LT, lt)                                              \
      DISPATCH_CASE(GT, gt)                                              \
      DISPATCH_CASE(GTE, gte)                                            \
      DISPATCH_CASE(EQ, eq)                                              \
      DISPATCH_CASE(FEQ, feq)                                            \
      DISPATCH_CASE(NOTEQ, noteq)                                        \
      DISPATCH_CASE(FNOTEQ, fnoteq)                                      \
      DISPATCH_CASE(JMP_NOTEQ, jmp_noteq)                                \
      DISPATCH_CASE(JMP_EQ, jmp_eq)                                \
      DISPATCH_CASE(JMP, jmp)                                            \
      DISPATCH_CASE(JMP_NOTERR, jmp_noterr)                              \
      DISPATCH_CASE(SUB, sub)                                            \
      DISPATCH_CASE(ADD, add)                                            \
      DISPATCH_CASE(MUL, mul)                                            \
      DISPATCH_CASE(DIV, div)                                            \
      DISPATCH_CASE(MOD, mod)                                            \
      DISPATCH_CASE(FADD, fadd)                                          \
      DISPATCH_CASE(FSUB, fsub)                                          \
      DISPATCH_CASE(FMUL, fmul)                                          \
      DISPATCH_CASE(FDIV, fdiv)                                          \
      DISPATCH_CASE(FMOD, fmod)                                          \
      DISPATCH_CASE(FLT, flt)                                            \
      DISPATCH_CASE(FLTE, flte)                                          \
      DISPATCH_CASE(FGT, fgt)                                            \
      DISPATCH_CASE(FGTE, fgte)                                          \
      DISPATCH_CASE(INC, inc)                                            \
      DISPATCH_CASE(DEC, dec)                                            \
      DISPATCH_CASE(NOT, not)                                            \
      DISPATCH_CASE(INEG, ineg)                                          \
      DISPATCH_CASE(FNEG, fneg)                                          \
      DISPATCH_CASE(LOGAND, logand)                                      \
      DISPATCH_CASE(LOGOR, logor)                                        \
      DISPATCH_CASE(FLOGAND, flogand)                                    \
      DISPATCH_CASE(FLOGOR, flogor)                                      \
      DISPATCH_CASE(BLTINFN_SET, bltinfnset)                             \
      DISPATCH_CASE(CALL_BLTIN, call_bltin)                              \
      DISPATCH_CASE(POP, pop)                                            \
      DISPATCH_CASE(STRINGSET, stringset)                                \
      DISPATCH_CASE(SUBSCR, subscr)                                      \
      DISPATCH_CASE(SUBSCR_STORE, subscr_store)                          \
      DISPATCH_CASE(STRUCTSET, structset)                                \
      DISPATCH_CASE(LISTSET, listset)                                    \
      DISPATCH_CASE(LISTSET_SIZE, listset_size)                          \
      DISPATCH_CASE(LISTLENGTH, listlength)                              \
      DISPATCH_CASE(FUNCTIONSET, functionset)                            \
      DISPATCH_CASE(TUPLESET, tupleset)                                  \
      DISPATCH_CASE(MEMBER_LOAD, member_load)                            \
      DISPATCH_CASE(MEMBER_STORE, member_store)                          \
      DISPATCH_CASE(ITER_NEXT, iter_next)                                \
      DISPATCH_CASE(STRCAT, strcat)                                      \
      DISPATCH_CASE(BREAKPOINT, breakpoint)                              \
      default:                                                               \
                                                                             printf("err:%d\n", *pc);                        \
      mxc_raise_err(frame, RTERR_UNIMPLEMENTED);                         \
    }                                                                      \
  } while(0)
#endif

#define List_Setitem(val, index, item) (olist(val)->elem[(index)] = (item))

#define Member_Getitem(ob, offset)       (ostrct(ob)->field[(offset)])
#define Member_Setitem(ob, offset, item) (ostrct(ob)->field[(offset)] = (item))

#define PEEK_i32(pc)  \
  ((uint8_t)(pc)[3]<<24|(uint8_t)(pc)[2]<<16|   \
   (uint8_t)(pc)[1]<<8 |(uint8_t)(pc)[0])
#define READ_i32(pc) (pc += 4, PEEK_i32(pc - 4))
#define PEEK_i8(pc) (*(pc))
#define READ_i8(pc) (PEEK_i8(pc++))

#define CASE(op) OP_ ## op:

MContext *cur_frame;
extern clock_t gc_time;

int vm_run(MContext *frame) {
#ifdef MXC_DEBUG
  printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

  int ret = vm_exec(frame);

#ifdef MXC_DEBUG
  printf("GC time: %.5lf\n", (double)(gc_time) / CLOCKS_PER_SEC);
  printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

  return ret;
}

int vm_exec(MContext *frame) {
#ifndef DPTEST
  static const void *optable[] = {
#define OPCODE_DEF(op) &&OP_ ## op,
#include "opcode-def.h"
#undef OPCODE_DEF
  };
#endif

  cur_frame = frame;

  MxcValue *gvmap = frame->gvars;
  uint8_t *pc = &frame->code[0];
  Literal **lit_table = (Literal **)ltable->data;
  int key;

  Dispatch();

  CASE(PUSH) {
    ++pc;
    key = READ_i32(pc); 
    MxcValue ob = lit_table[key]->raw;
    Push(ob);
    INCREF(ob);

    Dispatch();
  }
  CASE(IPUSH) {
    ++pc;
    Push(mval_int(READ_i32(pc)));

    Dispatch();
  }
  CASE(CPUSH) {
    ++pc;
    Push(new_char(READ_i8(pc)));

    Dispatch();
  }
  CASE(LPUSH) {
    ++pc;
    key = READ_i32(pc);
    Push(mval_int(lit_table[key]->lnum));

    Dispatch();
  }
  CASE(PUSHCONST_0) {
    ++pc;
    MxcValue ob = mval_int(0);
    Push(ob);

    Dispatch();
  }
  CASE(PUSHCONST_1) {
    ++pc;
    MxcValue ob = mval_int(1);
    Push(ob);

    Dispatch();
  }
  CASE(PUSHCONST_2) {
    ++pc;
    MxcValue ob = mval_int(2);
    Push(ob);

    Dispatch();
  }
  CASE(PUSHCONST_3) {
    ++pc;
    MxcValue ob = mval_int(3);
    Push(ob);

    Dispatch();
  }
  CASE(PUSHTRUE) {
    ++pc;
    Push(mval_true);

    Dispatch();
  }
  CASE(PUSHFALSE) {
    ++pc;
    Push(mval_false);

    Dispatch();
  }
  CASE(PUSHNULL) {
    ++pc;
    Push(mval_null);

    Dispatch();
  }
  CASE(FPUSH){
    ++pc;
    key = READ_i32(pc);
    Push(mval_float(lit_table[key]->fnumber));

    Dispatch();
  }
  CASE(POP) {
    ++pc;
    (void)Pop();

    Dispatch();
  }
  CASE(ADD) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(num_add(l, r));

    Dispatch();
  }
  CASE(FADD) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(FloatAdd(l, r));

    Dispatch();
  }
  CASE(STRCAT) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(str_concat(ostr(l), ostr(r)));

    Dispatch();
  }
  CASE(SUB) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(num_sub(l, r));

    Dispatch();
  }
  CASE(FSUB) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(FloatSub(l, r));

    Dispatch();
  }
  CASE(MUL) {
    ++pc; // mul
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(num_mul(l, r));

    Dispatch();
  }
  CASE(FMUL) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(FloatMul(l, r));

    Dispatch();
  }
  CASE(DIV) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    MxcValue res = num_div(l, r);
    if(Invalid_val(res)) {
      mxc_raise_err(frame, RTERR_ZERO_DIVISION);
      goto exit_failure;
    }
    SetTop(res);

    Dispatch();
  }
  CASE(FDIV) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    MxcValue res = float_div(l, r);
    if(Invalid_val(res)) {
      mxc_raise_err(frame, RTERR_ZERO_DIVISION);
      goto exit_failure;
    }
    SetTop(res);

    Dispatch();
  }
  CASE(MOD) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    MxcValue res = num_mod(l, r);
    if(Invalid_val(res)) {
      mxc_raise_err(frame, RTERR_ZERO_DIVISION);
      goto exit_failure;
    }
    SetTop(res);

    Dispatch();
  }
  CASE(LOGOR) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(bool_logor(l, r));

    Dispatch();
  }
  CASE(LOGAND) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(bool_logand(l, r));

    Dispatch();
  }
  CASE(BXOR) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(IntXor(l, r));

    Dispatch();
  }
  CASE(EQ) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(num_eq(l, r));

    Dispatch();
  }
  CASE(FEQ) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(float_eq(l, r));

    Dispatch();
  }
  CASE(NOTEQ) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(num_noteq(l, r));

    Dispatch();
  }
  CASE(FNOTEQ) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(float_neq(l, r));

    Dispatch();
  }
  CASE(LT) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(int_lt(l, r));

    Dispatch();
  }
  CASE(FLT) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(float_lt(l, r));

    Dispatch();
  }
  CASE(LTE) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(int_lte(l, r));

    Dispatch();
  }
  CASE(FLTE) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(float_lt(l, r));

    DECREF(r);
    DECREF(l);

    Dispatch();
  }
  CASE(GT) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(int_gt(l, r));

    Dispatch();
  }
  CASE(FGT) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(float_gt(l, r));

    Dispatch();
  }
  CASE(GTE) {
    ++pc;
    MxcValue r = Pop();
    MxcValue l = Top();
    SetTop(int_gte(l, r));

    Dispatch();
  }
  CASE(INEG) {
    ++pc;
    MxcValue u = Top();
    SetTop(num_neg(u));

    Dispatch();
  }
  CASE(FNEG) {
    ++pc;
    MxcValue u = Top();
    SetTop(mval_float(-(u.fnum)));

    Dispatch();
  }
  CASE(NOT) {
    ++pc;
    MxcValue b = Top();
    SetTop(bool_not(b));

    Dispatch();
  }
  CASE(STORE_GLOBAL) {
    ++pc;
    key = READ_i32(pc);

    gvmap[key] = Top();

    Dispatch();
  }
  CASE(STORE_LOCAL) {
    ++pc;
    key = READ_i32(pc);

    frame->lvars[key] = Top();

    Dispatch();
  }
  CASE(LOAD_GLOBAL) {
    ++pc;
    key = READ_i32(pc);
    MxcValue ob = gvmap[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
  }
  CASE(LOAD_LOCAL) {
    ++pc;
    key = READ_i32(pc);
    MxcValue ob = frame->lvars[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
  }
  CASE(JMP) {
    ++pc;
    frame->pc = READ_i32(pc);
    pc = &frame->code[frame->pc];

    Dispatch();
  }
  CASE(JMP_EQ) {
    ++pc;
    MxcValue a = Pop();
    if(a.num) {
      frame->pc = READ_i32(pc);
      pc = &frame->code[frame->pc];
    }
    else {
      pc += 4;
    }

    Dispatch();
  }
  CASE(JMP_NOTEQ) {
    ++pc;
    MxcValue a = Pop();
    if(!a.num) {
      frame->pc = READ_i32(pc);
      pc = &frame->code[frame->pc];
    }
    else {
      pc += 4;
    }

    Dispatch();
  }
  CASE(JMP_NOTERR) {
    ++pc;
    if(!error_flag) {
      frame->pc = READ_i32(pc);
      pc = &frame->code[frame->pc];
    }
    else {
      pc += 4;
    }
    error_flag--;

    Dispatch();
  }
  CASE(LISTSET) {
    ++pc;
    int narg = READ_i32(pc);
    MxcValue list = new_list(narg);
    ITERABLE(olist(list))->next = Top();
    while(--narg >= 0) {
      List_Setitem(list, narg, Pop());
    }
    Push(list);

    Dispatch();
  }
  CASE(LISTSET_SIZE) {
    ++pc;
    MxcValue n = Pop();
    MxcValue init = Pop();
    MxcValue ob = new_list_with_size(n, init);
    ITERABLE(olist(ob))->next = init;
    Push(ob);

    Dispatch();
  }
  CASE(LISTLENGTH) {
    ++pc;
    MxcValue ls = Pop();
    Push(mval_int(ITERABLE(olist(ls))->length));

    Dispatch();
  }
  CASE(SUBSCR) {
    ++pc;
    MxcIterable *ls = (MxcIterable *)olist(Pop());
    MxcValue idx = Top();
    MxcValue ob = SYSTEM(ls)->get(ls, idx.num);
    if(Invalid_val(ob)) {
      raise_outofrange(frame,
          idx,
          mval_int(ITERABLE(ls)->length));
      goto exit_failure;
    }
    SetTop(ob);

    Dispatch();
  }
  CASE(SUBSCR_STORE) {
    ++pc;
    MxcIterable *ls = (MxcIterable *)olist(Pop());
    MxcValue idx = Pop();
    MxcValue top = Top();
    MxcValue res = SYSTEM(ls)->set(ls, idx.num, top);
    if(Invalid_val(res)) {
      raise_outofrange(frame,
          idx,
          mval_int(ls->length));
      goto exit_failure;
    }

    Dispatch();
  }
  CASE(STRINGSET) {
    ++pc;
    key = READ_i32(pc);
    char *str = lit_table[key]->str;
    Push(new_string_static(str, strlen(str)));

    Dispatch();
  }
  CASE(TUPLESET) {
    ++pc;
    Dispatch();
  }
  CASE(FUNCTIONSET) {
    ++pc;
    key = READ_i32(pc);
    Push(new_function(lit_table[key]->func));

    Dispatch();
  }
  CASE(ITERFN_SET) {
    ++pc;
    key = READ_i32(pc);
    Push(new_mfiber(lit_table[key]->func, frame));
    Dispatch();
  }
  CASE(STRUCTSET) {
    ++pc;
    int nfield = READ_i32(pc);
    Push(new_struct(nfield));

    Dispatch();
  }
  CASE(CALL) {
    ++pc;
    int nargs = READ_i32(pc);
    MxcValue callee = Pop();
    int ret = ocallee(callee)->call(ocallee(callee), frame, nargs);
    if(ret) {
      goto exit_failure;
    }

    Dispatch();
  }
  CASE(MEMBER_LOAD) {
    ++pc;
    int offset = READ_i32(pc);
    MxcValue strct = Pop();
    MxcValue data = Member_Getitem(strct, offset);

    Push(data);

    Dispatch();
  }
  CASE(MEMBER_STORE) {
    ++pc;
    int offset = READ_i32(pc);
    MxcValue strct = Pop();
    MxcValue data = Top();

    Member_Setitem(strct, offset, data);

    Dispatch();
  }
  CASE(ITER) {
    ++pc;
    MxcObject *iterable = Pop().obj;

    Dispatch();
  }
  CASE(ITER_NEXT) {
    ++pc;
    MxcIterable *iter = (MxcIterable *)Top().obj;
    MxcValue res = iterable_next(iter); 
    if(Invalid_val(res)) {
      frame->pc = READ_i32(pc); 
      pc = &frame->code[frame->pc];
    }
    else {
      pc += 4;
    }
    Push(res);

    Dispatch();
  }
  CASE(BREAKPOINT) {
    ++pc;
    start_debug(frame);
    Dispatch();
  }
  CASE(ASSERT) {
    ++pc;
    MxcValue top = Pop();
    if(!top.num) {
      mxc_raise_err(frame, RTERR_ASSERT);
      goto exit_failure;
    }

    Dispatch();
  }
  CASE(RET) {
    ++pc;
    return 0;
  }
  CASE(YIELD) {
    ++pc;
    // TODO
    return 0;
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
      mxc_raise_err(frame, RTERR_UNIMPLEMENTED);
      goto exit_failure;
    }

exit_failure:
  runtime_error(frame);

  return 1;
}

void stack_dump() {
  MxcValue *base = cur_frame->stackbase;
  MxcValue *cur = cur_frame->stackptr;
  MxcValue ob;
  puts("---stack---");
  while(base < cur) {
    ob = *--cur;
    printf("%s\n", ostr(mval2str(ob))->str);
  }
  puts("-----------");
}
