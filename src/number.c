#include "number.h"



static void integer_to_big(struct number * num) {
	assert(num->type == INTEGER);
	long tmp_log = num->data.integer;
	num->type = BIG;
	num->data.big = long_to_big(num->data.integer);
	log_info("int %ld -> big_int @%p", tmp_log, num->data.big);
}

static struct number long_to_number(long l) {
	struct number num;
	num.type = INTEGER;
	num.data.integer = l;
	return num;
}

static struct number str_to_number_base(int len, const char * str, int base) {
	assert(str != NULL);
	assert(len > 0);
	assert(base >= 1);

	log_debug("struct number from '%.*s' [base %d]", len, str, base);

	char * point = strchr(str, '.');
	if ((point != NULL) && (point < str + len)) { // if there is a point
		log_warn("sorry, no float yet (float %.*s -> int %.*s) [base %d]", len, str, point - str, str, base);
		len = point - str; // for now NO FLOAT
	}

	struct number num;

	if (base == 1) { // unary base
		return long_to_number(len);
	}
	assert(base >= 2);

	// try to fit in an `long`
	num.data.integer = strtol(str, NULL, base);
	if (!errno) {
		log_info("int %ld ", num.data.integer);
		num.type = INTEGER;
		return num;
	} else {
		int errsv = errno;
		log_info("strtol() failed with str '%.*s...' [base %d] (%s), trying with str_to_big()", 11, str, base, strerror(errsv));
		errno = 0;
	}

	// try in a `struct big_int`
	num.data.big = str_to_big(len, str, base);
	log_info("big '%.*s' [base %d] = @%p", len, str, base, num.data.big);
	num.type = BIG;
	return num;
}

struct number str_to_number(int len, const char * str) {

	if ((len >= 2) && (str[1] == 'x')) {
		int base = str[0] - '0';
		assert((0 <= base) && (base < 10));

		if (base == 0) {
			return str_to_number_base(len - 2, str + 2, 16);
		}
		return str_to_number_base(len - 2, str + 2, base); 
	}
	return str_to_number_base(len, str, 10);
}



void number_neg(struct number * n1) {

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
	log_info("int %ld", n1->data.integer);
}

// function perform operation on big_int (like `big_int_add`)
typedef struct big_int * (big_operation)(struct big_int * b1, struct big_int * b2);

static void convert_to_big_op(struct number * n1, struct number * n2, big_operation op) {
	if (n1->type == INTEGER) {
		integer_to_big(n1);
	}
	if (n2->type == INTEGER) {
		integer_to_big(n2);
	}
	assert(n1->type == BIG);
	assert(n2->type == BIG);
	n1->data.big = op(n1->data.big, n2->data.big);
}

// I would like to factorize those function with function pointer, but builtins don't have addresses 
void number_add(struct number * n1, struct number * n2) {

	if ((n1->type != INTEGER) || (n2->type != INTEGER)) {
		convert_to_big_op(n1, n2, big_int_add);
		return;
	}
	assert(n1->type == INTEGER);
	assert(n2->type == INTEGER);

	long res = 0;
	if (__builtin_saddl_overflow(n1->data.integer, n2->data.integer, &res)) { // overflow has occurred
		log_info("add prevent overflow");
		convert_to_big_op(n1, n2, big_int_add);
		return;
	}
	log_info("int %ld + %ld = %ld", n1->data.integer, n2->data.integer, res);
	n1->data.integer = res;
}

void number_sub(struct number * n1, struct number * n2) {

	if ((n1->type != INTEGER) || (n2->type != INTEGER)) {
		convert_to_big_op(n1, n2, big_int_sub);
		return;
	}
	assert(n1->type == INTEGER);
	assert(n2->type == INTEGER);

	long res = 0;
	if (__builtin_ssubl_overflow(n1->data.integer, n2->data.integer, &res)) { // overflow has occurred
		log_info("sub prevent overflow");
		convert_to_big_op(n1, n2, big_int_sub);
		return;
	}
	log_info("int %ld - %ld = %ld", n1->data.integer, n2->data.integer, res);
	n1->data.integer = res;
}

void number_mul(struct number * n1, struct number * n2) {

	if ((n1->type != INTEGER) || (n2->type != INTEGER)) {
		convert_to_big_op(n1, n2, big_int_mul);
		return;
	}
	assert(n1->type == INTEGER);
	assert(n2->type == INTEGER);

	long res = 0;
	if (__builtin_smull_overflow(n1->data.integer, n2->data.integer, &res)) { // overflow has occurred
		log_info("mult prevent overflow");
		convert_to_big_op(n1, n2, big_int_mul);
		return;
	}
	log_info("int %ld * %ld = %ld", n1->data.integer, n2->data.integer, res);
	n1->data.integer = res;
}

void number_pow(struct number * n1, struct number * n2) {

	long expo;
	if (n2->type == BIG) { // exponent shouldn't be a big_int
		// try to convert is in a long
		expo = big_to_long(n2->data.big);
		if (expo == LONG_MIN) { // convertion failed
			error_set(POW_BIG, NULL, NULL, 0);
			return;
		}
	} 
	else {
		expo = n2->data.integer;
	}
	if (expo < 0) {
		error_set(POW_NEG, NULL, NULL, 0);
		return;
	}

	assert(expo >= 0);
	if (n1->type == INTEGER) {
		integer_to_big(n1);
	}
	n1->data.big = big_int_pow(n1->data.big, expo);
}



static void number_print_long(long num) {
	if (num >= 0) {
		printf("INT %#lx = %ld", num, num);
	} 
	else if (num == LONG_MIN) { // -LONG_MIN doesn't fit in a long
		printf("INT -0x8000000000000000 = %ld", LONG_MIN);
	} 
	else {
		printf("INT -%#lx = %ld", -num, num);
	}
	return;
}

void number_print(const struct number * const num) {

	if (num->type == BIG) {
		long r = big_to_long(num->data.big);
		if (r != LONG_MIN) { // try to print in a integer format
			number_print_long(r);
			return;
		}
		printf("BIG ");
		big_int_print(num->data.big);
		return;
	}
	assert(num->type == INTEGER);
	number_print_long(num->data.integer);
	return;
}

void number_copy(const struct number * const src, struct number * const dst) {
	*dst = *src;
}

void number_free(struct number num) {
	if (num.type == BIG) {
		big_int_free(num.data.big);
	}
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
	struct number sn1 = str_to_number(4, "1234");
	assert(sn1.type == INTEGER);
	assert(sn1.data.integer == 1234);
	number_free(sn1);

	struct number sn2 = str_to_number(20, "92233720368547175808");
	assert(sn2.type == BIG);
	number_free(sn2);

	struct number sn3 = str_to_number(10, "5x1231301");
	assert(sn3.type == INTEGER);
	assert(sn3.data.integer == 23951);
	number_free(sn3);

	struct number sn4 = str_to_number(18, "1x000000000000000");
	assert(sn4.type == INTEGER);
	assert(sn4.data.integer == 16);
	number_free(sn4);


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
	struct number ne2 = str_to_number(19, "9223372036854775808");
	assert(ne2.type == BIG);
	assert(big_int_cmp(ne.data.big, ne2.data.big) == 0);
	number_free(ne2);
	number_free(ne);


	printf(" number_add\n");
	struct number a1 = long_to_number((long) 9213372036854775807);
	struct number a2 = long_to_number((long) 10000000000000002);
	assert(a1.type == INTEGER);
	assert(a2.type == INTEGER);
	struct number r12 = str_to_number(19, "9223372036854775809");
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

