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


static void eval_token(const struct token exp_token, struct stack * operands) {

	switch (exp_token.type) {

		case NUM_OPERAND: {
			struct number num = str_to_number(exp_token.len, exp_token.str);
			stack_push(operands, &num);
			return;
		}

		case VAR_OPERAND:
			log_error("Unknown variable '%.*s' (no variable yet)", exp_token.len, exp_token.str);
			error_set(UNMANAGED, NULL, exp_token.str, exp_token.len);
			return;
		case FUNC_NAME:
			log_error("Unknown function '%.*s' (no function yet) ", exp_token.len, exp_token.str);
			error_set(UNMANAGED, NULL, exp_token.str, exp_token.len);
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

		case UNARY_PLUS: // nothing to todo
			return;
		case UNARY_MINUS: {
			struct number * num = stack_peek(operands);
			number_neg(num);
			return;
		}

		default:
			return;
	}
}


struct number eval(const struct expr e) {

	struct stack * stack_exp = shunting_yard(e.len, e.list);
	// print_rpn_stack(stack_exp);
	int size = stack_size(stack_exp);
	struct stack * operands = stack_malloc(sizeof(struct number), size, (stack_copy_elem) number_copy);

	while (!stack_empty(stack_exp)) {
		struct token exp_token;
		stack_pop(stack_exp, &exp_token);
		eval_token(exp_token, operands);
		if (error_get()) {
			stack_free(stack_exp);
			stack_free(operands);
			return str_to_number(1, "0"); // why not
		}
	}
	assert(stack_empty(stack_exp));
	assert(stack_size(operands) == 1);

	struct number result;
	stack_pop(operands, &result);
	stack_free(stack_exp);
	stack_free(operands);
	return result;
}

