#ifndef NUMBER_H
#define NUMBER_H

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "big_int.h"
#include "log.h"


struct number {
	enum {
		INTEGER,
		BIG,
		NAN,
	} type;

	union {
		long integer;
		struct big_int * big;
	} data;
};


struct number str_to_number(int len, const char * str, int base);


// n2 may be free after each operation, the result is in n1
void number_neg(struct number * n1);

void number_add(struct number * n1, struct number * n2);

void number_sub(struct number * n1, struct number * n2);

void number_mul(struct number * n1, struct number * n2);


void number_print(const struct number * const num);

void number_copy(const struct number * const src, struct number * const dst);

void number_free(struct number num);


void test_number();

#endif // NUMBER_H 
