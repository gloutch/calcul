#include "shunting_yard.h"

#define TOKEN_STACK(size) stack_malloc(sizeof(struct parser_token), size, (stack_copy_elem) copy_parser_token);


// < 0 means left associative
//  0  means 
// 0 < means right associative
static int assoc(const struct parser_token * const token) {
	switch (token->type) {

		case MINUS:
			return -1;
		case PLUS:
		case ASTERISK:
		case LPARENT:
		case RPARENT:
			return 0;
		case UNARY_PLUS:
		case UNARY_MINUS:
			return 1;
		default:
			assert(0);
			return 0;
	}
}

static int preced(const struct parser_token * const token) {
	switch (token->type) {

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
static void shunting_yard_wye(const struct parser_token * input, struct stack * operator, struct stack * output) {

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
			while (((struct parser_token *) stack_peek(operator))->type != LPARENT) {
				struct parser_token tmp;
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
				struct parser_token * top = stack_peek(operator);
				if ( !((preced(top) > preced(input)) || ((preced(top) == preced(input)) && (assoc(top) < 0))) ) {
					break;
				}
				struct parser_token tmp;
				stack_pop(operator, &tmp);
				stack_push(output, &tmp);
			}
			stack_push(operator, input);
			return;
		}
		case RPARENT: { 	// )
			struct parser_token tmp;
			stack_pop(operator, &tmp);
			while (tmp.type != LPARENT) {
				stack_push(output, &tmp);
				stack_pop(operator, &tmp);
			} // discard LPARENT
			if ((stack_empty(operator) || (((struct parser_token *) stack_peek(operator))->type != FUNC_NAME))) {
				return;
			}
			stack_pop(operator, &tmp);
			stack_push(output, &tmp);
			return;
		}
		case ERROR:
			return;
	}
	assert(0);
	return;
}

struct stack * const shunting_yard(int n, const struct parser_token * token) {
	log_trace("shunting_yard");

	// oversized stacks
	struct stack * output   = TOKEN_STACK(n);
	struct stack * operator = TOKEN_STACK(n);

	for (int i = 0; i < n; i++) {
		shunting_yard_wye(&token[i], operator, output);
	}

	while (!stack_empty(operator)) {
		struct parser_token tmp;
		stack_pop(operator, &tmp);
		stack_push(output, &tmp);
	}
	assert(stack_empty(operator));

	// clear
	stack_free(operator);
	stack_reverse(output);
	return output;
}


static void print_rpn_stack_token(const struct parser_token * const token) {
	print_parser_token(token);
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
