#include "maxc.h"

void error(std::string msg) {
    std::cerr << "[error] " << msg << std::endl;
    exit(1);
}

void noexit_error(std::string msg) {
    std::cerr << "[error] " << msg << std::endl;
}
