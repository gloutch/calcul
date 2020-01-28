#include <stdio.h>

#include "big_int.h"
#include "lexer.h"
#include "stack.h"
#include "number.h"
#include "parser.h"
#include "shunting_yard.h"

/*
	This file should be compile as an executable to run all test
	Compile without -DNDEBUG to keep `assert`
*/


int main(int argc, char *argv[]) {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else

	printf("\nrun all tests\n\n");

	test_lexer();
	test_parser();
	test_stack();
	test_shunting_yard();
	test_big_int();
	test_number();

	#endif

	return 0;
}