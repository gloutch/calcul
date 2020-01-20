#include <stdio.h>

#include "lexer.h"
#include "stack.h"
#include "parser.h"
#include "shuning_yard.h"

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
	test_shuning_yard();
	#endif

	return 0;
}