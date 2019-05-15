#include "maxc.h"

void Bytecode::push_0arg(bytecode &iseq, OpCode op) {
    iseq.push_back((uint8_t)op);
}

void Bytecode::push_int8(bytecode &iseq, int8_t i8) {
    iseq.push_back((uint8_t)i8);
}

void Bytecode::push_int32(bytecode &iseq, int32_t i32) {
    iseq.push_back((uint8_t)((i32 >>  0) & 0xff));
    iseq.push_back((uint8_t)((i32 >>  8) & 0xff));
    iseq.push_back((uint8_t)((i32 >> 16) & 0xff));
    iseq.push_back((uint8_t)((i32 >> 24) & 0xff));
}
