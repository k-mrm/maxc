#include <stdlib.h>
#include "bytecode.h"
#include "literalpool.h"
#include "maxc.h"
#include "context.h"
#include "vm.h"
#include "error/error.h"

#ifdef DIRECT_THREADED
static const void **op_addr_table;

void op_addr_table_init() {
  op_addr_table = (const void **)vm_exec(NULL);
}
#else
void op_addr_table_init() {
  /* do nothing */
}
#endif

Bytecode *new_bytecode() {
  Bytecode *self = malloc(sizeof(Bytecode));
  self->code = malloc(sizeof(mptr_t) * 64);
  self->len = 0;
  self->reserved = 64;

  return self;
}

static void push(Bytecode *self, mptr_t a) {
  if(self->len == self->reserved) {
    self->reserved *= 2;
    self->code = realloc(self->code, sizeof(mptr_t) * self->reserved);
  }

  self->code[self->len++] = a;
}

#ifdef DIRECT_THREADED
void pushop(Bytecode *self, enum OPCODE op) {
  push(self, (mptr_t)op_addr_table[op]);
}
#else
void pushop(Bytecode *self, enum OPCODE op) {
  push(self, (mptr_t)op);
}
#endif

void pusharg(Bytecode *self, mptr_t a) {
  push(self, a);
}

void replace_int(size_t cpos, Bytecode *dst, int64_t src) {
  dst->code[cpos + 1] = (smptr_t)src;
}

static int32_t read_int32(mptr_t self[], size_t *pc) { // for Bytecode shower
  int32_t a = (smptr_t)self[(*pc)];

  *pc += 1;

  return a;
}

void codedump(mptr_t a[], size_t *i, Vector *lt) {
  /*
  printf("%04ld ", *i);
  int c;

  switch(c = a[(*i)++]) {
    case OP_PUSH: {
      int key = read_int32(a, i);
      printf("push %d", key);
      break;
    }
    case OP_IPUSH: {
      int i32 = read_int32(a, i);
      printf("ipush %d", i32);
      break;
    }
    case OP_LPUSH: {
      int id = read_int32(a, i);
      printf("lpush %ld", ((Literal *)lt->data[id])->lnum);
      break;
    }
    case OP_PUSHCONST_0: printf("pushconst0"); break;
    case OP_PUSHCONST_1: printf("pushconst1"); break;
    case OP_PUSHCONST_2: printf("pushconst2"); break;
    case OP_PUSHCONST_3: printf("pushconst3"); break;
    case OP_PUSHTRUE:    printf("pushtrue"); break;
    case OP_PUSHFALSE:   printf("pushfalse"); break;
    case OP_PUSHNULL:    printf("pushnull"); break;
    case OP_FPUSH: {
      int id = read_int32(a, i);
      printf("fpush %lf", ((Literal *)lt->data[id])->fnumber);
      break;
    }
    case OP_POP:        printf("pop"); break;
    case OP_ADD:        printf("add"); break;
    case OP_SUB:        printf("sub"); break;
    case OP_MUL:        printf("mul"); break;
    case OP_DIV:        printf("div"); break;
    case OP_MOD:        printf("mod"); break;
    case OP_LOGOR:      printf("or"); break;
    case OP_LOGAND:     printf("and"); break;
    case OP_EQ:         printf("eq"); break;
    case OP_NOTEQ:      printf("noteq"); break;
    case OP_LT:         printf("lt"); break;
    case OP_LTE:        printf("lte"); break;
    case OP_GT:         printf("gt"); break;
    case OP_GTE:        printf("gte"); break;
    case OP_FADD:       printf("fadd"); break;
    case OP_FSUB:       printf("fsub"); break;
    case OP_FMUL:       printf("fmul"); break;
    case OP_FDIV:       printf("fdiv"); break;
    case OP_FMOD:       printf("fmod"); break;
    case OP_FLOGOR:     printf("for"); break;
    case OP_FLOGAND:    printf("fand"); break;
    case OP_FEQ:        printf("feq"); break;
    case OP_FNOTEQ:     printf("fnoteq"); break;
    case OP_FLT:        printf("flt"); break;
    case OP_FLTE:       printf("flte"); break;
    case OP_FGT:        printf("fgt"); break;
    case OP_FGTE:       printf("fgte"); break;
    case OP_INEG:       printf("ineg"); break;
    case OP_FNEG:       printf("fneg"); break;
    case OP_NOT:        printf("not"); break;
    case OP_JMP: {
      int i32 = read_int32(a, i);
      printf("jmp %d", i32);
      break;
    }
    case OP_JMP_EQ:
                        printf("jmpeq");
                        break;
    case OP_JMP_NOTEQ: {
      int i32 = read_int32(a, i);
      printf("jmpneq %d", i32);
      break;
    }
    case OP_STORE_LOCAL: {
      int id = read_int32(a, i);

      printf("store_local %d", id);

      break;
    }
    case OP_STORE_GLOBAL: {
      int id = read_int32(a, i);

      printf("store_global %d", id);

      break;
    }
    case OP_LISTSET: {
      int n = read_int32(a, i);

      printf("listset %d", n);

      break;
    }
    case OP_LISTSET_SIZE: printf("listset-size"); break;
    case OP_LISTLENGTH: printf("listlength"); break;
    case OP_SUBSCR: printf("subscr"); break;
    case OP_SUBSCR_STORE: printf("subscr_store"); break;
    case OP_STRINGSET: {
      int k = read_int32(a, i);
      printf("stringset %s", ((Literal *)lt->data[k])->str);
      break;
    }
    case OP_TABLESET: {
      int k = read_int32(a, i);
      printf("tableset %d", k);
      break;
    }
    case OP_TUPLESET: printf("tupleset"); break;
    case OP_FUNCTIONSET: {
      int k = read_int32(a, i);
      userfunction *f = ((Literal *)lt->data[k])->func;

      printf("funcset ->\n");

      printf("length: %d\n", f->codesize);

      for(size_t n = 0; n < f->codesize;) {
        printf("  ");
        codedump(f->code, &n, lt);
        puts("");
      }

      break;
    }
    case OP_ITERFN_SET: {
      int k = read_int32(a, i);
      userfunction *f = ((Literal *)lt->data[k])->func;

      printf("iterator set ->\n");

      printf("length: %d\n", f->codesize);

      for(size_t n = 0; n < f->codesize; ) {
        printf("  ");
        codedump(f->code, &n, lt);
        puts("");
      }

      break;
    }
    case OP_STRUCTSET: {
      int n = read_int32(a, i);

      printf("structset %d", n);

      break;
    }
    case OP_LOAD_GLOBAL: {
      int id = read_int32(a, i);

      printf("load_global %d", id);

      break;
    }
    case OP_LOAD_LOCAL: {
      int id = read_int32(a, i);

      printf("load_local %d", id);

      break;
    }
    case OP_RET:    printf("ret"); break;
    case OP_YIELD:  printf("yield"); break;
    case OP_CALL: {
      int n = read_int32(a, i);
      printf("call arg:%d", n);
      break;
    }
    case OP_END: printf("end"); break;
    case OP_MEMBER_LOAD: {
      int n = read_int32(a, i);

      printf("member-load %d", n);

      break;
    }
    case OP_MEMBER_STORE: {
      int n = read_int32(a, i);

      printf("member-store %d", n);

      break;
    }
    case OP_ITER: printf("get-iter"); break;
    case OP_ITER_NEXT: {
      int n = read_int32(a, i);

      printf("iter_next %d", n);

      break;
    }
    case OP_STRCAT: printf("strcat"); break;
    case OP_BREAKPOINT: printf("breakpoint"); break;
    case OP_ASSERT: printf("assert"); break;
    default:        printf("!Error! %d", c); break;
  }
*/
}
