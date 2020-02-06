#include "shunting_yard.h"

#define TOKEN_STACK(size) stack_malloc(sizeof(struct token), size, (stack_copy_elem) token_copy);


// < 0 means left associative
//  0  means 
// 0 < means right associative
static int assoc(const struct token * const t) {
	switch (t->type) {

		case MINUS: // a - b - c = (a - b) - c
			return -1;
		case PLUS:
		case ASTERISK:
		case LPARENT:
		case RPARENT:
			return 0;
		case UNARY_PLUS:
		case UNARY_MINUS: // --a = -(-a)
			return 1;
		default:
			assert(0);
			return 0;
	}
}

static int preced(const struct token * const t) {
	switch (t->type) {

		case UNARY_PLUS:
		case UNARY_MINUS:
			return 14;
		case ASTERISK:
			return 13;
		case PLUS:
		case MINUS:
			return 12;
		case LPARENT:
		case RPARENT:
			return 1;
		default:
			assert(0);
			return 0;
	}
}


// let's assume the parenthesis are correct
static void shunting_yard_wye(const struct token * input, struct stack * operator, struct stack * output) {

	switch (input->type) {

		case NUM_OPERAND:	// operand
		case VAR_OPERAND: {
			stack_push(output, input);
			return;
		}
		case LPARENT:		// (
		case FUNC_NAME: {	// function
			stack_push(operator, input);
			return;
		}
		case ARG_SEP: {		// ,
			while (((struct token *) stack_peek(operator))->type != LPARENT) {
				struct token tmp;
				stack_pop(operator, &tmp);
				stack_push(output, &tmp);
			}
			return;
		}
		case PLUS:			// operator
		case MINUS:
		case ASTERISK:
		case UNARY_PLUS:
		case UNARY_MINUS: {
			while (!stack_empty(operator)) {
				struct token * top = stack_peek(operator);
				if ( !((preced(top) > preced(input)) || ((preced(top) == preced(input)) && (assoc(top) < 0))) ) {
					break;
				}
				struct token tmp;
				stack_pop(operator, &tmp);
				stack_push(output, &tmp);
			}
			stack_push(operator, input);
			return;
		}
		case RPARENT: { 	// )
			struct token tmp;
			stack_pop(operator, &tmp);
			while (tmp.type != LPARENT) {
				stack_push(output, &tmp);
				stack_pop(operator, &tmp);
			} // discard LPARENT
			if ((stack_empty(operator) || (((struct token *) stack_peek(operator))->type != FUNC_NAME))) {
				return;
			}
			stack_pop(operator, &tmp);
			stack_push(output, &tmp);
			return;
		}
		default:
			return;
	}
	assert(0);
	return;
}

struct stack * const shunting_yard(int n, const struct token * token) {
	log_debug("Shunting_yard on %d token", n);

	// oversized stacks
	struct stack * output   = TOKEN_STACK(n);
	struct stack * operator = TOKEN_STACK(n);

	for (int i = 0; i < n; i++) {
		shunting_yard_wye(&token[i], operator, output);
	}

	while (!stack_empty(operator)) {
		struct token tmp;
		stack_pop(operator, &tmp);
		stack_push(output, &tmp);
	}
	assert(stack_empty(operator));

	// clear
	stack_free(operator);
	stack_reverse(output);
	log_debug("Shunting_yard end, stack expression in %p", output);
	return output;
}


static void print_rpn_stack_token(const struct token * const t) {
	token_print(t);
	printf("\n");
}

void print_rpn_stack(struct stack const * const rpn_stack) {
	stack_print(rpn_stack, (stack_print_elem) print_rpn_stack_token);
}



/*
	TEST
*/


void test_shunting_yard() {
	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else
	printf("SHUNTING YARD:\n");
	// TODO
	printf("done\n\n");
	#endif
}
