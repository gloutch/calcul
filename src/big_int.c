#include "big_int.h"

#define BASE 256
#define LONG_SIZE 16


struct big_int {
	unsigned char * bin; // array of bytes (like little endian)
	unsigned int len; // number of used char int `bin`
	unsigned int cap; // real capacity of `bin`
	enum {
		POSITIVE,
		NEGATIVE,
	} sign;
};

static const unsigned char one_bin [] = {1};
static const unsigned char zero_bin[] = {0};
static const struct big_int BIG_ONE  = {(unsigned char *)  one_bin, 1, 1, POSITIVE};
static const struct big_int BIG_ZERO = {(unsigned char *) zero_bin, 1, 1, POSITIVE};


// malloc a valid zero
static struct big_int * malloc_big_int(int cap) {

	struct big_int * big = malloc(sizeof(struct big_int) + (sizeof(char) * cap));
	CHECK_MALLOC(big, "malloc_big_int");

	big->bin  = (unsigned char *) &big[1];
	big->sign = POSITIVE;
	big->cap  = cap;
	big->len  = 1;

	big->bin[0] = 0;
	return big;
}

static void realloc_big_int(struct big_int ** big, int cap) {
	assert((*big)->cap < cap);
	// printf("bit int: realloc\n");

	int sign = (*big)->sign;
	int len  = (*big)->len;

	struct big_int * new = realloc(*big, sizeof(struct big_int) + (sizeof(char) * cap));
	CHECK_MALLOC(new, "realloc_big_int");
	log_debug("realloc %p -> %p", *big, new);

	new->bin  = (unsigned char *) &new[1];
	new->sign = sign;
	new->len  = len;
	new->cap  = cap;

	*big = new; 
}

// num is a `len` long array of digit (base `base`)
// return the new size of the number represented in `num`
static int digit_div_2(int len, unsigned char * num, unsigned int base) {

	unsigned short rem = 0;

	for (int i = len - 1; i >= 0; i--) {
		rem = (unsigned short) num[i] + rem;
		num[i] = (unsigned char) (rem / 2);
		rem = ((rem % 2) ? base : 0);
	}
	return (num[len - 1] ? len : len - 1); // new size
}

static struct big_int * digit_to_big_int(int len, unsigned char * num, unsigned int base) {

	struct big_int * big = malloc_big_int(len);
	int i = 0;

	do {
		unsigned char tmp = 0;

		for (int j = 0; j < 8; j++) {

			if (num[0] % 2) { 
				tmp ^= (1 << j); // num is even
			}
			len = digit_div_2(len, num, base);
			if (len == 0) { // num is zero
				break;
			}
		}
		big->bin[i] = tmp;
		i++;
		assert(i <= big->cap);

	} while (len > 0);

	big->len = i;
	return big;
}



struct big_int * long_to_big(long l) {

	struct big_int * big = malloc_big_int(LONG_SIZE); // that's long enough
	unsigned long num;

	if (l < 0) {
		big->sign = NEGATIVE;
		num = -l;
	} else {
		num = l;
	}

	int i = 0;
	do {
		big->bin[i] = (unsigned char) num;
		num = num >> 8;
		i++;
	} while (num);
	big->len = i;

	return big;
}

static int char_to_digit(char c) {
	if (isdigit(c)) {
		return (c - '0');
	}
	assert(isalpha(c));
	c = tolower(c);
	return 10 + (c - 'a');
}

struct big_int * str_to_big(int len, const char * str, unsigned int base) {
	assert(len  > 0);
	assert(base > 1);

	unsigned char * digit = malloc(sizeof(unsigned char) * len);
	CHECK_MALLOC(digit, "str_to_big (big_int.c)");

	for (int i = 0; i < len; i++) {
		digit[i] = char_to_digit(str[len - i - 1]);
	}

	struct big_int * big = digit_to_big_int(len, digit, base);
	free(digit);
	LOG_FREE(digit);
	return big;
}



void big_int_neg(struct big_int * b) {
	log_info("big -%p", b);
	b->sign = (b->sign == POSITIVE ? NEGATIVE : POSITIVE);
}

void big_int_swap(struct big_int * b1, struct big_int * b2) {	
	struct big_int tmp = *b2;
	*b2 = *b1;
	*b1 = tmp;
}

// don't check sign, compare as positive big_ints
static int big_int_cmp_bin(const struct big_int * b1, const struct big_int * b2) {

	if (b1->len != b2->len) {
		return (b1->len - b2->len);
	}

	int i = b1->len - 1;
	while ((i > 0) && (b1->bin[i] == b2->bin[i])) {
		i--;
	}
	return ((int) b1->bin[i] - (int) b2->bin[i]);
}

int big_int_cmp(const struct big_int * b1, const struct big_int * b2) {

	if (b1->sign == POSITIVE) {
		if (b2->sign == NEGATIVE) {
			return 1;
		}
		else {
			assert((b1->sign == POSITIVE) && (b2->sign == POSITIVE));
			return big_int_cmp_bin(b1, b2);
		}
	}
	else {
		if (b2->sign == POSITIVE) {
			return -1;
		}
		else {
			assert((b1->sign == NEGATIVE) && (b2->sign == NEGATIVE));
			return big_int_cmp_bin(b2, b1);
		}		
	}
}



// assume b1 is long enough to store the result
static void add_big(struct big_int * b1, const struct big_int * b2) {

	const struct big_int * i1; // shorter
	const struct big_int * i2; // longer
	if (b1->len < b2->len) {
		i1 = b1;
		i2 = b2;
	} else {
		i1 = b2;
		i2 = b1;
	}

	unsigned short rem = 0;
	int i = 0;

	while (i < i1->len) {

		rem = (unsigned short) i1->bin[i] + (unsigned short) i2->bin[i] + rem;
		b1->bin[i] = (unsigned char) rem; // result in b1
		rem = rem >> 8;
		i++;
	}
	while (i < i2->len) {

		rem = (unsigned short) i2->bin[i] + rem;
		b1->bin[i] = (unsigned char) rem;
		rem = rem >> 8;
		i++;
	}
	assert((0 <= rem) && (rem < BASE));
	if (rem > 0) {
		b1->bin[i] = (unsigned char) rem;
		i++;
	}

	b1->len = i;
}

struct big_int * big_int_add(struct big_int * b1, struct big_int * b2) {
	log_info("big %p + %p = %p", b1, b2, b1);

	// check sign
	if ((b1->sign == POSITIVE) && (b2->sign == NEGATIVE)) {
		b2->sign = POSITIVE;
		return big_int_sub(b1, b2);
	}
	if ((b1->sign == NEGATIVE) && (b2->sign == POSITIVE)) {
		b1->sign = POSITIVE;
		big_int_swap(b1, b2);
		return big_int_sub(b1, b2);
	}
	assert(b1->sign == b2->sign);


	struct big_int * result = b1;
	// check capacity
	int max_size = (b1->len < b2->len ? b2->len : b1->len);
	// the result of a int sum is a (max_size + 1) long integer
	if (result->cap <= max_size + 1) {
		realloc_big_int(&result, max_size * 2);
	}
	assert(result->cap > max_size + 1);

	// operation
	add_big(result, b2);
	return result;
}


// assume b1 is long enough to store the result, and b1 > b2
static void sub_big(struct big_int * b1, const struct big_int * b2) {
	assert(big_int_cmp_bin(b1, b2) > 0);

	unsigned short rem = 0;
	int i   = 0;
	int len = 0;

	while (i < b2->len) {

		rem = (unsigned short) b1->bin[i] - (unsigned short) b2->bin[i] - rem;
		b1->bin[i] = (unsigned char) rem;
		rem = rem >> 15;
		if (b1->bin[i]) { // if the last updated digit isn't zero
			len = i + 1;
		}
		i++;
	}
	while (i < b1->len) {

		rem = (unsigned short) b1->bin[i] - rem;
		b1->bin[i] = (unsigned char) rem;
		rem = rem >> 15;
		if (b1->bin[i]) {
			len = i + 1;
		}
		i++;
	}
	assert((0 <= rem) && (rem < BASE));
	if (rem > 0) {

		b1->bin[i] = b1->bin[i] - (unsigned char) rem;
		if (b1->bin[i]) {
			len = i + 1;
		}
	}

	assert(len > 0);
	b1->len = len;
}

struct big_int * big_int_sub(struct big_int * b1, struct big_int * b2) {
	log_info("big %p - %p = %p", b1, b2, b1);

	// check sign
	if (b2->sign == NEGATIVE) {
		b2->sign = POSITIVE;
		return big_int_add(b1, b2);
	}
	assert(b2->sign == POSITIVE);
	if (b1->sign == NEGATIVE) {
		b1->sign = POSITIVE;
		struct big_int * result = big_int_add(b1, b2); 
		result->sign = NEGATIVE;
		return result;
	}
	assert(b1->sign == POSITIVE);
	assert(b2->sign == POSITIVE);

	// operation
	int cmp = big_int_cmp_bin(b1, b2);

	if (cmp == 0) { // transform b1 to zero
		b1->sign = POSITIVE;
		b1->len = 1;
		b1->bin[0] = 0;
	}
	else if (cmp > 0) { // b1 > b2
		sub_big(b1, b2);
	}
	else { // b2 > b1
		big_int_swap(b1, b2);
		sub_big(b1, b2);
		big_int_neg(b1);
	}
	return b1;
}


// b1 is long enough, b2 and sum may be free after
static void mul_big(struct big_int * b1, struct big_int * b2, struct big_int * sum) {
	assert(big_int_cmp(sum, &BIG_ZERO) == 0);

	while ((b2->len != 1) || (b2->bin[0] != 1)) { // b2 != 1

		if (b2->bin[0] % 2) { // b2 is odd
			sub_big(b2, &BIG_ONE);
			add_big(sum, b1);
			b2->len = digit_div_2(b2->len, b2->bin, BASE);
			add_big(b1, b1);
		}
		else { // b2 is even
			b2->len = digit_div_2(b2->len, b2->bin, BASE);
			add_big(b1, b1);
		}
	}
	assert((b2->len == 1) && (b2->bin[0] == 1));
	add_big(b1, sum);
}

struct big_int * big_int_mul(struct big_int * b1, struct big_int * b2) {
	log_info("big %p * %p = %p", b1, b2, b1);;

	// check zero
	if (((b1->len == 1) && (b1->bin[0] == 0)) || // b1 == 0
		((b2->len == 1) && (b2->bin[0] == 0))) { // b2 == 0
		b1->sign   = POSITIVE;
		b1->len    = 1;
		b1->bin[0] = 0;
		return b1;
	}

	// quickly find the longer
	if (b1->len < b2->len) {
		big_int_swap(b1, b2);
	} 
	struct big_int * result = b1;
	result->sign = (b1->sign == b2->sign ? POSITIVE : NEGATIVE);
	
	// check capacity, the result should be store in a (b1->len + b2->len) long integer
	int max_size = b1->len + b2->len;
	if (result->cap <= max_size) {
		realloc_big_int(&result, max_size * 2);
	}
	assert(result->cap > max_size);

	// operation
	struct big_int * zero = malloc_big_int(max_size);
	assert(b2->len <= result->len);
	mul_big(result, b2, zero);
	big_int_free(zero);
	return result;
}



void big_int_print(const struct big_int * const big) {
	if (big->sign == NEGATIVE) {
		printf("-");
	}
	printf("0x");
	printf("%hhx", big->bin[big->len - 1]);

	for (int i = big->len - 2; i >= 0; i--) {
		printf("%02hhx", big->bin[i]);
	}
}

void big_int_free(struct big_int * big) {
	big->len = 0;
	big->cap = 0;
	LOG_FREE(big);
	free(big);
}



/*
	TEST
*/


void test_big_int() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else
	printf("BIG INT\n");


	printf(" digit_div_2\n");
	unsigned char num1[] = {1, 2, 3, 4, 5}; // 54321
	assert(digit_div_2(5, num1, 10) == 5); // 27160
	assert(num1[0] == 0);
	assert(num1[1] == 6);
	assert(num1[2] == 1);
	assert(num1[3] == 7);
	assert(num1[4] == 2);

	unsigned char num2[] = {9, 2, 11, 10, 2, 8, 12}; // 209 890 089 -> 0xC82AB29
	assert(digit_div_2(7, num2, 16) == 7); // 104 945 044 -> 0x6415594
	assert(num2[0] == 4);
	assert(num2[1] == 9);
	assert(num2[2] == 5);
	assert(num2[3] == 5);
	assert(num2[4] == 1);
	assert(num2[5] == 4);
	assert(num2[6] == 6);

	unsigned char num3[] = {3, 1}; // 13
	assert(digit_div_2(2, num3, 10) == 1); // 6
	assert(num3[0] == 6);
	assert(num3[1] == 0);

	struct big_int * d1 = long_to_big(0x2210);
	// big_int_print(d1);
	d1->len = digit_div_2(d1->len, d1->bin, BASE);
	// big_int_print(d1);
	big_int_free(d1);


	printf(" malloc_big_int\n");
	struct big_int * b1 = malloc_big_int(3);
	assert(b1->sign == POSITIVE);
	assert(b1->cap  == 3);
	assert(b1->len  == 1);
	assert(b1->bin[0] == 0);
	// big_int_print(b1);
	big_int_free(b1);


	printf(" digit_to_big_int\n");
	unsigned char num4[] = {2, 4}; // 42
	struct big_int * b4  = digit_to_big_int(2, num4, 10);
	assert(b4->sign == POSITIVE);
	assert(b4->len  == 1);
	assert(b4->cap  == 2);
	assert(b4->bin[0] == 42);
	// big_int_print(b4);
	big_int_free(b4);

	unsigned char num5[] = {9, 2, 11, 10, 2, 8, 12}; // 209 890 089 -> 0x C 82 AB 29
	struct big_int * b5  = digit_to_big_int(7, num5, 16);
	assert(b5->sign == POSITIVE);
	assert(b5->len  == 4);
	assert(b5->cap  == 7);
	assert(b5->bin[0] == 0x29);
	assert(b5->bin[1] == 0xAB);
	assert(b5->bin[2] == 0x82);
	assert(b5->bin[3] == 0x0C);
	// big_int_print(b5);
	big_int_free(b5);


	printf(" str_to_big\n");
	struct big_int * b6 = str_to_big(6, "123456", 10); // 0x 1 E2 40
	assert(b6->sign == POSITIVE);
	assert(b6->len  == 3);
	assert(b6->cap  == 6);
	assert(b6->bin[0] == 0x40);
	assert(b6->bin[1] == 0xE2);
	assert(b6->bin[2] == 0x01);
	// big_int_print(b6);
	big_int_free(b6);

	struct big_int * b7 = str_to_big(21, "919476744083708551629", 10); // 31 d8 4d 9b 25 c6 33 a1 cd
	assert(b7->sign == POSITIVE);
	assert(b7->len  == 9);
	assert(b7->cap  == 21);
	assert(b7->bin[0] == 0xCD);
	assert(b7->bin[1] == 0xA1);
	assert(b7->bin[2] == 0x33);
	assert(b7->bin[3] == 0xC6);
	assert(b7->bin[4] == 0x25);
	assert(b7->bin[5] == 0x9B);
	assert(b7->bin[6] == 0x4D);
	assert(b7->bin[7] == 0xD8);
	assert(b7->bin[8] == 0x31);
	// big_int_print(b7);
	big_int_free(b7);

	assert(char_to_digit('b') == 11);
	assert(char_to_digit('d') == 13);


	printf(" long_to_big\n");
	struct big_int * b8 = long_to_big(123456);
	assert(b8->sign == POSITIVE);
	assert(b8->len  == 3);
	assert(b8->cap  == LONG_SIZE);
	assert(b8->bin[0] == 0x40);
	assert(b8->bin[1] == 0xE2);
	assert(b8->bin[2] == 0x1);
	big_int_free(b8);

	struct big_int * b81 = long_to_big(-123456);
	assert(b81->sign == NEGATIVE);
	assert(b81->len  == 3);
	assert(b81->cap  == LONG_SIZE);
	assert(b81->bin[0] == 0x40);
	assert(b81->bin[1] == 0xE2);
	assert(b81->bin[2] == 0x1);
	big_int_free(b81);


	printf(" realloc_big_int\n");
	struct big_int * b9 = str_to_big(1, "1", 10);
	assert(b9->sign == POSITIVE);
	assert(b9->len  == 1);
	assert(b9->cap  == 1);
	assert(b9->bin[0] == 1);
	realloc_big_int(&b9, 6);
	assert(b9->sign == POSITIVE);
	assert(b9->len  == 1);
	assert(b9->cap  == 6);
	assert(b9->bin[0] == 1);
	big_int_free(b9);


	printf(" (zeros)\n");
	unsigned char zero[] = {0};
	assert(digit_div_2(1, zero, 10) == 0);
	assert(zero[0] == 0);

	struct big_int * dzero = digit_to_big_int(1, zero, 10);
	assert(dzero->sign == POSITIVE);
	assert(dzero->len  == 1);
	assert(dzero->cap  == 1);
	assert(dzero->bin[0] == 0);
	big_int_free(dzero);

	struct big_int * szero = str_to_big(1, "0", 10);
	assert(szero->sign == POSITIVE);
	assert(szero->len  == 1);
	assert(szero->cap  == 1);
	assert(szero->bin[0] == 0);
	big_int_free(szero);

	struct big_int * lzero = long_to_big(0); // Same big_int zero from `str_to_big` or `long_to_big`
	assert(lzero->sign == POSITIVE);
	assert(lzero->len  == 1);
	assert(lzero->cap  == LONG_SIZE);
	assert(lzero->bin[0] == 0);
	big_int_free(lzero);


	printf(" big_int_neg\n");
	struct big_int * neg = long_to_big(34);
	big_int_neg(neg);
	assert(neg->sign == NEGATIVE);
	assert(neg->len  == 1);
	assert(neg->cap  == LONG_SIZE);
	assert(neg->bin[0] == 34);
	big_int_neg(neg);
	assert(neg->sign == POSITIVE);
	assert(neg->len  == 1);
	assert(neg->cap  == LONG_SIZE);
	assert(neg->bin[0] == 34);
	big_int_free(neg);


	printf(" big_int_add\n"); 
	struct big_int * ab1 = long_to_big(230);
	struct big_int * ab2 = long_to_big(42);
	assert(big_int_add(ab1, ab2) == ab1); // 272 = 16 + (1 * 256)
	assert(ab1->sign == POSITIVE);
	assert(ab1->len  == 2);
	assert(ab1->cap  = LONG_SIZE);
	assert(ab1->bin[0] == 16);
	assert(ab1->bin[1] == 1);
	big_int_free(ab1);
	big_int_free(ab2);

	struct big_int *ab3 = long_to_big(0);
	struct big_int *ab4 = long_to_big(13);
	assert(big_int_add(ab3, ab4) == ab3);
	assert(ab3->sign == POSITIVE);
	assert(ab3->len  == 1);
	assert(ab3->cap  = LONG_SIZE);
	assert(ab3->bin[0] == 13);
	big_int_free(ab3);
	big_int_free(ab4);

	// big_int_print(&BIG_ONE);
	// big_int_print(&BIG_ZERO);

	printf(" big_int_cmp\n");
	struct big_int * c1 = long_to_big(2456712);
	struct big_int * c2 = long_to_big(34234);
	assert(big_int_cmp(c1, c2) > 0);
	assert(big_int_cmp(c2, c1) < 0);
	assert(big_int_cmp(c1, c1) == 0);
	assert(big_int_cmp(c2, c2) == 0);
	big_int_free(c1);
	big_int_free(c2);

	struct big_int * c3 = long_to_big(1);
	struct big_int * c4 = long_to_big(-1);
	struct big_int * c5 = long_to_big(-2);
	assert(big_int_cmp(c3, c4) > 0);
	assert(big_int_cmp(c4, c5) > 0);
	assert(big_int_cmp(c5, c3) < 0);
	assert(big_int_cmp(c5, c5) == 0);
	big_int_free(c3);
	big_int_free(c4);
	big_int_free(c5);


	printf(" big_int_swap\n");
	struct big_int * swap1 = long_to_big(12);
	struct big_int * swap2 = long_to_big(34);
	big_int_swap(swap1, swap2);
	assert(swap1->bin[0] == 34);
	assert(swap2->bin[0] == 12);
	big_int_free(swap1);
	big_int_free(swap2);


	printf(" big_int_sub\n");
	struct big_int * s1 = long_to_big(279);
	struct big_int * s2 = long_to_big(31);
	assert(big_int_sub(s1, s2) == s1);
	assert(s1->len == 1);
	assert(s1->bin[0] == (279 - 31));
	big_int_free(s1);
	big_int_free(s2);

	struct big_int * s3 = long_to_big(348867916243);
	struct big_int * s4 = long_to_big(348867916242);
	assert(big_int_sub(s3, s4) == s3);
	// big_int_print(s3);
	assert(s3->len == 1);
	assert(s3->bin[0] == 1);
	assert(big_int_cmp(s3, &BIG_ONE) == 0);
	big_int_free(s3);
	big_int_free(s4);

	struct big_int * s5 = long_to_big(279);
	assert(s5->len == 2);
	assert(s5->bin[0] == 23);
	assert(s5->bin[1] == 1);
	sub_big(s5, &BIG_ONE);
	assert(s5->len == 2);
	assert(s5->bin[0] == 22);
	assert(s5->bin[1] == 1);
	big_int_free(s5);


	printf(" (add signed integer)\n");
	struct big_int * i1  = long_to_big(1);
	struct big_int * i2  = long_to_big(-1);
	struct big_int * r12 = big_int_add(i1, i2);
	assert(r12 == i1);
	assert(big_int_cmp(r12, &BIG_ZERO) == 0);
	big_int_free(i1);
	big_int_free(i2);

	i1  = long_to_big(1);
	i2  = long_to_big(-1);
	r12 = big_int_add(i2, i1);
	assert(r12 == i2);
	assert(big_int_cmp(r12, &BIG_ZERO) == 0);
	big_int_free(i1);
	big_int_free(i2);

	i1  = long_to_big(-1);
	i2  = long_to_big(-2);
	r12 = big_int_add(i2, i1);
	assert(r12 == i2);
	assert(r12->sign == NEGATIVE);
	assert(r12->len  == 1);
	assert(r12->bin[0] == 3);
	big_int_free(i1);
	big_int_free(i2);


	printf(" (sub signed integer)\n");
	i1  = long_to_big(1);
	i2  = long_to_big(-1);
	r12 = big_int_sub(i1, i2);
	assert(r12 == i1);
	assert(r12->sign == POSITIVE);
	assert(r12->len  == 1);
	assert(r12->bin[0] == 2);
	big_int_free(i1);
	big_int_free(i2);

	i1  = long_to_big(-1);
	i2  = long_to_big(1);
	r12 = big_int_sub(i1, i2);
	assert(r12 == i1);
	assert(r12->sign == NEGATIVE);
	assert(r12->len  == 1);
	assert(r12->bin[0] == 2);
	// big_int_print(r12);
	big_int_free(i1);
	big_int_free(i2);

	i1  = long_to_big(-1);
	i2  = long_to_big(-2);
	r12 = big_int_sub(i1, i2);
	assert(r12 == i1);
	assert(r12->sign == POSITIVE);
	assert(r12->len  == 1);
	assert(r12->bin[0] == 1);
	// big_int_print(r12);
	big_int_free(i1);
	big_int_free(i2);


	printf(" big_int_mul\n");
	struct big_int * m1 = long_to_big(-5);
	struct big_int * m2 = long_to_big(223776);
	struct big_int * rm12 = long_to_big(-5 * 223776);
	assert(big_int_mul(m1, m2) == m1);
	assert(big_int_cmp(m1, rm12) == 0);
	big_int_free(m1);
	big_int_free(m2);
	big_int_free(rm12);

	m1 = long_to_big(-51786950902);
	m2 = long_to_big(2232776);
	rm12 = long_to_big((long) -51786950902 * (long) 2232776);
	assert(big_int_mul(m1, m2) == m1);
	assert(big_int_cmp(m1, rm12) == 0);
	big_int_free(m1);
	big_int_free(m2);
	big_int_free(rm12);

	m1 = long_to_big(-517);
	m2 = long_to_big(2232);
	rm12 = long_to_big((long) -517 * (long) 2232);
	assert(big_int_mul(m1, m2) == m1);
	assert(big_int_cmp(m1, rm12) == 0);
	big_int_free(m1);
	big_int_free(m2);
	big_int_free(rm12);


	printf("done\n\n");
	#endif
}

