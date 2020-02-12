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

	log_set_quiet(1);
	#ifdef LOG_LEVEL
	log_set_fp(stdout);
	log_set_level(LOG_LEVEL);
	static const char *level_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
	printf("\n\trun all tests  ** log level %s **\n\n", level_names[LOG_LEVEL]);
	
	#else
	printf("\n\trun all tests  ** NO log **\n\n");
	#endif // LOG_LEVEL


	// test_lexer();
	// test_parser();
	// test_stack();
	// test_shunting_yard();
	test_big_int();
	// test_number();

	#endif // NDEBUG

	return 0;
}