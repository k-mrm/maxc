#include "vm.h"
#include "ast.h"
#include "bytecode.h"
#include "error/error.h"
#include "error/runtime-err.h"
#include "literalpool.h"
#include "builtins.h"
#include "debug.h"
#include "object/object.h"
#include "object/boolobject.h"
#include "object/charobject.h"
#include "object/floatobject.h"
#include "object/funcobject.h"
#include "object/intobject.h"
#include "object/listobject.h"
#include "object/nullobject.h"
#include "object/strobject.h"

// #define DPTEST

int error_flag = 0;

#ifndef DPTEST
#define Dispatch() goto *optable[*pc]
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

#define List_Setitem(ob, index, item) (ob->elem[index] = (item))
#define List_Getitem(ob, index) (ob->elem[index])

#define Member_Getitem(ob, offset) (ob->field[offset])
#define Member_Setitem(ob, offset, item) (ob->field[offset] = (item))

#define PEEK_i32(pc)  \
    ((uint8_t)(pc)[3]<<24|(uint8_t)(pc)[2]<<16|   \
     (uint8_t)(pc)[1]<<8 |(uint8_t)(pc)[0])

#define READ_i32(pc) (pc += 4, PEEK_i32(pc - 4))

#define PEEK_i8(pc) (*(pc))

#define READ_i8(pc) (PEEK_i8(pc++))

#define CASE(op) OP_ ## op:

int VM_run(Frame *frame) {
#ifdef MXC_DEBUG
    printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

    int ret = vm_exec(frame);

#ifdef MXC_DEBUG
    printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

    return ret;
}

int vm_exec(Frame *frame) {

#ifndef DPTEST
    static const void *optable[] = {
#define OPCODE_DEF(op) &&OP_ ## op,
#include "opcode-def.h"
#undef OPCODE_DEF
    };
#endif

    MxcObject **gvmap = frame->gvars;
    uint8_t *pc = &frame->code[0];
    Literal **lit_table = (Literal **)ltable->data;

    int key;

    Dispatch();

    CASE(PUSH) {
        ++pc;
        key = READ_i32(pc); 
        MxcObject *ob = lit_table[key]->raw;
        Push(lit_table[key]->raw);
        INCREF(ob);

        Dispatch();
    }
    CASE(IPUSH) {
        ++pc;
        Push(new_intobject(READ_i32(pc)));

        Dispatch();
    }
    CASE(CPUSH) {
        ++pc;
        Push(new_charobject(READ_i8(pc)));

        Dispatch();
    }
    CASE(LPUSH) {
        ++pc;
        key = READ_i32(pc);
        Push(new_intobject(lit_table[key]->lnum));

        Dispatch();
    }
    CASE(PUSHCONST_0) {
        ++pc;
        Push(new_intobject(0));

        Dispatch();
    }
    CASE(PUSHCONST_1) {
        ++pc;
        Push(new_intobject(1));

        Dispatch();
    }
    CASE(PUSHCONST_2) {
        ++pc;
        Push(new_intobject(2));

        Dispatch();
    }
    CASE(PUSHCONST_3) {
        ++pc;
        Push(new_intobject(3));

        Dispatch();
    }
    CASE(PUSHTRUE) {
        ++pc;
        Push(&MxcTrue);

        INCREF(&MxcTrue);

        Dispatch();
    }
    CASE(PUSHFALSE) {
        ++pc;
        Push(&MxcFalse);

        INCREF(&MxcFalse);

        Dispatch();
    }
    CASE(PUSHNULL) {
        ++pc;
        Push(&MxcNull);

        INCREF(&MxcNull);

        Dispatch();
    }
    CASE(FPUSH){
        ++pc;
        key = READ_i32(pc);
        Push(new_floatobject(lit_table[key]->fnumber));

        Dispatch();
    }
    CASE(POP) {
        ++pc;
        (void)Pop();

        Dispatch();
    }
    CASE(ADD) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(IntAdd(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FADD) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(FloatAdd(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(STRCAT) {
        ++pc;
        StringObject *r = (StringObject *)Pop();
        StringObject *l = (StringObject *)Top();
        SetTop(str_concat(l, r));

        Dispatch();
    }
    CASE(SUB) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(IntSub(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FSUB) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(FloatSub(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(MUL) {
        ++pc; // mul
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(IntMul(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FMUL) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(FloatMul(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(DIV) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        MxcObject *res = (MxcObject *)int_div(l, r);
        if(!res) {
            mxc_raise_err(frame, RTERR_ZERO_DIVISION);
            goto exit_failure;
        }
        SetTop(res);

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FDIV) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        MxcObject *res = (MxcObject *)float_div(l, r);
        if(!res) {
            mxc_raise_err(frame, RTERR_ZERO_DIVISION);
            goto exit_failure;
        }
        SetTop(res);

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(MOD) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_mod(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(LOGOR) {
        ++pc;
        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();
        SetTop(bool_logor(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(LOGAND) {
        ++pc;
        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();
        SetTop(bool_logand(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(BXOR) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(IntXor(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(EQ) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_eq(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FEQ) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(float_eq(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(NOTEQ) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_noteq(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FNOTEQ) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(float_neq(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(LT) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_lt(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FLT) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(float_lt(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(LTE) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_lte(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(GT) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_gt(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(FGT) {
        ++pc;
        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();
        SetTop(float_gt(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(GTE) {
        ++pc;
        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();
        SetTop(int_gte(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(INC) {
        ++pc;
        IntObject *u = (IntObject *)Top();
        ++u->inum;

        Dispatch();
    }
    CASE(DEC) {
        ++pc;
        IntObject *u = (IntObject *)Top();
        --u->inum;

        Dispatch();
    }
    CASE(INEG) {
        ++pc;
        IntObject *u = (IntObject *)Top();
        SetTop(new_intobject(-(u->inum)));

        DECREF(u);

        Dispatch();
    }
    CASE(FNEG) {
        ++pc;
        FloatObject *u = (FloatObject *)Top();
        SetTop(new_floatobject(-(u->fnum)));

        DECREF(u);

        Dispatch();
    }
    CASE(NOT) {
        ++pc;
        BoolObject *b = (BoolObject *)Top();
        SetTop(bool_not(b));

        DECREF(b);

        Dispatch();
    }
    CASE(STORE_GLOBAL) {
        ++pc;
        key = READ_i32(pc);
        MxcObject *old = gvmap[key];
        if(old) {
            DECREF(old);
        }

        gvmap[key] = Top();

        Dispatch();
    }
    CASE(STORE_LOCAL) {
        ++pc;
        key = READ_i32(pc);
        MxcObject *old = (MxcObject *)frame->lvars[key];
        if(old) {
            DECREF(old);
        }

        frame->lvars[key] = Top();

        Dispatch();
    }
    CASE(LOAD_GLOBAL) {
        ++pc;
        key = READ_i32(pc);
        MxcObject *ob = gvmap[key];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(LOAD_LOCAL) {
        ++pc;
        key = READ_i32(pc);
        MxcObject *ob = (MxcObject *)frame->lvars[key];
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
        IntObject *a = (IntObject *)Pop();
        if(a->inum) {
            frame->pc = READ_i32(pc);
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }

        DECREF(a);

        Dispatch();
    }
    CASE(JMP_NOTEQ) {
        ++pc;
        IntObject *a = (IntObject *)Pop();
        if(!a->inum) {
            frame->pc = READ_i32(pc);
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }

        DECREF(a);

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
        ListObject *list = new_listobject(narg);
        ((MxcIterable *)list)->next = Top();
        while(--narg >= 0) {
            List_Setitem(list, narg, Pop());
        }
        Push(list);

        Dispatch();
    }
    CASE(LISTSET_SIZE) {
        ++pc;
        IntObject *n = (IntObject *)Pop();
        MxcObject *init = Pop();
        ListObject *ob = new_listobject_size(n, init);
        ((MxcIterable *)ob)->next = init;
        Push(ob);

        Dispatch();
    }
    CASE(LISTLENGTH) {
        ++pc;
        ListObject *ls = (ListObject *)Pop();
        Push(new_intobject(ITERABLE(ls)->length));
        DECREF(ls);

        Dispatch();
    }
    CASE(SUBSCR) {
        ++pc;
        MxcIterable *ls = (MxcIterable *)Pop();
        IntObject *idx = (IntObject *)Top();
        MxcObject *ob = OBJIMPL(ls)->get(ls, idx->inum);
        if(!ob) {
            raise_outofrange(frame,
                    (MxcObject *)idx,
                    (MxcObject *)new_intobject(ls->length));
            goto exit_failure;
        }
        INCREF(ob);

        DECREF(ls);
        DECREF(idx);
        SetTop(ob);

        Dispatch();
    }
    CASE(SUBSCR_STORE) {
        ++pc;
        MxcIterable *ob = (MxcIterable *)Pop();
        IntObject *idx = (IntObject *)Pop();
        MxcObject *top = Top();
        if(!OBJIMPL(ob)->set(ob, idx->inum, top)) {
            raise_outofrange(frame,
                             (MxcObject *)idx,
                             (MxcObject *)new_intobject(ob->length));
            goto exit_failure;
        }

        DECREF(ob);
        DECREF(idx);

        Dispatch();
    }
    CASE(STRINGSET) {
        ++pc;
        key = READ_i32(pc);
        Push(new_stringobject(lit_table[key]->str, true));

        Dispatch();
    }
    CASE(TUPLESET) {
        ++pc;
        Dispatch();
    }
    CASE(FUNCTIONSET) {
        ++pc;
        key = READ_i32(pc);
        Push(new_functionobject(lit_table[key]->func));

        Dispatch();
    }
    CASE(BLTINFN_SET) {
        ++pc;
        Dispatch();
    }
    CASE(STRUCTSET) {
        ++pc;
        int nfield = READ_i32(pc);
        Push(new_structobject(nfield));

        Dispatch();
    }
    CASE(CALL) {
        ++pc;
        int nargs = READ_i32(pc);
        CallableObject *callee = (CallableObject *)Pop();
        int ret = callee->call(callee, frame, nargs);
        if(ret) {
            goto exit_failure;
        }
        DECREF(callee);

        Dispatch();
    }
    CASE(MEMBER_LOAD) {
        ++pc;
        int offset = READ_i32(pc);
        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Member_Getitem(ob, offset);
        INCREF(data);

        Push(data);

        Dispatch();
    }
    CASE(MEMBER_STORE) {
        ++pc;
        int offset = READ_i32(pc);
        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Top();

        Member_Setitem(ob, offset, data);

        Dispatch();
    }
    CASE(ITER_NEXT) {
        ++pc;
        MxcIterable *iter = (MxcIterable *)Top();
        MxcObject *res = iterable_next(iter); 
        if(!res) {
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
        BoolObject *top = (BoolObject *)Pop();
        if(!top->boolean) {
            mxc_raise_err(frame, RTERR_ASSERT);
            goto exit_failure;
        }

        Dispatch();
    }
    CASE(RET) {
        ++pc;
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
    CASE(FLTE)
    CASE(FGTE) {
        mxc_raise_err(frame, RTERR_UNIMPLEMENTED);
        goto exit_failure;
    }

exit_failure:
    runtime_error(frame);

    return 1;
}
