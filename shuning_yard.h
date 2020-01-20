#ifndef SHUNING_YARD_H
#define SHUNING_YARD_H

#include <assert.h>
#include "config.h"
#include "parser.h"
#include "stack.h"


/*

	`shuning_yard` convert an array of token to a stack in Reverse Polish Notation

*/


struct stack * const shunting_yard(int n, const struct parser_token * token);

void print_rpn_stack(struct stack const * const rpn_stack);


void test_shuning_yard();


#endif // SHUNING_YARD_H