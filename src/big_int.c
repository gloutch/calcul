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

static struct big_int * extend_capacity(struct big_int * big, int cap) {
	if (big->cap >= cap) {
		return big;
	}

	struct big_int save = *big;
	int new_cap = ((2 * big->cap) > cap ? (2 * big->cap) : cap); // max(2 * big->cap, cap)

	struct big_int * new = realloc(big, sizeof(struct big_int) + new_cap);
	CHECK_MALLOC(new, "extend_capacity");

	new->bin  = (unsigned char *) &new[1];
	new->sign = save.sign;
	new->len  = save.len;
	new->cap  = new_cap;

	log_debug("big realloc @%p->bin %p[%d] ==> @%p->bin %p[%d]", big, save.bin, save.cap, new, new->bin, new->cap);
	assert(new->cap >= cap);
	return new;
}

// num is a `len` long array of digit (base `base`)
// return the new size of the number represented in `num`
static int digit_div_2(int len, unsigned char * num, unsigned int base) {
	assert(base <= BASE);
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

static void big_swap(struct big_int * b1, struct big_int * b2) {
	assert(b1->len <= b2->cap);
	assert(b2->len <= b1->cap);

	int mlen = (b1->len > b2->len ? b1->len : b2->len); // max(b1->len, b2->len)
	assert(mlen <= b1->cap);
	assert(mlen <= b2->cap);

	struct big_int b1save = *b1;

	// swap ->bin
	for (int i = 0; i < mlen; i++) {
		const unsigned char tmp = b1->bin[i];
		b1->bin[i] = b2->bin[i];
		b2->bin[i] = tmp;
	}

	b1->len  = b2->len;
	b1->cap  = b2->cap;
	b1->sign = b2->sign;

	b2->len  = b1save.len;
	b2->cap  = b1save.cap;
	b2->sign = b1save.sign;
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

	log_info("long %ld to big @%p", l, big);
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
	CHECK_MALLOC(digit, "str_to_big");

	for (int i = 0; i < len; i++) {
		digit[i] = char_to_digit(str[len - i - 1]);
	}

	struct big_int * big = digit_to_big_int(len, digit, base);
	free(digit);
	LOG_FREE(digit);
	return big;
}


int big_int_length(const struct big_int * b) {
	return b->len;
}

void big_int_neg(struct big_int * b) {
	log_info("big -%p", b);
	if ((b->len == 1) && (b->bin[0] == 0)) {
		assert(b->sign == POSITIVE);
		return; // no neg on zero;
	}
	b->sign = (b->sign == POSITIVE ? NEGATIVE : POSITIVE);
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



static void add_big(struct big_int * b1, const struct big_int * b2) {
	log_info("big @%p + @%p = @%p", b1, b2, b1);
	assert(b1->cap >= (b1->len > b2->len ? b1->len : b2->len) + 1);

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
	// check sign
	if ((b1->sign == POSITIVE) && (b2->sign == NEGATIVE)) {
		b2->sign = POSITIVE;
		return big_int_sub(b1, b2);
	}
	if ((b1->sign == NEGATIVE) && (b2->sign == POSITIVE)) {
		b1->sign = POSITIVE;
		b1 = big_int_sub(b1, b2);
		big_int_neg(b1);
		return b1;
	}
	assert(b1->sign == b2->sign);

	b1 = extend_capacity(b1, (b1->len < b2->len ? b2->len : b1->len) + 1);
	add_big(b1, b2);
	return b1;
}


static void sub_big(struct big_int * b1, const struct big_int * b2) {
	log_info("big @%p - #%p = @%p", b1, b2, b1);
	assert(big_int_cmp_bin(b1, b2) > 0);
	assert(b1->cap >= b1->len);

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
	// check sign
	if (b2->sign == NEGATIVE) {
		b2->sign = POSITIVE;
		return big_int_add(b1, b2);
	}
	if (b1->sign == NEGATIVE) {
		b1->sign = POSITIVE;
		assert(b2->sign == POSITIVE);
		b1 = big_int_add(b1, b2); 
		b1->sign = NEGATIVE;
		return b1;
	}
	assert(b1->sign == POSITIVE);
	assert(b2->sign == POSITIVE);

	// operation
	int cmp = big_int_cmp_bin(b1, b2);

	if (cmp == 0) { // transform b1 to zero
		b1->sign = POSITIVE;
		b1->len = 1;
		b1->bin[0] = 0;
		return b1;
	}
	if (cmp > 0) { // b1 > b2
		sub_big(b1, b2);
		return b1;
	}

	// b2 > b1
	b1 = extend_capacity(b1, b2->cap);
	big_swap(b1, b2);
	sub_big(b1, b2);
	b1->sign = NEGATIVE;
	return b1;
}


static void mul_big(struct big_int * b1, const struct big_int * b2) {
	log_info("big @%p * @%p = @%p", b1, b2, b1);
	// printf("b1->len %d cap %d, b2->len %d cap %d\n", b1->len, b1->cap, b2->len, b2->cap);
	assert(b1 != b2);
	assert(b1->len <= b2->len); 		// b2 is the longer
	assert(b1->cap >= 2 * b2->len); 	// b1 is long enough

	memcpy(&(b1->bin[b2->len]), b1->bin, b1->len);
	memset(&(b1->bin[b2->len + b1->len]), 0, (b2->len - b1->len));

	int w = 0;
	long rem = 0;
	int len = b2->len;

	for (int i = 0; i < len; i++) {
		for (int j = i; j >= 0; j--) {
			// printf("%d, %d\n", j, len + i - j);
			rem += b2->bin[j] * b1->bin[len + i - j];
		}
		b1->bin[w++] = rem % BASE;
		rem /= BASE;
	}
	log_debug("big_int_sqr monitor long rem = %ld >= 0", rem);
	assert(rem >= 0);
	for (int i = 1; i < b1->len; i++) {
		for (int j = i; j < len; j++) {
			// printf("%d, %d\n", j, 2 * len - j + i - 1);
			rem += b2->bin[j] * b1->bin[2 * len - j + i - 1];
		}
		b1->bin[w++] = rem % BASE;
		rem /= BASE;
	}
	assert((0 <= rem) && (rem < BASE));
	if (rem > 0) {
		b1->bin[w++] = rem;
	}
	b1->len = w;
}

struct big_int * big_int_mul(struct big_int * b1, struct big_int * b2) {
	assert(b1 != b2);

	// check zero
	if (((b1->len == 1) && (b1->bin[0] == 0)) || // b1 == 0 or b2 == 0
		((b2->len == 1) && (b2->bin[0] == 0))) {
		b1->sign   = POSITIVE;
		b1->len    = 1;
		b1->bin[0] = 0;
		return b1;
	}

	// check capacity
	if (b1->len > b2->len) { // b1 is longer
		printf("b1 len %d cap %d, b2 len %d cap %d\n", b1->len, b1->cap, b2->len, b2->cap);
		if (b2->cap >= b1->len) { // b1 fit in b2
			big_swap(b1, b2);
			b1 = extend_capacity(b1, 2 * b2->len);
		}
		else {
			// I don't want to extend b2 just for the swap
			// I also need to extend b1 after. So I extend once b1 to fit b1 and b2
			b1 = extend_capacity(b1, b1->len * 3);
			memcpy(&(b1->bin[2 * b1->len]), b1->bin, b1->len); 	// copy b1 further
			memcpy(b1->bin, b2->bin, b2->len);					// copy b2 on b1

			b2->bin = &(b1->bin[2 * b1->len]);					// point b2 to b1->bin

			int b1_len = b1->len;								// swap length
			b1->len = b2->len;
			b2->len = b1_len;
		}
	} else {
		b1 = extend_capacity(b1, 2 * b2->len);
	}
	assert(b1->len <= b2->len);
	assert(b1->cap >= 2 * b2->len);
	mul_big(b1, b2);
	
	// sign
	b1->sign = (b1->sign == b2->sign ? POSITIVE : NEGATIVE);
	return b1;
}



struct big_int * big_int_sqr(struct big_int * b) {
	// set sign
	b->sign = POSITIVE;

	// check capacity
	int len = b->len;
	b = extend_capacity(b, 2 * len);

	// shitf the number on the second half of bin
	memcpy(&(b->bin[len]), b->bin, b->len);
	int w = 0;   			// where to write the result
	int l = len; 			// lower cursor on the tmp num
	int u = l + len - 1; 	// upper cursor
	long rem = 0; 			// hoping a long is enough

	for (int i = 0; i < len; i++) {
		for (int j = 0; j <= i; j++) {
			// printf("%d, %d\n", l + j, l + i - j); // debug
			rem += b->bin[l + j] * b->bin[l + i - j];
		}
		b->bin[w++] = rem % BASE;
		rem = rem / BASE;
	}
	log_debug("big_int_sqr monitor long rem = %ld >= 0", rem);
	assert(rem >= 0);
	for (int i = len - 2; i >= 0; i--) {
		for (int j = 0; j <= i; j++) {
			// printf("%d, %d\n", u - i + j, u - j); // debug
			rem += b->bin[u - i + j] * b->bin[u - j];
		}
		b->bin[w++] = rem % BASE;
		rem = rem / BASE;
	}
	assert((0 <= rem) && (rem < BASE));
	if (rem > 0) {
		b->bin[w++] = rem;
	}
	b->len = w;
	return b;
}

struct big_int * big_int_pow(struct big_int * b, long expo) {
	log_info("big @%p ^ %ld", b, expo);
	assert(expo >= 0);

	// check special value of expo
	if (expo == 0) {
		b->len = 1;
		b->bin[0] = 1;
		b->sign = POSITIVE;
		return b;
	}
	if (expo == 1) {
		return b;
	}
	if (expo == 2) {
		return big_int_sqr(b);
	}

	// check sign
	b->sign = (expo % 2 ? NEGATIVE : POSITIVE);

	// expoentiation by squaring
	struct big_int * prod = malloc_big_int(b->cap * 2);
	prod->bin[0] = 1;

	while (expo != 1) {
		log_debug("big expo, b @%p, prod @%p, expo %ld", b, prod, expo);

		if (expo % 2) { // odd
			assert(prod->len <= b->len);
			prod = extend_capacity(prod, 2 * b->len);
			mul_big(prod, b);
			expo -= 1;
			b = big_int_sqr(b);
			expo /= 2;
		} 
		else { // even
			b = big_int_sqr(b);
			expo /= 2;
		}
	}
	assert(expo == 1);
	
	assert(prod->len <= b->len);
	prod = extend_capacity(prod, 2 * b->len);
	mul_big(prod, b);
	
	big_int_free(b);
	log_info("big expo in @%p", prod);
	return prod;
}



// return LONG_MIN if big_int can't fit in an long
long big_to_long(const struct big_int * big) {
	log_debug("trying to convert big %p [%d bytes] in long", big, big->len);

	if (big->len > sizeof(unsigned long)) {
		return LONG_MIN;
	}
	// fit in a long
	unsigned long ures = 0;
	memcpy((char *) &ures, (const char *) big->bin, big->len);
	if (ures == 0) { // LONG_MIN is 0 in a unsinged long
		return LONG_MIN;
	}
	if (ures > (unsigned long) LONG_MAX) {
		return LONG_MIN;
	}

	long res = (big->sign == POSITIVE ? (long) ures : -(long) ures);
	log_info("big %p == (long %ld)", big, res);
	return res;
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


	printf(" extend_capacity\n");
	struct big_int * b9 = str_to_big(1, "1", 10);
	assert(b9->sign == POSITIVE);
	assert(b9->len  == 1);
	assert(b9->cap  == 1);
	assert(b9->bin[0] == 1);
	b9 = extend_capacity(b9, 6);
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
	// big_int_print(m1);
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

	m1 = str_to_big(17, "10000000000000000", 16);
	m2 = long_to_big(2);
	struct big_int * m12 = big_int_mul(m1, m2);
	// big_int_print(m12);
	big_int_free(m2);
	big_int_free(m12);


	printf(" big_to_long\n");
	long l1 = (long) 1527261;
	struct big_int * p1 = long_to_big(l1);
	long cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	l1 = (long) 0x10000000000;
	p1 = long_to_big(l1);
	cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	l1 = (long) -1527261;
	p1 = long_to_big(l1);
	cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	l1 = (long) LONG_MAX;
	p1 = long_to_big(l1);
	cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	l1 = - (long) LONG_MAX;
	p1 = long_to_big(l1);
	cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	l1 = (long) LONG_MIN;
	p1 = long_to_big(l1);
	cvt1 = big_to_long(p1);
	assert(l1 == cvt1);
	big_int_free(p1);

	p1 = str_to_big(20, "18446744073709551615", 10); // doesn't fit in long
	cvt1 = big_to_long(p1);
	assert(cvt1 == LONG_MIN);
	big_int_free(p1);


	printf(" big_int_sqr\n");
	long s = 256;
	struct big_int * bsqr = big_int_sqr(long_to_big(s));
	long sqr = big_to_long(bsqr);
	assert(sqr == s * s);
	big_int_free(bsqr);

	s = 3333333;
	bsqr = big_int_sqr(long_to_big(s));
	sqr = big_to_long(bsqr);
	assert(sqr == s * s);
	big_int_free(bsqr);

	s = -1808780;
	bsqr = big_int_sqr(long_to_big(s));
	sqr = big_to_long(bsqr);
	assert(sqr == s * s);
	big_int_free(bsqr);


	printf(" big_int_pow\n");
	struct big_int * pow = long_to_big(3);
	long pow_res = big_to_long(big_int_pow(pow, 0));
	assert(pow_res == 1);
	big_int_free(pow);

	pow = long_to_big(3);
	pow_res = big_to_long(big_int_pow(pow, 1));
	assert(pow_res == 3);
	big_int_free(pow);

	pow = long_to_big(3);
	pow_res = big_to_long(big_int_pow(pow, 2));
	assert(pow_res == 9);
	big_int_free(pow);

	pow = long_to_big(3);
	pow = big_int_pow(pow, 3);
	pow_res = big_to_long(pow);
	assert(pow_res == 27);
	big_int_free(pow);

	pow = long_to_big(2);
	pow = big_int_pow(pow, 56);
	pow_res = big_to_long(pow);
	assert(pow_res == (long) 72057594037927936);
	big_int_free(pow);

	pow = long_to_big(3);
	pow = big_int_pow(pow, 300);
	pow_res = big_to_long(pow);
	assert(pow_res == LONG_MIN); // pow_res doesn't fit in long
	big_int_print(pow);
	printf(" = 3 ^ 300\n");
	big_int_free(pow);

	printf("done\n\n");
	#endif
}

