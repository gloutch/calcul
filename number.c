#include "number.h"



static void integer_to_big(struct number * num) {
	assert(num->type == INTEGER);
	long tmp_log = num->data.integer;
	num->type = BIG;
	num->data.big = long_to_big(num->data.integer);
	log_info("int %ld -> big_int ptr %p", tmp_log, num->data.big);
}

static struct number long_to_number(long l) {
	struct number num;
	num.type = INTEGER;
	num.data.integer = l;
	return num;
}

struct number str_to_number(int len, const char * str, int base) {
	assert(str != NULL);

	char * point = strchr(str, '.');
	if (point != NULL) { // if there is a point
		log_warn("float %.*s -> int %.*s (sorry, no float yet)", len, str, point - str, str);
		len = point - str; // for now NO FLOAT
	}

	struct number num;

	if (len < 19) { // fit in long integer
		num.type = INTEGER;
		num.data.integer = strtol(str, NULL, base);
		if (!errno) {
			return num;
		}
		log_warn("strtol failed on num %.*s (trying with big_int)", len, str);
	}
	num.type = BIG;
	num.data.big = str_to_big(len, str, base);
	return num;
}



void number_neg(struct number * n1) {
	assert(n1->type != NAN);

	if (n1->type == BIG) {
		big_int_neg(n1->data.big);
		return;
	}

	assert(n1->type == INTEGER);

	if (n1->data.integer == LONG_MIN) { // special case
		integer_to_big(n1);
		big_int_neg(n1->data.big);
		return;
	}

	n1->data.integer = -n1->data.integer;
}


static void add_big(struct number * n1, struct number * n2) {

	if (n1->type == INTEGER) {
		integer_to_big(n1);
	}
	if (n2->type == INTEGER) {
		integer_to_big(n2);
	}
	assert(n1->type == BIG);
	assert(n2->type == BIG);
	big_int_add(n1->data.big, n2->data.big);
}

void number_add(struct number * n1, struct number * n2) {

	if ((n1->type != INTEGER) || (n2->type != INTEGER)) {
		add_big(n1, n2);
	}
	assert(n1->type == INTEGER);
	assert(n2->type == INTEGER);

	long res = 0;
	if (__builtin_saddl_overflow(n1->data.integer, n2->data.integer, &res)) { // overflow has occurred
		log_info("prevent overflow");
		add_big(n1, n2);
		return;
	}
	log_info("int %ld + %ld = %ld", n1->data.integer, n2->data.integer, res);
	n1->data.integer = res;
}


void number_sub(struct number * n1, struct number * n2) {	
}

void number_mul(struct number * n1, struct number * n2) {	
}



void number_print(const struct number * const num) {

	switch (num->type) {

		case INTEGER:
			printf("INTEGER %#lx ", num->data.integer);
			return;

		case BIG:
			printf("BIG_INT ");
			big_int_print(num->data.big);
			printf(" ");
			return;

		case NAN:
			printf("NAN ");
			return;
	}
	assert(0);
}

void number_copy(const struct number * const src, struct number * const dst) {
	*dst = *src;
}

void number_free(struct number num) {

	if (num.type == BIG) {
		big_int_free(num.data.big);
	}
	num.type = NAN;
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


	printf(" str_to_number\n");
	struct number sn1 = str_to_number(4, "1234", 10);
	assert(sn1.type == INTEGER);
	assert(sn1.data.integer == 1234);
	number_free(sn1);

	struct number sn2 = str_to_number(20, "92233720368547175808", 10);
	assert(sn2.type == BIG);
	number_free(sn2);


	printf(" number_neg\n");
	struct number ne = long_to_number(56);
	number_neg(&ne);
	assert(ne.data.integer == -56);
	number_free(ne);

	ne = long_to_number(LONG_MAX);
	number_neg(&ne);
	assert(ne.data.integer == (LONG_MIN + 1));
	number_free(ne);

	ne = long_to_number(LONG_MIN); // -9223372036854775808
	number_neg(&ne);
	assert(ne.type == BIG);
	struct number ne2 = str_to_number(19, "9223372036854775808", 10);
	assert(ne2.type == BIG);
	assert(big_int_cmp(ne.data.big, ne2.data.big) == 0);
	number_free(ne2);
	number_free(ne);


	printf(" number_add\n");
	struct number a1 = long_to_number((long) 9213372036854775807);
	struct number a2 = long_to_number((long) 10000000000000002);
	assert(a1.type == INTEGER);
	assert(a2.type == INTEGER);
	struct number r12 = str_to_number(19, "9223372036854775809", 10);
	assert(r12.type == BIG);
	number_add(&a1, &a2); // 9223372036854775809 > LONG_MAX + 2 -> overflow
	assert(a1.type == BIG);
	assert(big_int_cmp(a1.data.big, r12.data.big) == 0);
	number_free(a1);
	number_free(a2);
	number_free(r12);


	printf("done\n\n");
	#endif
}

