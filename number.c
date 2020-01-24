#include "number.h"




void print_number(const struct number * const num) {

	switch (num->type) {

		case INTEGER:
			printf("INTEGER %ld ", num->data.integer);
			return;

		case BIG_INT:
			printf("BIG_INT ... ");
			return;

		case NAN:
			printf("NAN");
			return;
	}
	assert(0);
}


void copy_number(const struct number * const src, struct number * const dst) {
	*dst = *src;
}



struct number str_to_number(int len, const char * str) {
	assert(str != NULL);

	char * point = strchr(str, '.');
	if (point != NULL) { // if there is a point
		len = point - str; // for now NO FLOAT
	}

	struct number num;

	if (len < 19) { // fit in long integer
		num.type = INTEGER;
		num.data.integer = strtol(str, NULL, 10);
		if (!errno) {
			return num;
		}
	}
	num.type = NAN;
	return num;
}


struct number long_to_number(long l) {
	struct number num;
	num.type = INTEGER;
	num.data.integer = l;
	return num;
}



/*
	TEST
*/


void test_number() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else
	printf("NUMBER:\n");


	printf(" long_to_number\n");
	struct number ln1 = long_to_number(LONG_MIN);
	assert(ln1.type == INTEGER);
	assert(ln1.data.integer == LONG_MIN);

	struct number ln2 = long_to_number(LONG_MAX);
	assert(ln2.type == INTEGER);
	assert(ln2.data.integer == LONG_MAX);


	printf(" str_to_number\n");
	struct number sn1 = str_to_number(4, "1234");
	assert(sn1.type == INTEGER);
	assert(sn1.data.integer == 1234);
	

	printf("done\n");
	#endif
}