#ifndef BIG_INT_H
#define BIG_INT_H

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"


struct big_int * str_to_big(int len, const char * str, unsigned int base);

struct big_int * long_to_big(long l);


// operation (pls use the returned ptr instead of b1) b2 may be free after
// struct big_int * add_big_int(struct big_int * b1, const struct big_int * b2); // b1 = b1 + b2

// struct big_int * sub_big_int(struct big_int * b1, const struct big_int * b2);

// struct big_int * mult_big_int(struct big_int * b1, const struct big_int * b2);


// void neg_big_int(struct big_int * big);

void print_big_int(const struct big_int * const big);

void free_big_int(struct big_int * big);


// test
void test_big_int();

#endif // BIG_INT_H
