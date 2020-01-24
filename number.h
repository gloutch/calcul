#ifndef NUMBER_H
#define NUMBER_H

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum number_type {
	INTEGER,
	BIG_INT,
	NAN,
};

struct number {
	enum number_type type;
	union {
		long integer;
	} data;
};

void print_number(const struct number * const num);

void copy_number(const struct number * const src, struct number * const dst);


struct number str_to_number(int len, const char * str);

struct number long_to_number(long l);


void test_number();

#endif // NUMBER_H