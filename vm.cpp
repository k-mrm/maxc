#include "maxc.h"

int VM::run(std::vector<vmcode_t> code) {
    if(code.empty())
        return 1;
    for(auto c: code)
        exec(c);
    return 0;
}

void VM::exec(vmcode_t c) {
    switch(c.type) {
        case OPCODE::PUSH: {
            int a = c.value;
            s.push(value_t(a));
        } break;
        case OPCODE::ADD: {
            auto r = s.top(); s.pop();
            auto l = s.top(); s.pop();
            s.push(value_t(l.num + r.num));
        } break;
        case OPCODE::SUB: {
            auto r = s.top(); s.pop();
            auto l = s.top(); s.pop();
            s.push(value_t(l.num - r.num));
        } break;
        case OPCODE::MUL: {
            auto r = s.top(); s.pop();
            auto l = s.top(); s.pop();
            s.push(value_t(l.num * r.num));
        } break;
        case OPCODE::DIV: {
            auto r = s.top(); s.pop();
            auto l = s.top(); s.pop();
            s.push(value_t(l.num / r.num));
        } break;
        case OPCODE::PRINT: {
            std::cout << s.top().num << std::endl;
        } break;
        default:
            error("???");
    }
}
