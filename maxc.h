#ifndef MAXC_H
#define MAXC_H

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "token.h"

class Maxc {
  public:
    int run(std::string src);

    void show_usage();

  private:
    std::string version = "0.0.2";
};

enum class ErrorKind {};

void error(const char *, ...);
void error(const location_t &, const location_t &, const char *, ...);
void warning(const location_t &, const location_t &, const char *, ...);
void runtime_err(const char *, ...);
void debug(const char *, ...);
void showline(int, int);

#endif
