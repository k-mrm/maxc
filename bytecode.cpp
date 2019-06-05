#include "maxc.h"

namespace Bytecode {


void push_0arg(bytecode &self, OpCode op) {
    self.push_back((uint8_t)op);
}

void push_int8(bytecode &self, int8_t i8) {
    self.push_back((uint8_t)i8);
}


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

void push_store(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::STORE);

    push_int32(self, id);
}

void push_load(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::LOAD);

    push_int32(self, id);
}

void push_strset(bytecode &self, int id) {
    self.push_back((uint8_t)OpCode::STRINGSET);

    push_int32(self, id);
}



void push_int32(bytecode &self, int32_t i32) {
    self.push_back((uint8_t)((i32 >>  0) & 0xff));
    self.push_back((uint8_t)((i32 >>  8) & 0xff));
    self.push_back((uint8_t)((i32 >> 16) & 0xff));
    self.push_back((uint8_t)((i32 >> 24) & 0xff));
}

void replace_int32(size_t cpos, bytecode &dst, size_t src) {
    dst[cpos + 1] = ((uint8_t)((src >>  0) & 0xff));
    dst[cpos + 2] = ((uint8_t)((src >>  8) & 0xff));
    dst[cpos + 3] = ((uint8_t)((src >> 16) & 0xff));
    dst[cpos + 4] = ((uint8_t)((src >> 24) & 0xff));
}

int32_t read_int32(bytecode &self, size_t &pc) {
    int32_t a = (int32_t)(((uint8_t)self[pc + 3] << 24)
                        + ((uint8_t)self[pc + 2] << 16)
                        + ((uint8_t)self[pc + 1] <<  8)
                        + ((uint8_t)self[pc + 0]     ));

    pc += 4;

    return a;
}


}   //Bytecode
