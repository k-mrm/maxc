#include "bytecode.h"
#include "literalpool.h"
#include "maxc.h"

namespace Bytecode {

void push_0arg(bytecode &self, OpCode op) { self.push_back((uint8_t)op); }

void push_int8(bytecode &self, int8_t i8) { self.push_back((uint8_t)i8); }

void push_ipush(bytecode &self, int32_t i32) {
    self.push_back((uint8_t)OpCode::IPUSH);

    push_int32(self, i32);
}

void push_jmp(bytecode &self, size_t pc) {
    self.push_back((uint8_t)OpCode::JMP);

    push_int32(self, (int32_t)pc);
}

void push_jmpneq(bytecode &self, size_t pc) {
    self.push_back((uint8_t)OpCode::JMP_NOTEQ);

    push_int32(self, (int32_t)pc);
}

void push_store(bytecode &self, int id, bool isglobal) {
    self.push_back(isglobal ? (uint8_t)OpCode::STORE_GLOBAL
                            : (uint8_t)OpCode::STORE_LOCAL);

    push_int32(self, id);
}

void push_load(bytecode &self, int id, bool isglobal) {
    self.push_back(isglobal ? (uint8_t)OpCode::LOAD_GLOBAL
                            : (uint8_t)OpCode::LOAD_LOCAL);

    push_int32(self, id);
}

void push_strset(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::STRINGSET);

    push_int32(self, id);
}

void push_fpush(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::FPUSH);

    push_int32(self, id);
}

void push_functionset(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::FUNCTIONSET);

    push_int32(self, id);
}

void push_bltinfn_set(bytecode &self, BltinFnKind n) {
    self.push_back((uint8_t)OpCode::BLTINFN_SET);

    push_int32(self, (int)n);
}

void push_structset(bytecode &self, int nfield) {
    self.push_back((uint8_t)OpCode::STRUCTSET);

    push_int32(self, nfield);
}

void push_bltinfn_call(bytecode &self, int nargs) {
    self.push_back((uint8_t)OpCode::CALL_BLTIN);

    push_int32(self, nargs);
}

void push_member_load(bytecode &self, int offset) {
    self.push_back((uint8_t)OpCode::MEMBER_LOAD);

    push_int32(self, offset);
}

void push_member_store(bytecode &self, int offset) {
    self.push_back((uint8_t)OpCode::MEMBER_STORE);

    push_int32(self, offset);
}

void push_int32(bytecode &self, int32_t i32) {
    self.push_back((uint8_t)((i32 >> 0) & 0xff));
    self.push_back((uint8_t)((i32 >> 8) & 0xff));
    self.push_back((uint8_t)((i32 >> 16) & 0xff));
    self.push_back((uint8_t)((i32 >> 24) & 0xff));
}

void replace_int32(size_t cpos, bytecode &dst, size_t src) {
    dst[cpos + 1] = ((uint8_t)((src >> 0) & 0xff));
    dst[cpos + 2] = ((uint8_t)((src >> 8) & 0xff));
    dst[cpos + 3] = ((uint8_t)((src >> 16) & 0xff));
    dst[cpos + 4] = ((uint8_t)((src >> 24) & 0xff));
}

int32_t read_int32(uint8_t self[], size_t &pc) { // for bytecode shower
    int32_t a = (int32_t)(
        ((uint8_t)self[pc + 3] << 24) + ((uint8_t)self[pc + 2] << 16) +
        ((uint8_t)self[pc + 1] << 8) + ((uint8_t)self[pc + 0]));

    pc += 4;

    return a;
}

#ifdef MXC_DEBUG
void show(uint8_t a[], size_t &i, LiteralPool &ltable) {
    printf("%04ld ", i);

    switch((OpCode)a[i++]) {
    case OpCode::PUSH:
        printf("push");
        break;
    case OpCode::IPUSH: {
        int i32 = read_int32(a, i);
        printf("ipush %d", i32);
        break;
    }
    case OpCode::PUSHCONST_0:
        printf("pushconst0");
        break;
    case OpCode::PUSHCONST_1:
        printf("pushconst1");
        break;
    case OpCode::PUSHCONST_2:
        printf("pushconst2");
        break;
    case OpCode::PUSHCONST_3:
        printf("pushconst3");
        break;
    case OpCode::PUSHTRUE:
        printf("pushtrue");
        break;
    case OpCode::PUSHFALSE:
        printf("pushfalse");
        break;
    case OpCode::FPUSH: {
        int id = read_int32(a, i);

        printf("fpush %lf", ltable.table[id].number);
        break;
    }
    case OpCode::POP:
        printf("pop");
        break;
    case OpCode::ADD:
        printf("add");
        break;
    case OpCode::SUB:
        printf("sub");
        break;
    case OpCode::MUL:
        printf("mul");
        break;
    case OpCode::DIV:
        printf("div");
        break;
    case OpCode::MOD:
        printf("mod");
        break;
    case OpCode::LOGOR:
        printf("or");
        break;
    case OpCode::LOGAND:
        printf("and");
        break;
    case OpCode::EQ:
        printf("eq");
        break;
    case OpCode::NOTEQ:
        printf("noteq");
        break;
    case OpCode::LT:
        printf("lt");
        break;
    case OpCode::LTE:
        printf("lte");
        break;
    case OpCode::GT:
        printf("gt");
        break;
    case OpCode::GTE:
        printf("gte");
        break;
    case OpCode::FADD:
        printf("fadd");
        break;
    case OpCode::FSUB:
        printf("fsub");
        break;
    case OpCode::FMUL:
        printf("fmul");
        break;
    case OpCode::FDIV:
        printf("fdiv");
        break;
    case OpCode::FMOD:
        printf("fmod");
        break;
    case OpCode::FLOGOR:
        printf("for");
        break;
    case OpCode::FLOGAND:
        printf("fand");
        break;
    case OpCode::FEQ:
        printf("feq");
        break;
    case OpCode::FNOTEQ:
        printf("fnoteq");
        break;
    case OpCode::FLT:
        printf("flt");
        break;
    case OpCode::FLTE:
        printf("flte");
        break;
    case OpCode::FGT:
        printf("fgt");
        break;
    case OpCode::FGTE:
        printf("fgte");
        break;
    case OpCode::INC:
        printf("inc");
        break;
    case OpCode::DEC:
        printf("dec");
        break;
    case OpCode::JMP: {
        int i32 = read_int32(a, i);
        printf("jmp %d", i32);
        break;
    }
    case OpCode::JMP_EQ:
        printf("jmpeq");
        break;
    case OpCode::JMP_NOTEQ: {
        int i32 = read_int32(a, i);
        printf("jmpneq %d", i32);
        break;
    }
    case OpCode::FORMAT:
        printf("format");
        break;
    case OpCode::TYPEOF:
        printf("typeof");
        break;
    case OpCode::STORE_LOCAL: {
        int id = read_int32(a, i);

        printf("store_local %d", id);

        break;
    }
    case OpCode::STORE_GLOBAL: {
        int id = read_int32(a, i);

        printf("store_global %d", id);

        break;
    }
    case OpCode::LISTSET:
        printf("listset");
        break;
    case OpCode::SUBSCR:
        printf("subscr");
        break;
    case OpCode::SUBSCR_STORE:
        printf("subscr_store");
        break;
    case OpCode::STRINGSET: {
        int k = read_int32(a, i);
        printf("stringset %s", ltable.table[k].str.c_str());
        break;
    }
    case OpCode::TUPLESET:
        printf("tupleset");
        break;
    case OpCode::FUNCTIONSET: {
        int k = read_int32(a, i);
        userfunction f = ltable.table[k].func;

        printf("funcset ->\n");

        printf("length: %lu\n", f.codelength);

        for(size_t n = 0; n < f.codelength;) {
            printf("  ");
            show(f.code, n, ltable);
            puts("");
        }

        break;
    }
    case OpCode::BLTINFN_SET: {
        int n = read_int32(a, i);

        printf("bltinfn %d", n);

        break;
    }
    case OpCode::STRUCTSET: {
        int n = read_int32(a, i);

        printf("structset %d", n);

        break;
    }
    case OpCode::LOAD_GLOBAL: {
        int id = read_int32(a, i);

        printf("load_global %d", id);

        break;
    }
    case OpCode::LOAD_LOCAL: {
        int id = read_int32(a, i);

        printf("load_local %d", id);

        break;
    }
    case OpCode::RET:
        printf("ret");
        break;
    case OpCode::CALL:
        printf("call");
        break;
    case OpCode::CALL_BLTIN: {
        int n = read_int32(a, i);

        printf("bltinfn-call arg:%d", n);

        break;
    }
    case OpCode::END:
        printf("end");
        break;
    case OpCode::MEMBER_LOAD: {
        int n = read_int32(a, i);

        printf("member-load %d", n);

        break;
    }
    case OpCode::MEMBER_STORE: {
        int n = read_int32(a, i);

        printf("member-store %d", n);

        break;
    }
    default:
        printf("!Error!");
        break;
    }
}
#endif

} // namespace Bytecode
