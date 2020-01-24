#ifndef SHUNTING_YARD_H
#define SHUNTING_YARD_H

#include <assert.h>
#include "config.h"
#include "parser.h"
#include "stack.h"


/*

	`shunting_yard` convert an array of token to a stack in Reverse Polish Notation

*/


struct stack * const shunting_yard(int n, const struct parser_token * token);

void print_rpn_stack(struct stack const * const rpn_stack);


void test_shunting_yard();


#endif // SHUNTING_YARD_H