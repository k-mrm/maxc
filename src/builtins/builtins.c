#include "builtins.h"

NodeVariable *bltin_print;
NodeVariable *bltin_println;
NodeVariable *bltin_string_size;
NodeVariable *bltin_int_to_float;
NodeVariable *bltin_objectid;
NodeVariable *bltin_error;
NodeVariable *bltin_exit;
NodeVariable *bltin_readline;

NodeVariable *bltin_methods[] = {
    bltin_print,
    bltin_println,
    bltin_string_size,
    bltin_int_to_float,
    bltin_objectid,
    bltin_error,
    bltin_exit,
    bltin_readline,
};
