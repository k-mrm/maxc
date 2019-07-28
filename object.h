#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "function.h"
#include "env.h"
#include "error.h"
#include "maxc.h"

#define OBJECT_POOL

struct MxcObject {
    int refcount;
};

struct IntObject : MxcObject {
    int inum32;
};

struct FloatObject : MxcObject {
    double fnum;
};

struct BoolObject : MxcObject {
    bool boolean;
};

struct CharObject : MxcObject {
    char ch;
};

struct ListObject : MxcObject {
    MxcObject **elem;
    size_t size;
};

struct StringObject : MxcObject {
    const char *str;
};

struct TupleObject : MxcObject {}; // TODO

struct FunctionObject : MxcObject {
    userfunction func;
};

struct BltinFuncObject : MxcObject {
    bltinfn_ty func;
};

struct NullObject : MxcObject {};

#ifdef OBJECT_POOL

union obalign {
    IntObject i;
    FloatObject fl;
    BoolObject b;
    CharObject c;
    ListObject l;
    StringObject s;
    TupleObject t;
    FunctionObject fn;
    BltinFuncObject bf;
};

class ObjectPool {
  public:
    std::vector<MxcObject *> pool;

    void realloc();

    ObjectPool() {
        pool.resize(100);

        for(int i = 0; i < 100; ++i) {
            pool[i] = (MxcObject *)malloc(sizeof(obalign));
        }
    }
};

#endif

namespace Object {

MxcObject *Mxc_malloc(size_t);

IntObject *alloc_intobject(int);
IntObject *int_add(IntObject *, IntObject *);
IntObject *int_sub(IntObject *, IntObject *);
IntObject *int_mul(IntObject *, IntObject *);
IntObject *int_div(IntObject *, IntObject *);
IntObject *int_mod(IntObject *, IntObject *);
BoolObject *int_eq(IntObject *, IntObject *);
BoolObject *int_noteq(IntObject *, IntObject *);
BoolObject *int_lt(IntObject *, IntObject *);
BoolObject *int_lte(IntObject *, IntObject *);
BoolObject *int_gt(IntObject *, IntObject *);
BoolObject *int_gte(IntObject *, IntObject *);
IntObject *int_inc(IntObject *);
IntObject *int_dec(IntObject *);

BoolObject *bool_logor(BoolObject *, BoolObject *);
BoolObject *bool_logand(BoolObject *, BoolObject *);

FloatObject *alloc_floatobject(double);

BoolObject *alloc_boolobject(int);
CharObject *alloc_charobject(char);
StringObject *alloc_stringobject(const char *);
FunctionObject *alloc_functionobject(userfunction);
BltinFuncObject *alloc_bltinfnobject(bltinfn_ty &);
ListObject *alloc_listobject(size_t);

BoolObject *bool_from_int(IntObject *);

}; // namespace Object

// test
#define IntAdd(l, r) (Object::alloc_intobject(l->inum32 + r->inum32))
#define IntSub(l, r) (Object::alloc_intobject(l->inum32 - r->inum32))
#define IntLte(l, r) (Object::alloc_boolobject(l->inum32 <= r->inum32))
#define IntLt(l, r) (Object::alloc_boolobject(l->inum32 < r->inum32))
#define IntGt(l, r) (Object::alloc_boolobject(l->inum32 > r->inum32))
#define FloatAdd(l, r) (Object::alloc_floatobject(l->fnum + r->fnum))
#define FloatSub(l, r) (Object::alloc_floatobject(l->fnum - r->fnum))
#define FloatMul(l, r) (Object::alloc_floatobject(l->fnum * r->fnum))
#define FloatDiv(l, r) (Object::alloc_floatobject(l->fnum / r->fnum))
#define FloatLt(l, r) (Object::alloc_boolobject(l->fnum < r->fnum))
#define FloatGt(l, r) (Object::alloc_boolobject(l->fnum > r->fnum))

#endif
