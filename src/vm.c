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

static int vm_exec(VM *);

#ifndef DPTEST
#define Dispatch()                                                             \
    do {                                                                       \
        goto *codetable[(uint8_t)frame->code[vm->frame->pc]];                      \
    } while(0)
#else
#define DISPATCH_CASE(name, smallname)                                         \
    case OP_##name:                                                    \
        goto code_##smallname;

#define Dispatch()                                                             \
    do {                                                                       \
        switch(vm->frame->code[vm->frame->pc]) {                                       \
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
            DISPATCH_CASE(STRUCTSET, structset)                                \
            DISPATCH_CASE(FUNCTIONSET, functionset)                            \
            DISPATCH_CASE(MEMBER_LOAD, member_load)                            \
            DISPATCH_CASE(MEMBER_STORE, member_store)                            \
        default:                                                               \
            printf("%d:", vm->frame->code[vm->frame->pc]);                             \
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

VM *New_VM(Bytecode *iseq, int ngvar) {
    VM *v = malloc(sizeof(VM));

    v->frame = New_Global_Frame(iseq);
    v->gvmap = New_Vector();
    vec_allocate(v->gvmap, ngvar);
    v->framestack = New_Vector();

    return v;
}

int VM_run(VM *vm) {
    stackptr = (MxcObject **)malloc(sizeof(MxcObject *) * 1000);

#ifdef MXC_DEBUG
    printf("\e[2mptr: %p\e[0m\n", stackptr);
#endif

    int ret = vm_exec(vm);

#ifdef MXC_DEBUG
    printf("\e[2mptr: %p\e[0m\n", stackptr);
#endif

    return ret;
}

static int vm_exec(VM *vm) {
#ifndef DPTEST
    static const void *codetable[] = {
        &&code_end,          &&code_push,        &&code_ipush,
        &&code_pushconst_0,  &&code_pushconst_1, &&code_pushconst_2,
        &&code_pushconst_3,  &&code_pushtrue,    &&code_pushfalse,
        &&code_pop,          &&code_add,         &&code_sub,
        &&code_mul,          &&code_div,         &&code_mod,
        &&code_logor,        &&code_logand,      &&code_eq,
        &&code_noteq,        &&code_lt,          &&code_lte,
        &&code_gt,           &&code_gte,         &&code_jmp,
        &&code_jmp_eq,       &&code_jmp_noteq,   &&code_inc,
        &&code_dec,          &&code_format,      &&code_typeof,
        &&code_load_global,  &&code_load_local,  &&code_store_global,
        &&code_store_local,  &&code_listset,     &&code_subscr,
        &&code_subscr_store, &&code_stringset,   &&code_tupleset,
        &&code_functionset,  &&code_bltinfnset,  &&code_ret,
        &&code_call,         &&code_call_bltin,  &&code_callmethod,
    };
#endif

    Dispatch();

code_ipush:
    ++vm->frame->pc;
    Push(alloc_intobject(READ_i32(vm->frame->code, vm->frame->pc)));
    vm->frame->pc += 4;

    Dispatch();
code_pushconst_0:
    ++vm->frame->pc;
    Push(alloc_intobject(0));

    Dispatch();
code_pushconst_1:
    ++vm->frame->pc;
    Push(alloc_intobject(1));

    Dispatch();
code_pushconst_2:
    ++vm->frame->pc;
    Push(alloc_intobject(2));

    Dispatch();
code_pushconst_3:
    ++vm->frame->pc;
    Push(alloc_intobject(3));

    Dispatch();
code_pushtrue:
    ++vm->frame->pc;
    Push(&MxcTrue);
    INCREF(&MxcTrue);

    Dispatch();
code_pushfalse:
    ++vm->frame->pc;
    Push(&MxcFalse);
    INCREF(&MxcFalse);

    Dispatch();
code_fpush : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    Push(alloc_floatobject(((Literal *)ltable->data[key])->fnumber));

    Dispatch();
}
code_pop:
    ++vm->frame->pc;
    Pop();

    Dispatch();
code_add : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntAdd(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fadd : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatAdd(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_sub : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntSub(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fsub : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatSub(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_mul : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntMul(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fmul : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(FloatMul(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_div : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(IntDiv(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fdiv : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop(); 
    Push(FloatDiv(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_mod : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_mod(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_logor : {
    ++vm->frame->pc;

    BoolObject *r = (BoolObject *)Pop();
    BoolObject *l = (BoolObject *)Pop();

    Push(bool_logor(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_logand : {
    ++vm->frame->pc;

    BoolObject *r = (BoolObject *)Pop();
    BoolObject *l = (BoolObject *)Pop();

    Push(bool_logand(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_eq : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_eq(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_noteq : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_noteq(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_lt : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_lt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_flt : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(float_lt(l, r));

    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_lte : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_lte(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_gt : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_gt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_fgt : {
    ++vm->frame->pc;

    FloatObject *r = (FloatObject *)Pop();
    FloatObject *l = (FloatObject *)Pop();

    Push(float_gt(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_gte : {
    ++vm->frame->pc;

    IntObject *r = (IntObject *)Pop();
    IntObject *l = (IntObject *)Pop();

    Push(int_gte(l, r));
    DECREF(r);
    DECREF(l);

    Dispatch();
}
code_inc : {
    ++vm->frame->pc;

    IntObject *u = (IntObject *)Pop();

    Push(int_inc(u));

    Dispatch();
}
code_dec : {
    ++vm->frame->pc;

    IntObject *u = (IntObject *)Pop();

    Push(int_dec(u));

    Dispatch();
}
code_store_global : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    MxcObject *old = vm->gvmap->data[key];
    if(old) {
        DECREF(old);
    }

    vm->gvmap->data[key] = Pop();

    Dispatch();
}
code_store_local : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    MxcObject *old = (MxcObject *)vm->frame->lvars->data[key];
    if(old) {
        DECREF(old);
    }

    vm->frame->lvars->data[key] = Pop();

    Dispatch();
}
code_load_global : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    MxcObject *ob = vm->gvmap->data[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_load_local : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    MxcObject *ob = (MxcObject *)vm->frame->lvars->data[key];
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_jmp:
    ++vm->frame->pc;

    vm->frame->pc = READ_i32(vm->frame->code, vm->frame->pc);

    Dispatch();
code_jmp_eq : {
    ++vm->frame->pc;

    BoolObject *a = (BoolObject *)Pop();

    if(a->boolean == true)
        vm->frame->pc = READ_i32(vm->frame->code, vm->frame->pc);
    else
        vm->frame->pc += 4;

    DECREF(a);

    Dispatch();
}
code_jmp_noteq : {
    ++vm->frame->pc;

    BoolObject *a = (BoolObject *)Pop();

    if(a->boolean == false)
        vm->frame->pc = READ_i32(vm->frame->code, vm->frame->pc);
    else
        vm->frame->pc += 4; // skip arg

    DECREF(a);

    Dispatch();
}
code_listset : {
    ++vm->frame->pc;
    /*
    auto ob = alloc_listobject(code[vm->frame->pc].size);

    for(cnt = 0; cnt < code[vm->frame->pc].size; ++cnt) {
        List_Setitem(ob, cnt, stk.top()); Pop();
    }

    Push(ob);
    */
    Dispatch();
}
code_subscr : {
    ++vm->frame->pc;
    ListObject *ls = (ListObject *)Pop();
    IntObject *idx = (IntObject *)Pop();
    MxcObject *ob = List_Getitem(ls, idx->inum);
    INCREF(ob);
    Push(ob);

    Dispatch();
}
code_subscr_store : {
    ++vm->frame->pc;
    ListObject *ob = (ListObject *)Pop();
    IntObject *idx = (IntObject *)Pop();
    List_Setitem(ob, idx->inum, Pop());

    Dispatch();
}
code_stringset : {
    ++vm->frame->pc;
    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    Push(alloc_stringobject(((Literal *)ltable->data[key])->str));

    Dispatch();
}
code_tupleset : {
    ++vm->frame->pc;
    /*
    vmcode_t &c = code[vm->frame->pc];
    TupleObject tupob;
    for(lfcnt = 0; lfcnt < c.size; ++lfcnt) {
        tupob.tup.push_back(s.top()); s.pop();
    }
    s.push(value_t(tupob));*/
    Dispatch();
}
code_functionset : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    Push(alloc_functionobject(((Literal *)ltable->data[key])->func));

    Dispatch();
}
code_bltinfnset : {
    ++vm->frame->pc;

    int key = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    Push(alloc_bltinfnobject(bltinfns[key]));

    Dispatch();
}
code_structset: {
    ++vm->frame->pc;

    int nfield = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    Push(alloc_structobject(nfield));

    Dispatch();
}
code_call : {
    ++vm->frame->pc;

    vec_push(vm->framestack, vm->frame);

    FunctionObject *callee = (FunctionObject *)Pop();

    vm->frame = New_Frame(callee->func);

    vm_exec(vm);

    vm->frame = vec_pop(vm->framestack);

    Dispatch();
}
code_call_bltin : {
    ++vm->frame->pc;

    int nargs = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    BltinFuncObject *callee = (BltinFuncObject *)Pop();

    MxcObject *ret = callee->func(nargs);

    Push(ret);

    Dispatch();
}
code_member_load: {
    ++vm->frame->pc;

    int offset = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    StructObject *ob = (StructObject *)Pop();

    MxcObject *data = Member_Getitem(ob, offset);

    Push(data);

    Dispatch();
}
code_member_store: {
    ++vm->frame->pc;

    int offset = READ_i32(vm->frame->code, vm->frame->pc);
    vm->frame->pc += 4;

    StructObject *ob = (StructObject *)Pop();
    MxcObject *data = Pop();

    Member_Setitem(ob, offset, data);

    Dispatch();
}
code_ret : {
    ++vm->frame->pc;

    for(int i = 0; i < vm->frame->nlvars; ++i) {
        DECREF(vm->frame->lvars->data[i]);
    }

    Delete_Frame(vm->frame);

    return 0;
}
code_end:
    return 0;
}
