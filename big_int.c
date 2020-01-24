#include "big_int.h"

#define BASE 256
#define LONG_SIZE 16

struct big_int {
	unsigned char * bin; // array of byte (like little endian)
	enum {
		POSITIVE,
		NEGATIVE,
	} sign;
	int len; // number of used char int `bin`
	int cap; // real capacity of the `bin`
};



static struct big_int * malloc_big_int(int cap) {

	struct big_int * big = malloc(sizeof(struct big_int) + (sizeof(char) * cap));
	CHECK_MALLOC(big, "malloc_big_int (big_int.c)");

	big->bin  = (unsigned char *) &big[1];
	big->sign = POSITIVE;
	big->len  = 0;
	big->cap  = cap;
	return big;
}


static void realloc_big_int(struct big_int ** big, int cap) {
	assert((*big)->cap < cap);
	// printf("bit int: realloc\n");

	int sign = (*big)->sign;
	int len  = (*big)->len;

	struct big_int * new = realloc(*big, sizeof(struct big_int) + (sizeof(char) * cap));
	CHECK_MALLOC(new, "realloc_big_int (big_int.c)");

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
		rem = num[i] + rem;
		num[i] = rem / 2;
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



struct big_int * str_to_big(int len, const char * str, unsigned int base) {
	assert(len  > 0);
	assert(base > 1);
	assert(base <= 10); // for now

	unsigned char * digit = malloc(sizeof(unsigned char) * len);
	CHECK_MALLOC(digit, "str_to_big (big_int.c)");

	for (int i = 0; i < len; i++) {
		digit[i] = str[len - i - 1] - '0'; // char to digit
	}

	struct big_int * big = digit_to_big_int(len, digit, base);
	free(digit);
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
		big->bin[i] = num % BASE;
		num /= BASE;
		i++;
	} while (num);
	big->len = i;

	return big;
}


// // assume b1 is long enough to store the result
// static void add_big(struct big_int * b1, const struct big_int * b2) {

// 	const struct big_int * i1; // shorter
// 	const struct big_int * i2; // longer
// 	if (b1->size < b2->size) {
// 		i1 = b1;
// 		i2 = b2;
// 	} else {
// 		i1 = b2;
// 		i2 = b1;
// 	}

// 	unsigned char res = 0;
// 	int i = 0;

// 	while (i < i1->size) {

// 		res = i1->array[i] + i2->array[i] + res;
// 		b1->array[i] = res % BASE; // result in b1
// 		res /= BASE;
// 		i++;
// 	}
// 	while (i < i2->size) {

// 		res = i2->array[i] + res;
// 		b1->array[i] = res % BASE;
// 		res /= BASE;
// 		i++;
// 	}
// 	assert((0 <= res) && (res < BASE));
// 	if (res > 0) {
// 		b1->array[i] = res;
// 		i++;
// 	}

// 	b1->size = i;
// }

// struct big_int * add_big_int(struct big_int * b1, const struct big_int * b2) {

// 	struct big_int * result = b1;
// 	int max_size = (b1->size < b2->size ? b2->size : b1->size);

// 	// the result of a int sum is a (max_size + 1) long integer
// 	if (result->cap <= max_size + 1) {
// 		realloc_big_int(&result, max_size * 2);
// 	}
// 	assert(result->cap > max_size + 1);

// 	add_big(result, b2);
// 	return result;
// }

// struct big_int * sub_big_int(struct big_int * b1, const struct big_int * b2) {
// 	return NULL;
// }

// struct big_int * mult_big_int(struct big_int * b1, const struct big_int * b2) {

// 	struct big_int * result = b1;
// 	int max_size = b1->size + b2->size;

// 	// the result should be store in a (b1->size + b2->size) long integer
// 	if (result->cap <= max_size) {
// 		realloc_big_int(&realloc, max_size * 2);
// 	}
// 	assert(result->cap > max_size);

// 	// expo

// 	return NULL;
// }



// int big_int_is_null(const struct big_int * big) {
// 	return 0;
// }

// void neg_big_int(struct big_int * big) {
// 	big->sign = (big->sign == POSITIVE ? NEGATIVE : POSITIVE);
// }

void print_big_int(const struct big_int * const big) {
	if (big->sign == NEGATIVE) {
		printf("-");
	}
	printf("0x");

	for (int i = big->len - 1; i >= 0; i--) {
		printf("%hhx", big->bin[i]);
	}
}

void free_big_int(struct big_int * big) {
	big->len = 0;
	big->cap = 0;
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


	printf(" malloc_big_int\n");
	struct big_int * b1 = malloc_big_int(3);
	assert(b1->sign == POSITIVE);
	assert(b1->len  == 0);
	assert(b1->cap  == 3);
	// print_big_int(b1);
	free_big_int(b1);


	printf(" digit_to_big_int\n");
	unsigned char num4[] = {2, 4}; // 42
	struct big_int * b4  = digit_to_big_int(2, num4, 10);
	assert(b4->sign == POSITIVE);
	assert(b4->len  == 1);
	assert(b4->cap  == 2);
	assert(b4->bin[0] == 42);
	// print_big_int(b4);
	free_big_int(b4);

	unsigned char num5[] = {9, 2, 11, 10, 2, 8, 12}; // 209 890 089 -> 0x C 82 AB 29
	struct big_int * b5  = digit_to_big_int(7, num5, 16);
	assert(b5->sign == POSITIVE);
	assert(b5->len  == 4);
	assert(b5->cap  == 7);
	assert(b5->bin[0] == 0x29);
	assert(b5->bin[1] == 0xAB);
	assert(b5->bin[2] == 0x82);
	assert(b5->bin[3] == 0x0C);
	// print_big_int(b5);
	free_big_int(b5);


	printf(" str_to_big\n");
	struct big_int * b6 = str_to_big(6, "123456", 10); // 0x 1 E2 40
	assert(b6->sign == POSITIVE);
	assert(b6->len  == 3);
	assert(b6->cap  == 6);
	assert(b6->bin[0] == 0x40);
	assert(b6->bin[1] == 0xE2);
	assert(b6->bin[2] == 0x01);
	// print_big_int(b6);
	free_big_int(b6);

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
	// print_big_int(b7);
	free_big_int(b7);


	printf(" long_to_big\n");
	struct big_int * b8 = long_to_big(123456);
	assert(b8->sign == POSITIVE);
	assert(b8->len  == 3);
	assert(b8->cap  == LONG_SIZE);
	assert(b8->bin[0] == 0x40);
	assert(b8->bin[1] == 0xE2);
	assert(b8->bin[2] == 0x1);
	free_big_int(b8);

	struct big_int * b81 = long_to_big(-123456);
	assert(b81->sign == NEGATIVE);
	assert(b81->len  == 3);
	assert(b81->cap  == LONG_SIZE);
	assert(b81->bin[0] == 0x40);
	assert(b81->bin[1] == 0xE2);
	assert(b81->bin[2] == 0x1);
	free_big_int(b81);


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
	free_big_int(b9);


	printf(" (handle zeros)\n");
	unsigned char zero[] = {0};
	assert(digit_div_2(1, zero, 10) == 0);
	assert(zero[0] == 0);

	struct big_int * dzero = digit_to_big_int(1, zero, 10);
	assert(dzero->sign == POSITIVE);
	assert(dzero->len  == 1);
	assert(dzero->cap  == 1);
	assert(dzero->bin[0] == 0);
	free_big_int(dzero);

	struct big_int * szero = str_to_big(1, "0", 10);
	assert(szero->sign == POSITIVE);
	assert(szero->len  == 1);
	assert(szero->cap  == 1);
	assert(szero->bin[0] == 0);
	free_big_int(szero);

	struct big_int * lzero = long_to_big(0); // Same zero big_int from `str_to_big` to `long_to_big`
	assert(lzero->sign == POSITIVE);
	assert(lzero->len  == 1);
	assert(lzero->cap  == LONG_SIZE);
	assert(lzero->bin[0] == 0);
	free_big_int(lzero);




	// printf(" neg_big_int\n");
	// struct big_int * b4 = long_to_big(34);
	// neg_big_int(b4);
	// assert(b4->sign == NEGATIVE);
	// assert(b4->size == 2);
	// assert(b4->array[0] == 4);
	// assert(b4->array[1] == 3);
	// // printf(" ");
	// // print_big_int(b4);
	// // printf("\n");
	// free_big_int(b4);


	// printf(" add_big_int\n");
	// struct big_int * b5 = long_to_big(31);
	// struct big_int * b6 = long_to_big(42);

	// assert(b5->size == 2);
	// // print_big_int(b5);
	// assert(b6->size == 2);
	// // print_big_int(b6);
	// add_big(b5, b6);
	// // print_big_int(b5);
	// assert(b5->array[0] == 3); // 73
	// assert(b5->array[1] == 7);
	// free_big_int(b5);
	// free_big_int(b6);

	// struct big_int * b7 = long_to_big(51);
	// struct big_int * b8 = long_to_big(92);
	// assert(b7->size == 2);
	// assert(b8->size == 2);
	// add_big(b7, b8);
	// assert(b7->size == 3); // 143
	// assert(b7->array[0] == 3);
	// assert(b7->array[1] == 4);
	// assert(b7->array[2] == 1);
	// free_big_int(b7);
	// free_big_int(b8);

	// struct big_int * b9 = long_to_big(599);
	// struct big_int * b10 = long_to_big(1399);
	// add_big(b9, b10);
	// assert(b9->size == 4); // 1998
	// assert(b9->array[0] == 8);
	// assert(b9->array[1] == 9);
	// assert(b9->array[2] == 9);
	// assert(b9->array[3] == 1);
	// free_big_int(b9);
	// free_big_int(b10);

	// struct big_int * b11 = long_to_big(999);
	// struct big_int * b12 = long_to_big(9999);
	// add_big(b11, b12);
	// assert(b11->size == 5); // 10998
	// assert(b11->array[0] == 8);	
	// assert(b11->array[1] == 9);
	// assert(b11->array[2] == 9);
	// assert(b11->array[3] == 0);
	// assert(b11->array[4] == 1);
	// free_big_int(b11);
	// free_big_int(b12);



	printf("done\n");
	#endif
}

