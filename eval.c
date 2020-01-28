#include "eval.h"


typedef void (bin_op)(struct number * n1, struct number * n2);

static struct number binary_op(struct stack * operands, bin_op operation) {

	struct number n2;
	stack_pop(operands, &n2);
	struct number n1;
	stack_pop(operands, &n1);
	operation(&n1, &n2);
	number_free(n2);
	return n1;
}


static void eval_token(const struct parser_token exp_token, struct stack * operands) {

	switch (exp_token.type) {

		case NUM_OPERAND: {
			struct number num = str_to_number(exp_token.len, exp_token.str, 10);
			stack_push(operands, &num);
			return;
		}

		case VAR_OPERAND:
		case FUNC_NAME:
			log_fatal("Sorry, '%.*s' no variable management yet ", exp_token.len, exp_token.str);
			exit(0);
			return;

		case PLUS: {
			struct number res = binary_op(operands, number_add);
			stack_push(operands, &res);
			return;
		}
		case MINUS: {
			struct number res = binary_op(operands, number_sub);
			stack_push(operands, &res);
			return;
		}
		case ASTERISK: {
			struct number res = binary_op(operands, number_mul);
			stack_push(operands, &res);
			return;
		}

		case UNARY_PLUS:
			return;
		case UNARY_MINUS: {
			struct number * num = stack_peek(operands);
			number_neg(num);
			return;
		}

		default:
			assert(0);
	}
	assert(0);
}


struct number eval(struct parser_result exp) {
	log_trace("eval");

	struct stack * stack_exp = shunting_yard(exp.size, exp.tarray);
	// print_rpn_stack(stack_exp);
	int size = stack_size(stack_exp);
	struct stack * operands = stack_malloc(sizeof(struct number), size, (stack_copy_elem) number_copy);

	while (!stack_empty(stack_exp)) {
		struct parser_token exp_token;
		stack_pop(stack_exp, &exp_token);
		eval_token(exp_token, operands);
	}
	assert(stack_empty(stack_exp));
	assert(stack_size(operands) == 1);

	struct number result;
	stack_pop(operands, &result);
	stack_free(stack_exp);
	stack_free(operands);
	return result;
}

