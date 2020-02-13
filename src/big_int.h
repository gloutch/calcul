#ifndef BIG_INT_H
#define BIG_INT_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "limits.h"
#include "log.h"
#include "string.h"


struct big_int * long_to_big(long l);

struct big_int * str_to_big(int len, const char * str, unsigned int base);


int big_int_length(const struct big_int * b);

void big_int_neg(struct big_int * b);

int big_int_cmp(const struct big_int * b1, const struct big_int * b2);


// operation (b2 should be free after)
// b1 is reused to store the result
struct big_int * big_int_add(struct big_int * b1, struct big_int * b2);

struct big_int * big_int_sub(struct big_int * b1, struct big_int * b2);

struct big_int * big_int_mul(struct big_int * b1, struct big_int * b2);

struct big_int * big_int_sqr(struct big_int * b);

struct big_int * big_int_pow(struct big_int * b, long expo);


// return LONG_MIN if big_int can't fit in an long
long big_to_long(const struct big_int * big);

void big_int_print(const struct big_int * const big);

void big_int_free(struct big_int * big);


// test
void test_big_int();

#endif // BIG_INT_H

