#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "maxc.h"

ReadStatus intern_readline(size_t alloc,
                           size_t *cursor,
                           char *end,
                           size_t end_len) {
    ReadStatus status = {
        .err.eof = 0,
        .err.toolong = 0
    };
    char a[alloc];
    memset(a, 0, alloc);
    char last_char;
    char *str;
    size_t max = alloc - end_len - 1;
    *cursor = 0;

    while((last_char = getchar()) != '\n') {
        if(last_char == EOF) {
            status.err.eof = 1;
            goto err;
        }

        a[(*cursor)++] = last_char;

        if(max <= *cursor) {
            status.err.toolong = 1;
            goto err;
        }
    }

    str = malloc(sizeof(char) * (*cursor + end_len + 1));
    sprintf(str, "%s%s", a, end);

    status.str = str;
    return status;

err:
    status.str = NULL;
    return status;
}

const unsigned int intern_ascii_to_numtable[] = {
     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
/*0*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*1*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*2*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*3*/   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,99,99,99,99,99,99,
/*4*/  99,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
/*5*/  25,26,27,28,29,30,31,32,33,34,35,99,99,99,99,99,
/*6*/  99,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
/*7*/  25,26,27,28,29,30,31,32,33,34,35,99,99,99,99,99,
/*8*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*9*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*a*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*b*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*c*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*d*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*e*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
/*f*/  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
};

int64_t intern_scan_digiti(char *str, int base, int *overflow, size_t *len) {
    char *s = str;
    int64_t res = 0;
    int64_t tmp;
    int d;
    int64_t mulov_border = INT64_MAX / base;

    while(*s) {
        d = intern_ascii_to_numtable[(int)*s++];
        if(d >= base) {
            --s;
            // TODO: raise error
            break;
        }
        if(res > mulov_border) {
            *overflow = 1;
            --s;
            break;
        }
        res *= base;
        tmp = res;
        res += d;
        if(res < tmp) {
            *overflow = 1;
            --s;
            res = tmp / base;
            break;
        }
    }
    *len = s - str;

    return res;
}

void *xmalloc(size_t n) {
    void *p = malloc(n);
    if(!p) {
        intern_die("No Memory Error");
    }
    return p;
}

void intern_die(char *msg) {
    fprintf(stderr, "maxc die: %s\n", msg);
    intern_abort();
}

void intern_abort() {
    fprintf(stderr, "aborted.\n");
    abort();
}
