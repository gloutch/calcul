#ifndef SHUNTING_YARD_H
#define SHUNTING_YARD_H

#include <assert.h>
#include "config.h"
#include "log.h"
#include "stack.h"
#include "token.h"


/*
	`shunting_yard` convert an array of token to a stack in Reverse Polish Notation
*/


struct stack * const shunting_yard(int n, const struct token * token);

void print_rpn_stack(struct stack const * const rpn_stack);


void test_shunting_yard();


#endif // SHUNTING_YARD_H