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
#include <list>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

//#define MXC_DEBUG

class Maxc {
  public:
    int run(std::string src);

    void show_usage();

  private:
    std::string version = "0.0.2";
};

#endif
