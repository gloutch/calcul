// #include "parser.h"


// /*
// 	CHANGE SYNTAX
// */


// static int op_left_asso(struct token const * const tok) {
// 	return 1; // no right associative operator for now
// }


// static int preced(struct token const * const tok) {
// 	switch (tok->type) {
// 		case LP:
// 		case RP:
// 			return 1;
// 		case PLUS:
// 			return 2;
// 		case MULT:
// 			return 3;
// 		default:
// 			assert(0);
// 			return 0;
// 	}
// }

// // let's assume the parenthesis are correct
// static void shunting_yard_wye(struct token const * const input, struct stack * const operator, struct stack * const output) {

// 	switch (input->type) {

// 		case INT_NUM: // number
// 			stack_push(output, (void *) input);
// 			return;

// 		case PLUS: // operator
// 		case MULT: {

// 			while (!stack_empty(operator)) {
// 				struct token const * top = stack_peek(operator);

// 				// ugly condition about the top token on the operator stack
// 				if (!(
// 					(top->type != LP)
// 						&&
// 						((preced(top) > preced(input))
// 							||
// 						((preced(top) == preced(input)) && op_left_asso(top)))
// 					))
// 				{
// 					break;
// 				}
// 				struct token tmp;
// 				stack_pop(operator, &tmp);
// 				stack_push(output, &tmp);
// 			}

// 			stack_push(operator, input);
// 			return;
// 		}
// 		case LP:
// 			stack_push(operator, (void *) input);
// 			return;

// 		case RP: {

// 			struct token tmp;
// 			stack_pop(operator, &tmp); // the stack can't be empty because there is the LP token

// 			while (tmp.type != LP) {
// 				stack_push(output, &tmp);
// 				stack_pop(operator, &tmp); // the LP doesn't pop up, thus the stack isn't empty
// 			}
// 			assert(tmp.type == LP);
// 			return;
// 		}
// 		default:
// 			assert(0);
// 			return;
// 	}
// }


// static struct stack * const shunting_yard(struct token const * const array) {

// 	int size = token_array_len(array) - 1; // without END_TOKEN
// 	// oversize stack
// 	struct stack * operator = stack_malloc(sizeof(struct token), size, (stack_copy_elem) copy_token);
// 	struct stack * output   = stack_malloc(sizeof(struct token), size, (stack_copy_elem) copy_token);

// 	for (int i = 0; i < size; i++) {
// 		shunting_yard_wye(&array[i], operator, output);
// 	}

// 	struct token tmp;
// 	while (!stack_empty(operator)) {
// 		stack_pop(operator, &tmp);
// 		stack_push(output, &tmp);
// 	}
// 	assert(stack_empty(operator));

// 	// clear
// 	stack_free(operator);
// 	stack_reverse(output);
// 	return output;
// }


// static void print_token_RPN(struct token const * const tok) {
// 	printf(" %.*s ", tok->len, tok->str);
// }


// void print_RPN_stack(struct stack const * const RPN) {
// 	stack_print(RPN, (stack_print_elem) print_token_RPN);
// }


// /*
// 	CHECK FUNCTION

// Returns 0 if NO error is detected
// Otherwise, returns 1 and edits the `struct parser_result` with the corresponding error.

// */


// static int check_err_null(struct parser_result * const res, char const * const str) {
// 	if (str != NULL) {
// 		return 0;
// 	}
// 	res->RPN_stack = NULL;
// 	res->type      = ERR_NULL;
// 	return 1;
// }


// static int check_err_token(struct parser_result * const res, struct token const * const array) {
	
// 	int last = token_array_len(array) - 1;

// 	if (array[last].type == END_TOKEN) {
// 		return 0;
// 	}
// 	// array[i].type == ERROR

// 	struct stack * err = stack_malloc(sizeof(struct token), 1, (stack_copy_elem) copy_token);
// 	stack_push(err, &array[last]);

// 	res->RPN_stack = err;
// 	res->type      = ERR_TOKEN;
// 	return 1;
// }


// static int check_err_parent(struct parser_result * const res, struct token const * const array) {

// 	int const len = token_array_len(array);

// 	int lp_counter = 0;
// 	int rp_counter = 0;

// 	for (int i = 0; i < len; i++) {

// 		if (array[i].type == LP) {
// 			lp_counter++;
// 		}
// 		else if (array[i].type == RP) {
// 			rp_counter++;
// 		}
// 	}
// 	if (lp_counter == rp_counter) { // correct parenthesis number
// 		return 0;
// 	}

// 	// Find the parenthesis that doesn't match
// 	int max_parent = (lp_counter < rp_counter ? rp_counter : lp_counter);
// 	struct stack * err = stack_malloc(sizeof(struct token), max_parent, (stack_copy_elem) copy_token);
// 	struct token tmp;

// 	for (int i = 0; i < len; i++) {

// 		if (array[i].type == LP) {
// 			stack_push(err, &array[i]);
// 		}
// 		else if (array[i].type == RP) {

// 			if (stack_empty(err)) { // there is not LP corresponding to this RP
// 				stack_push(err, &array[i]);
// 				break;
// 			}
// 			stack_pop(err, (void *) &tmp);
// 		}
// 	}
// 	assert(!stack_empty(err)); // the top parenthesis token on the stack mismatch

// 	res->RPN_stack = err;
// 	res->type      = ERR_PARENT;
// 	return 1;
// }


// /*
// 	PARSER
// */


// // use all function above and check result at each step
// struct parser_result parser(char const * string) {

// 	// init the returned structure
// 	struct parser_result res;
// 	res.RPN_stack = NULL;
// 	res.type      = CORRECT;

// 	if (check_err_null(&res, string)) {
// 		return res;
// 	}

// 	// lexer
// 	struct token * tok_array = lexer(string);
	
// 	if (check_err_token(&res, tok_array)) {
// 		free(tok_array);
// 		return res;
// 	}
// 	// Even if shunting yard check parenthesis
// 	// I prefere check before to create useless stacks
// 	if (check_err_parent(&res, tok_array)) {
// 		free(tok_array);
// 		return res;
// 	}

// 	// syntaxe change (NO ERROR)
// 	struct stack * RPN_stack = shunting_yard(tok_array);
// 	free(tok_array);

// 	res.RPN_stack = RPN_stack;
// 	res.type      = CORRECT;
// 	return res;
// }


// /*
// 	TEST SECTION
// */


// static void test_shunting_yard() {
	
// 	// too lazy to test `shunting_yard_wye`
// 	// I directly test `shunting_yard`

// 	char *str = " 34 + 4  ";
// 	struct token * arr1 = lexer(str);
// 	struct stack * RPN1 = shunting_yard(arr1); // 34 4 +
// 	free(arr1);

// 	assert(stack_size(RPN1) == 3);
// 	struct token tmp;

// 	stack_pop(RPN1, &tmp);
// 	assert(tmp.type == INT_NUM);
// 	assert(tmp.str == (str + 1));
// 	assert(tmp.len == 2);

// 	stack_pop(RPN1, &tmp);
// 	assert(tmp.type == INT_NUM);
// 	assert(tmp.str == (str + 6));
// 	assert(tmp.len == 1);

// 	stack_pop(RPN1, &tmp);
// 	assert(tmp.type == PLUS);
// 	assert(tmp.str == (str + 4));
// 	assert(tmp.len == 1);

// 	stack_free(RPN1);

// 	// TODO an better example
// }

// static void test_check() {

// 	struct parser_result res;
// 	res.type = CORRECT;

// 	// check_err_null
// 	assert(!check_err_null(&res, ""));
// 	assert(res.type == CORRECT);
// 	assert(!check_err_null(&res, "nothing"));
// 	assert(res.type == CORRECT);
// 	assert(check_err_null(&res, NULL));
// 	assert(res.type == ERR_NULL);


// 	// check_err_token
// 	res.type = CORRECT;

// 	struct token *arr1 = lexer("18 + 3");
// 	assert(!check_err_token(&res, arr1));
// 	assert(res.type == CORRECT);
// 	free(arr1);

// 	struct token *arr2 = lexer("18 + A");
// 	assert(check_err_token(&res, arr2));
// 	assert(res.type == ERR_TOKEN);

// 	struct token e1;
// 	stack_pop(res.RPN_stack, &e1);
// 	assert(e1.type == arr2[2].type); // same token, (error token '==' arr2[2])
// 	assert(e1.str == arr2[2].str);
// 	assert(e1.len == arr2[2].len);
// 	free(arr2);


// 	// check_err_parrnt
// 	res.type = CORRECT;

// 	struct token *parent1 = lexer("18 + 3");
// 	assert(!check_err_parent(&res, parent1));
// 	assert(res.type == CORRECT);
// 	free(parent1);

// 	struct token *parent2 = lexer("18 * (8 * 3  ");
// 	assert(check_err_parent(&res, parent2));
// 	assert(res.type == ERR_PARENT);

// 	struct token p2;
// 	stack_pop(res.RPN_stack, &p2);
// 	assert(p2.type == LP);
// 	assert(*(p2.str) == '(');
// 	assert(p2.len == 1);
// 	free(parent2);

// 	struct token *parent3 = lexer("18 *  8 * 3 )");
// 	assert(check_err_parent(&res, parent2));
// 	assert(res.type == ERR_PARENT);

// 	struct token p3;
// 	stack_pop(res.RPN_stack, &p3);
// 	assert(p3.type == RP);
// 	assert(*(p3.str) == ')');
// 	assert(p3.len == 1);
// 	free(parent3);
// }

// // TODO test parser function

// void test_parser() {

// 	#ifdef NDEBUG
// 	printf("COMPILE ERROR: should NOT be compile with '-DNDEBUG'\n\n");
// 	exit(1);
// 	#endif

// 	printf("TEST lexer: ");
// 	test_lexer();
// 	printf("done\n");

// 	printf("TEST parser check: ");
// 	test_check();
// 	printf("done\n");

// 	printf("TEST shunting yard: ");
// 	test_shunting_yard();
// 	printf("done\n");
// }











