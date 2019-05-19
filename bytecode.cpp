#include "maxc.h"

namespace Bytecode {


void push_0arg(bytecode &self, OpCode op) {
    debug("Opcode: %d\n", (uint8_t)op);
    self.push_back((uint8_t)op);
}

void push_int8(bytecode &self, int8_t i8) {
    self.push_back((uint8_t)i8);
}

void push_int32(bytecode &self, int32_t i32) {
    self.push_back((uint8_t)((i32 >>  0) & 0xff));
    self.push_back((uint8_t)((i32 >>  8) & 0xff));
    self.push_back((uint8_t)((i32 >> 16) & 0xff));
    self.push_back((uint8_t)((i32 >> 24) & 0xff));

    debug("%d\n", (uint8_t)((i32 >>  0) & 0xff));
    debug("%d\n", (uint8_t)((i32 >>  8) & 0xff));
    debug("%d\n", (uint8_t)((i32 >> 16) & 0xff));
    debug("%d\n", (uint8_t)((i32 >> 24) & 0xff));
}

void push_ipush(bytecode &self, int32_t i32) {
    self.push_back((uint8_t)OpCode::IPUSH);
    push_int32(self, i32);
}

int32_t read_int32(bytecode &self, size_t pc) {
    ;
}


}
