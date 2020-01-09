#include <stdio.h>

#include "stack.h"
#include "parser.h"

/*
	This file should be compile as an executable to run all test
	Compile without -DNDEBUG to keep `assert`
*/


int main(int argc, char *argv[]) {

	#ifdef NDEBUG
	printf("COMPILE ERROR: should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#endif

	printf("\nrun all tests\n\n");

	printf("parser\n");
	test_parser();
	printf("stack\n");
	test_stack();

	return 0;
}