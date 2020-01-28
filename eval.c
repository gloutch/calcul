#include "eval.h"



static struct number binary_op(struct stack * operands, enum parser_token_type op) {

	struct number n2;
	stack_pop(operands, &n2);
	struct number n1;
	stack_pop(operands, &n1);

	switch (op) {

		case PLUS:
			n1.data.integer += n2.data.integer;
			break;
		case MINUS:
			n1.data.integer -= n2.data.integer;
			break;
		case ASTERISK:
			n1.data.integer *= n2.data.integer;
			break;
		default:
			assert(0);
	}
	return n1;
}


static void eval_token(const struct parser_token exp_token, struct stack * operands) {

	switch (exp_token.type) {

		case NUM_OPERAND: {
			struct number num = str_to_number(exp_token.len, exp_token.str, 10);
			assert(num.type == INTEGER);
			stack_push(operands, &num);
			return;
		}

		case VAR_OPERAND:
		case FUNC_NAME:
			assert(0);
			return;

		case PLUS:
		case MINUS:
		case ASTERISK: {
			struct number res = binary_op(operands, exp_token.type);
			stack_push(operands, &res);
			return;
		}

		case UNARY_PLUS:
			return;

		case UNARY_MINUS: {
			struct number * num = stack_peek(operands);
			num->data.integer = -num->data.integer;
			return;
		}

		default:
			assert(0);
	}
	assert(0);
}


struct number eval(struct parser_result exp) {

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