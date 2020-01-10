#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "stack.h"

/*

The utimate result of the parser is a `struct parser_result`

If everything went well,
`type == CORRECT`
`RPN_stack` is a stack of token with the expression in Reverse Polish Notation (easier to evaluate)

Otherwise,
`type` describes the error
`RPN_stack` is NULL or it contains one token pointing to the error

Note: then the stack had to be free

*/



// enum result_type {
// 	CORRECT,		// everything turned great :D
// 	ERR_NULL,		// NULL string :(
// 	ERR_TOKEN,		// unknown token
// 	ERR_PARENT		// missing parenthesis
// };

// struct parser_result {
// 	enum result_type type;
// 	struct stack * RPN_stack;
// };


// struct parser_result parser(char const * string);

// void print_RPN_stack(struct stack const * const RPN);

// void print_token(struct token const * const tok);

// void copy_token(struct token const * const src, struct token * const dst);

// void test_parser();


#endif // PARSER_H