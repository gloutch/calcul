#ifndef BIG_INT_H
#define BIG_INT_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"


struct big_int * long_to_big(long l);

struct big_int * str_to_big(int len, const char * str, unsigned int base);


void big_int_neg(struct big_int * b);

void big_int_swap(struct big_int * b1, struct big_int * b2);

int big_int_cmp(const struct big_int * b1, const struct big_int * b2);


// operation (b2 may be free after)
// b1 is reused to store the result
struct big_int * big_int_add(struct big_int * b1, struct big_int * b2);

struct big_int * big_int_sub(struct big_int * b1, struct big_int * b2);

struct big_int * big_int_mul(struct big_int * b1, struct big_int * b2);


void big_int_print(const struct big_int * const big);

void big_int_free(struct big_int * big);


// test
void test_big_int();

#endif // BIG_INT_H

