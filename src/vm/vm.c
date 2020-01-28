#include "vm.h"
#include "ast.h"
#include "bytecode.h"
#include "error/error.h"
#include "error/runtime-err.h"
#include "literalpool.h"
#include "maxc.h"
#include "object/object.h"
#include "builtins.h"
#include "debug.h"

// #define DPTEST

int error_flag = 0;
extern bltinfn_ty bltinfns[];

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

#define FETCH_i32(pc) (pc += 4, PEEK_i32(pc - 4))

#define PEEK_i8(pc) (*(pc))

#define READ_i8(pc) (PEEK_i8(pc++))

#define CASE(op) op:

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

#define Push(ob) (*frame->stackptr++ = (MxcObject *)(ob))
#define Pop() (*--frame->stackptr)
#define Top() (frame->stackptr[-1])
#define SetTop(ob) (frame->stackptr[-1] = ((MxcObject *)(ob)))

#ifndef DPTEST
    static const void *optable[] = {
        &&code_end,          &&code_ipush, &&code_lpush, &&code_cpush,
        &&code_pushconst_0,  &&code_pushconst_1,  &&code_pushconst_2,
        &&code_pushconst_3,  &&code_pushtrue,     &&code_pushfalse,
        &&code_pushnull,     &&code_fpush,        &&code_pop, &&code_add,
        &&code_sub,          &&code_mul,          &&code_div,
        &&code_mod,          &&code_logor,        &&code_logand,
        &&code_eq,           &&code_noteq,        &&code_lt,
        &&code_lte,          &&code_gt,           &&code_gte,
        &&code_fadd,         &&code_fsub,         &&code_fmul,
        &&code_fdiv,         &&code_fmod,         &&code_flogor,
        &&code_flogand,      &&code_feq,          &&code_fnoteq,
        &&code_flt,          &&code_flte,         &&code_fgt,
        &&code_fgte,         &&code_jmp,          &&code_jmp_eq,
        &&code_jmp_noteq,    &&code_jmp_noterr,   &&code_inc, &&code_dec,
        &&code_not,          &&code_ineg,         &&code_fneg,
        &&code_load_global,  &&code_load_local,   &&code_store_global,
        &&code_store_local,  &&code_listset,      &&code_listset_size,
        &&code_listlength,   &&code_subscr,       &&code_subscr_store,
        &&code_stringset,    &&code_tupleset,     &&code_functionset,
        &&code_bltinfnset,   &&code_structset,    &&code_ret,
        &&code_call,         &&code_call_bltin,   &&code_member_load,
        &&code_member_store, &&code_iter_next,    &&code_strcat,
        &&code_breakpoint,
    };
#endif

    MxcObject **gvmap = frame->gvars;
    uint8_t *pc = &frame->code[0];

    Frame *new_frame;
    int key;

    Dispatch();

    CASE(code_ipush) {
        ++pc;
        Push(new_intobject(FETCH_i32(pc)));

        Dispatch();
    }
    CASE(code_cpush) {
        ++pc;
        Push(new_charobject(READ_i8(pc)));
        ++pc;

        Dispatch();
    }
    CASE(code_lpush) {
        ++pc;
        key = FETCH_i32(pc);

        Push(new_intobject(((Literal *)ltable->data[key])->lnum));

        Dispatch();
    }
    CASE(code_pushconst_0) {
        ++pc;
        Push(new_intobject(0));

        Dispatch();
    }
    CASE(code_pushconst_1) {
        ++pc;
        Push(new_intobject(1));

        Dispatch();
    }
    CASE(code_pushconst_2) {
        ++pc;
        Push(new_intobject(2));

        Dispatch();
    }
    CASE(code_pushconst_3) {
        ++pc;
        Push(new_intobject(3));

        Dispatch();
    }
    CASE(code_pushtrue) {
        ++pc;
        Push(&MxcTrue);
        INCREF(&MxcTrue);

        Dispatch();
    }
    CASE(code_pushfalse) {
        ++pc;
        Push(&MxcFalse);
        INCREF(&MxcFalse);

        Dispatch();
    }
    CASE(code_pushnull) {
        ++pc;
        Push(&MxcNull);
        INCREF(&MxcNull);

        Dispatch();
    }
    CASE(code_fpush){
        ++pc;

        key = FETCH_i32(pc);

        Push(new_floatobject(((Literal *)ltable->data[key])->fnumber));

        Dispatch();
    }
    CASE(code_pop) {
        ++pc;
        (void)Pop();

        Dispatch();
    }
    CASE(code_add) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntAdd(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_fadd) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatAdd(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_strcat) {
        ++pc;

        StringObject *r = (StringObject *)Pop();
        StringObject *l = (StringObject *)Top();

        SetTop(str_concat(l, r));

        Dispatch();
    }
    CASE(code_sub) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntSub(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_fsub) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatSub(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_mul) {
        ++pc; // mul

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntMul(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_fmul) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatMul(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_div) {
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
    CASE(code_fdiv) {
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
    CASE(code_mod) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_mod(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_logor) {
        ++pc;

        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();

        SetTop(bool_logor(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_logand) {
        ++pc;

        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();

        SetTop(bool_logand(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_eq) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_eq(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_feq) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_eq(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_noteq) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_noteq(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_fnoteq) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_neq(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_lt) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_lt(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_flt) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_lt(l, r));

        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_lte) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_lte(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_gt) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_gt(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_fgt) {
        ++pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_gt(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_gte) {
        ++pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_gte(l, r));
        DECREF(r);
        DECREF(l);

        Dispatch();
    }
    CASE(code_inc) {
        ++pc;

        IntObject *u = (IntObject *)Top();
        ++u->inum;

        Dispatch();
    }
    CASE(code_dec) {
        ++pc;

        IntObject *u = (IntObject *)Top();
        --u->inum;

        Dispatch();
    }
    CASE(code_ineg) {
        ++pc;

        IntObject *u = (IntObject *)Top();
        SetTop(new_intobject(-(u->inum)));
        DECREF(u);

        Dispatch();
    }
    CASE(code_fneg) {
        ++pc;

        FloatObject *u = (FloatObject *)Top();
        SetTop(new_floatobject(-(u->fnum)));
        DECREF(u);

        Dispatch();
    }
    CASE(code_not) {
        ++pc;

        BoolObject *b = (BoolObject *)Top();
        SetTop(bool_not(b));
        DECREF(b);

        Dispatch();
    }
    CASE(code_store_global) {
        ++pc;
        key = FETCH_i32(pc);

        MxcObject *old = gvmap[key];
        if(old) {
            DECREF(old);
        }

        gvmap[key] = Top();

        Dispatch();
    }
    CASE(code_store_local) {
        ++pc;
        key = FETCH_i32(pc);

        MxcObject *old = (MxcObject *)frame->lvars[key];
        if(old) {
            DECREF(old);
        }

        frame->lvars[key] = Top();

        Dispatch();
    }
    CASE(code_load_global) {
        ++pc;
        key = FETCH_i32(pc);

        MxcObject *ob = gvmap[key];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_load_local) {
        ++pc;
        key = FETCH_i32(pc);

        MxcObject *ob = (MxcObject *)frame->lvars[key];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_jmp) {
        ++pc;

        frame->pc = FETCH_i32(pc);
        pc = &frame->code[frame->pc];

        Dispatch();
    }
    CASE(code_jmp_eq) {
        ++pc;

        IntObject *a = (IntObject *)Pop();
        if(a->inum == 1) {
            frame->pc = FETCH_i32(pc);
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }

        DECREF(a);

        Dispatch();
    }
    CASE(code_jmp_noteq) {
        ++pc;

        IntObject *a = (IntObject *)Pop();
        if(a->inum == 0) {
            frame->pc = FETCH_i32(pc);
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }

        DECREF(a);

        Dispatch();
    }
    CASE(code_jmp_noterr) {
        ++pc;

        if(!error_flag) {
            frame->pc = FETCH_i32(pc);
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }

        error_flag--;

        Dispatch();
    }
    CASE(code_listset) {
        ++pc;

        int n = FETCH_i32(pc);

        ListObject *ob = new_listobject(n);

        ((MxcIterable *)ob)->next = Top();

        for(int i = 0; i < n; ++i) {
            List_Setitem(ob, i, Pop());
        }
        Push(ob);

        Dispatch();
    }
    CASE(code_listset_size) {
        ++pc;

        IntObject *n = (IntObject *)Pop();
        MxcObject *init = Pop();

        ListObject *ob = new_listobject_size(n, init);

        ((MxcIterable *)ob)->next = init;

        Push(ob);

        Dispatch();
    }
    CASE(code_listlength) {
        ++pc;

        ListObject *ls = (ListObject *)Pop();
        Push(new_intobject(ITERABLE(ls)->length));

        DECREF(ls);

        Dispatch();
    }
    CASE(code_subscr) {
        ++pc;

        MxcIterable *ls = (MxcIterable *)Pop();
        IntObject *idx = (IntObject *)Pop();
        MxcObject *ob = OBJIMPL(ls)->get(ls, idx->inum);
        if(!ob) {
            raise_outofrange(frame,
                             (MxcObject *)idx,
                             (MxcObject *)new_intobject(ls->length));
            goto exit_failure;
        }
        DECREF(ls);
        DECREF(idx);
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_subscr_store) {
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
        INCREF(top);

        Dispatch();
    }
    CASE(code_stringset) {
        ++pc;
        key = FETCH_i32(pc);

        Push(new_stringobject(((Literal *)ltable->data[key])->str, true));

        Dispatch();
    }
    CASE(code_tupleset) {
        ++pc;

        Dispatch();
    }
    CASE(code_functionset) {
        ++pc;
        key = FETCH_i32(pc);

        Push(new_functionobject(((Literal *)ltable->data[key])->func));

        Dispatch();
    }
    CASE(code_bltinfnset) {
        ++pc;
        key = FETCH_i32(pc);

        Push(new_bltinfnobject(bltinfns[key]));

        Dispatch();
    }
    CASE(code_structset) {
        ++pc;

        int nfield = FETCH_i32(pc);

        Push(new_structobject(nfield));

        Dispatch();
    }
    CASE(code_call) {
        ++pc;

        FunctionObject *callee = (FunctionObject *)Pop();

        new_frame = New_Frame(callee->func, frame);

        int res = vm_exec(new_frame);

        frame = new_frame->prev;
        frame->stackptr = new_frame->stackptr;
        Delete_Frame(new_frame);

        if(res) {
            return 1;
        }

        Dispatch();
    }
    CASE(code_call_bltin) {
        ++pc;
        int nargs = FETCH_i32(pc);

        BltinFuncObject *callee = (BltinFuncObject *)Pop();

        frame->stackptr -= nargs;
        MxcObject *ret = callee->func(frame->stackptr, nargs);
        DECREF(callee);

        Push(ret);

        Dispatch();
    }
    CASE(code_member_load) {
        ++pc;
        int offset = FETCH_i32(pc);

        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Member_Getitem(ob, offset);
        INCREF(data);

        Push(data);

        Dispatch();
    }
    CASE(code_member_store) {
        ++pc;
        int offset = FETCH_i32(pc);

        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Top();

        Member_Setitem(ob, offset, data);

        Dispatch();
    }
    CASE(code_iter_next) {
        ++pc;

        MxcIterable *iter = (MxcIterable *)Top();
        MxcObject *res = iterable_next(iter); 
        if(!res) {
            frame->pc = FETCH_i32(pc); 
            pc = &frame->code[frame->pc];
        }
        else {
            pc += 4;
        }
        Push(res);

        Dispatch();
    }
    CASE(code_breakpoint) {
        ++pc;

        start_debug(frame);

        Dispatch();
    }
    CASE(code_ret) {
        ++pc;

        for(size_t i = 0; i < frame->nlvars; ++i) {
            if(frame->lvars[i])
                DECREF(frame->lvars[i]);
        }

        return 0;
    }
    CASE(code_end) {
        /* exit_success */
        return 0;
    }
    // TODO
    CASE(code_flogor)
    CASE(code_flogand)
    CASE(code_fmod)
    CASE(code_flte)
    CASE(code_fgte) {
        mxc_raise_err(frame, RTERR_UNIMPLEMENTED);
        goto exit_failure;
    }

exit_failure:
    runtime_error(frame);

    return 1;
}
