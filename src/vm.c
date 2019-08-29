#include "vm.h"
#include "ast.h"
#include "bytecode.h"
#include "error.h"
#include "literalpool.h"
#include "maxc.h"
#include "object.h"

#define DPTEST

MxcObject **stackptr;
extern bltinfn_ty bltinfns[];

static int vm_exec();

static MxcObject **gvmap;
static Vector *framestack;
static Frame *frame;

#ifndef DPTEST
#define Dispatch() goto *codetable[(frame->code[frame->pc])]
#else
#define DISPATCH_CASE(name, smallname)                                         \
    case OP_##name:                                                            \
        goto code_##smallname;

#define Dispatch()                                                             \
    do {                                                                       \
        switch(frame->code[frame->pc]) {                                       \
            DISPATCH_CASE(END, end)                                            \
            DISPATCH_CASE(IPUSH, ipush)                                        \
            DISPATCH_CASE(FPUSH, fpush)                                        \
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
            DISPATCH_CASE(LTE, lte)                                            \
            DISPATCH_CASE(LT, lt)                                              \
            DISPATCH_CASE(GT, gt)                                              \
            DISPATCH_CASE(EQ, eq)                                              \
            DISPATCH_CASE(NOTEQ, noteq)                                        \
            DISPATCH_CASE(JMP_NOTEQ, jmp_noteq)                                \
            DISPATCH_CASE(JMP, jmp)                                            \
            DISPATCH_CASE(SUB, sub)                                            \
            DISPATCH_CASE(ADD, add)                                            \
            DISPATCH_CASE(MUL, mul)                                            \
            DISPATCH_CASE(DIV, div)                                            \
            DISPATCH_CASE(MOD, mod)                                            \
            DISPATCH_CASE(FADD, fadd)                                          \
            DISPATCH_CASE(FSUB, fsub)                                          \
            DISPATCH_CASE(FMUL, fmul)                                          \
            DISPATCH_CASE(FDIV, fdiv)                                          \
            DISPATCH_CASE(FLT, flt)                                            \
            DISPATCH_CASE(FGT, fgt)                                            \
            DISPATCH_CASE(INC, inc)                                            \
            DISPATCH_CASE(DEC, dec)                                            \
            DISPATCH_CASE(BLTINFN_SET, bltinfnset)                             \
            DISPATCH_CASE(CALL_BLTIN, call_bltin)                              \
            DISPATCH_CASE(POP, pop)                                            \
            DISPATCH_CASE(STRINGSET, stringset)                                \
            DISPATCH_CASE(SUBSCR, subscr)                                      \
            DISPATCH_CASE(STRUCTSET, structset)                                \
            DISPATCH_CASE(LISTSET, listset)                                    \
            DISPATCH_CASE(FUNCTIONSET, functionset)                            \
            DISPATCH_CASE(MEMBER_LOAD, member_load)                            \
            DISPATCH_CASE(MEMBER_STORE, member_store)                          \
        default:                                                               \
            printf("err:%d\n", frame->code[frame->pc]);                        \
            runtime_err("!!internal error!!");                                 \
        }                                                                      \
    } while(0)
#endif

#define List_Setitem(ob, index, item) (ob->elem[index] = item)
#define List_Getitem(ob, index) (ob->elem[index])

#define Member_Getitem(ob, offset) (ob->field[offset])
#define Member_Setitem(ob, offset, item) (ob->field[offset] = (item))

#define READ_i32(code, pc)                                                     \
    ((int64_t)(((uint8_t)code[pc + 3] << 24) + ((uint8_t)code[pc + 2] << 16) + \
               ((uint8_t)code[pc + 1] << 8) + ((uint8_t)code[pc + 0])))

int VM_run(Bytecode *iseq, int ngvar) {
    stackptr = (MxcObject **)malloc(sizeof(MxcObject *) * 1000);

    frame = New_Global_Frame(iseq);

    gvmap = malloc(sizeof(MxcObject *) * ngvar);
    for(int i = 0; i < ngvar; i++) {
        gvmap[i] = NULL;
    }

    framestack = New_Vector();

#ifdef MXC_DEBUG
    printf("\e[2mptr: %p\e[0m\n", stackptr);
#endif

    int ret = vm_exec();

#ifdef MXC_DEBUG
    printf("\e[2mptr: %p\e[0m\n", stackptr);
#endif

    return ret;
}

static int vm_exec() {
#ifndef DPTEST
    static const void *codetable[] = {
        &&code_end,          &&code_push,         &&code_ipush,
        &&code_pushconst_0,  &&code_pushconst_1,  &&code_pushconst_2,
        &&code_pushconst_3,  &&code_pushtrue,     &&code_pushfalse,
        &&code_fpush,        &&code_pop,          &&code_add,
        &&code_sub,          &&code_mul,          &&code_div,
        &&code_mod,          &&code_logor,        &&code_logand,
        &&code_eq,           &&code_noteq,        &&code_lt,
        &&code_lte,          &&code_gt,           &&code_gte,
        &&code_fadd,         &&code_fsub,         &&code_fmul,
        &&code_fdiv,         &&code_fmod,         &&code_flogor,
        &&code_flogand,      &&code_feq,          &&code_fnoteq,
        &&code_flt,          &&code_flte,         &&code_fgt,
        &&code_fgte,         &&code_jmp,          &&code_jmp_eq,
        &&code_jmp_noteq,    &&code_inc,          &&code_dec,
        &&code_load_global,  &&code_load_local,   &&code_store_global,
        &&code_store_local,  &&code_listset,      &&code_subscr,
        &&code_subscr_store, &&code_stringset,    &&code_tupleset,
        &&code_functionset,  &&code_bltinfnset,   &&code_structset,
        &&code_ret,          &&code_call,         &&code_call_bltin,
        &&code_member_load,  &&code_member_store,
    };
#endif

    Dispatch();

code_ipush:
    ++frame->pc;
    Push(alloc_intobject(READ_i32(frame->code, frame->pc)));
    frame->pc += 4;

    Dispatch();
code_pushconst_0:
    ++frame->pc;
    Push(alloc_intobject(0));

    Dispatch();
code_pushconst_1:
    ++frame->pc;
    Push(alloc_intobject(1));

    Dispatch();
code_pushconst_2:
    ++frame->pc;
    Push(alloc_intobject(2));

    Dispatch();
code_pushconst_3:
    ++frame->pc;
    Push(alloc_intobject(3));

    Dispatch();
code_pushtrue:
    ++frame->pc;
    Push(&MxcTrue);
    INCREF(&MxcTrue);

    Dispatch();
code_pushfalse:
    ++frame->pc;
    Push(&MxcFalse);
    INCREF(&MxcFalse);

    Dispatch();
code_fpush : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    Push(alloc_floatobject(((Literal *)ltable->data[key])->fnumber));

    Dispatch();
}
code_pop:
    ++frame->pc;
    Pop();

    Dispatch();
code_add : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntAdd(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fadd : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatAdd(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_sub : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntSub(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fsub : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatSub(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_mul : {
    ++frame->pc; // mul

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntMul(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fmul : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatMul(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_div : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntDiv(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fdiv : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();
    Push(FloatDiv(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_mod : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_mod(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_logor : {
    ++frame->pc;

    BoolObject *r = (BoolObject *)Pop();
    BoolObject *l = (BoolObject *)Pop();

    Push(bool_logor(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_logand : {
    ++frame->pc;

    BoolObject *r = (BoolObject *)Pop();
    BoolObject *l = (BoolObject *)Pop();

    Push(bool_logand(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_eq : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_eq(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_noteq : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_noteq(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_lt : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_lt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_flt : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(float_lt(l, r));

    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_lte : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_lte(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_gt : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_gt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fgt : {
    ++frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(float_gt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_gte : {
    ++frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_gte(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_inc : {
    ++frame->pc;

    IntObject *u = (IntObject *)Pop();

    Push(int_inc(u));

    Dispatch();
}
code_dec : {
    ++frame->pc;

    IntObject *u = (IntObject *)Pop();

    Push(int_dec(u));

    Dispatch();
}
code_store_global : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    MxcObject *old = gvmap[key];
    if(old) {
        DECREF(old);
    }

    gvmap[key] = Pop();

    Dispatch();
}
code_store_local : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    MxcObject *old = (MxcObject *)frame->lvars[key];
    if(old) {
        DECREF(old);
    }

    frame->lvars[key] = Pop();

    Dispatch();
}
code_load_global : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    MxcObject *ob = gvmap[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_load_local : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    MxcObject *ob = (MxcObject *)frame->lvars[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_jmp:
    ++frame->pc;

    frame->pc = READ_i32(frame->code, frame->pc);

    Dispatch();
code_jmp_eq : {
    ++frame->pc;

    BoolObject *a = (BoolObject *)Pop();

    if(a->boolean == true)
        frame->pc = READ_i32(frame->code, frame->pc);
    else
        frame->pc += 4;

    DECREF(a);

    Dispatch();
}
code_jmp_noteq : {
    ++frame->pc;

    BoolObject *a = (BoolObject *)Pop();

    if(a->boolean == false)
        frame->pc = READ_i32(frame->code, frame->pc);
    else
        frame->pc += 4; // skip arg

    DECREF(a);

    Dispatch();
}
code_listset : {
    ++frame->pc;

    int n = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    ListObject *ob = alloc_listobject(n);

    for(int i = 0; i < n; ++i) {
        List_Setitem(ob, i, Pop());
    }

    Push(ob);

    Dispatch();
}
code_subscr : {
    ++frame->pc;

    ListObject *ls = (ListObject *)Pop();
    IntObject *idx = (IntObject *)Pop();
    MxcObject *ob = List_Getitem(ls, idx->inum);
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_subscr_store : {
    ++frame->pc;
    ListObject *ob = (ListObject *)Pop();
    IntObject *idx = (IntObject *)Pop();
    List_Setitem(ob, idx->inum, Pop());

    Dispatch();
}
code_stringset : {
    ++frame->pc;
    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    Push(alloc_stringobject(((Literal *)ltable->data[key])->str));

    Dispatch();
}
code_tupleset : {
    ++frame->pc;

    /*
    vmcode_t &c = code[frame->pc];
    TupleObject tupob;
    for(lfcnt = 0; lfcnt < c.size; ++lfcnt) {
        tupob.tup.push_back(s.top()); s.pop();
    }
    s.push(value_t(tupob));*/
    Dispatch();
}
code_functionset : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    Push(alloc_functionobject(((Literal *)ltable->data[key])->func));

    Dispatch();
}
code_bltinfnset : {
    ++frame->pc;

    int key = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    Push(alloc_bltinfnobject(bltinfns[key]));

    Dispatch();
}
code_structset : {
    ++frame->pc;

    int nfield = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    Push(alloc_structobject(nfield));

    Dispatch();
}
code_call : {
    ++frame->pc;

    vec_push(framestack, frame);

    FunctionObject *callee = (FunctionObject *)Pop();

    frame = New_Frame(callee->func);

    vm_exec();

    frame = vec_pop(framestack);

    Dispatch();
}
code_call_bltin : {
    ++frame->pc;

    int nargs = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    BltinFuncObject *callee = (BltinFuncObject *)Pop();

    MxcObject *ret = callee->func(nargs);

    Push(ret);

    Dispatch();
}
code_member_load : {
    ++frame->pc; // member_load

    int offset = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    StructObject *ob = (StructObject *)Pop();

    MxcObject *data = Member_Getitem(ob, offset);

    INCREF(data);

    Push(data);

    Dispatch();
}
code_member_store : {
    ++frame->pc;

    int offset = READ_i32(frame->code, frame->pc);
    frame->pc += 4;

    StructObject *ob = (StructObject *)Pop();
    MxcObject *data = Pop();

    Member_Setitem(ob, offset, data);

    Dispatch();
}
code_ret : {
    ++frame->pc;

    for(int i = 0; i < frame->nlvars; ++i) {
        DECREF(frame->lvars[i]);
    }

    Delete_Frame(frame);

    return 0;
}
code_end:
    return 0;
// TODO
code_push:
code_feq:
code_flogor:
code_flogand:
code_fmod:
code_flte:
code_fnoteq:
code_fgte:
    runtime_err("unimplemented");
}
