#include "parser.h"

#define OPERAND(to)  (((to) == NUM_OPERAND) || ((to) == VAR_OPERAND))
#define BINARY(op)   (((op) == PLUS)        || ((op) == MINUS)       || ((op) == ASTERISK))
#define UNARY(op)    (((op) == UNARY_PLUS)  || ((op) == UNARY_MINUS))


/*
	INTERFACE lexer/parser
*/

static int is_binary_op(int i, const struct token * list) {
	assert(list[i].type == SYMBOL);

	if (i == 0) { // first token
		return 0;
	}
	assert(i > 0);
	enum token_type previous = list[i - 1].type;
	return ((previous == NUMBER) || (previous == NAME) || (previous == RPARENT)); // juste AFTER is an operand
}

static enum token_type convert_token(int i, int n, const struct token * t) {

	struct token token = t[i];
	switch (token.type) {

		case NAME:
			if ((i + 1 < n) && (t[i + 1].type == LPARENT)) { // if there is a '(' after NAME
				return FUNC_NAME;
			}
			return VAR_OPERAND;

		case SYMBOL: {
			if (strncmp(token.str, "+", token.len) == 0) {
				return (is_binary_op(i, t) ? PLUS : UNARY_PLUS);
			}
			if (strncmp(token.str, "-", token.len) == 0) {
				return (is_binary_op(i, t) ? MINUS : UNARY_MINUS);
			}
			if (strncmp(token.str, "*", token.len) == 0) {
				return ASTERISK;
			}
			return UNKNOWN;
		}
		case NUMBER:
			return NUM_OPERAND;
		// unchange
		case LPARENT:
		case RPARENT:
		case ARG_SEP:
			return token.type;
		default:
			return UNKNOWN;
	}
}

struct expr lexer_to_parser(const struct expr * e) {

	struct expr result = token_expr(e->len);
	enum token_type tmp;

	for (int i = 0; i < e->len; i++) {

		tmp = convert_token(i, e->len, e->list);
		if (tmp == UNKNOWN) {
			error_set(UNKNOWN_TOK, NULL, e->list[i].str, e->list[i].len);
			return result;
		}
		result.list[i]      = e->list[i];
		result.list[i].type = tmp;
	}

	return result;
}



/*
	CHECK functions

Returns 0 if NO error is detected
Otherwise, returns 1 and edits the `struct pexpr` with the corresponding error.

*/

#define TOKEN_STACK(size) stack_malloc(sizeof(struct token), size, (stack_copy_elem) token_copy);

// check parenthesis by counting them (no allocation)
// return 0 if correct, otherwise the max(lp, rp)
static int correct_parenthesis(int n, const struct token * l) {
	int lp = 0;
	int rp = 0;

	for (int i = 0; i < n; i++) {

		if (l[i].type == LPARENT) {
			lp++;
		}
		else if (l[i].type == RPARENT) {
			rp++;
			if (rp > lp) { // more RP than LP
				return rp;
			}
		}
	}
	return (lp == rp ? 0 : (lp > rp ? lp : rp));
}

// call `correct_parenthesis` and then find the wrong one (using stack)
static int check_parenthesis(int n, const struct token * list) {

	int size = correct_parenthesis(n, list);
	if (size == 0) {
		return 0;
	}

	struct stack * stack = TOKEN_STACK(size);
	struct token tmp;

	for (int i = 0; i < n; i++) {

		if (list[i].type == LPARENT) {
			stack_push(stack, &list[i]);
		}
		else if (list[i].type == RPARENT) {
			if (stack_empty(stack)) { // no corresponding LPARENT
				stack_push(stack, &list[i]);
				break;
			}
			stack_pop(stack, &tmp);
		}
	}
	assert(!stack_empty(stack)); //because parenthesis are wrong

	stack_pop(stack, &tmp);
	stack_free(stack);
	error_set(MIS_PARENT, tmp.str, NULL, 0);
	return 1;
}



// check if the next token is correct thanks to the current one
static int correct_next_token(enum token_type curr, enum token_type next) {

	switch (curr) {
		case PLUS: 			// operator (unary and binary)
		case MINUS:
		case ASTERISK:
		case UNARY_PLUS:
		case UNARY_MINUS:
			return (OPERAND(next) || UNARY(next) || (next == LPARENT) || (next == FUNC_NAME));
		case NUM_OPERAND: 	// operand
		case VAR_OPERAND:
			return (BINARY(next) || (next == RPARENT) || (next == ARG_SEP));
		case LPARENT:
			return (OPERAND(next) || UNARY(next) || (next == FUNC_NAME) || (next == LPARENT) || (next == RPARENT));
		case RPARENT:
			return (BINARY(next) || (next == ARG_SEP) || (next == RPARENT));
		case FUNC_NAME:
			return (next == LPARENT);
		case ARG_SEP:
			return (OPERAND(next) || UNARY(next) || (next == FUNC_NAME) || (next == LPARENT));
		default:
			return 0;
	}
}

// check basic rules about order of token of a math expression
static int check_token_order(int n, const struct token * list) {

	if (n <= 0) { // no token to check
		return 0;
	}

	enum token_type init = list[0].type;
	if (!( OPERAND(init) || UNARY(init) || (init == FUNC_NAME) || (init == LPARENT) )) { // check first token
		error_set(UNEXP_TOK, NULL, list[0].str, list[0].len);
		return 1;
	}

	for (int i = 1; i < n; i++) {
		const struct token t1 = list[i - 1];
		const struct token t2 = list[i];
		if (!correct_next_token(t1.type, t2.type)) {
			error_set(UNEXP_TOK, NULL, t2.str, t2.len);
			return 1;
		}
	}

	enum token_type last = list[n - 1].type; // check last token
	if (!( OPERAND(last) || (last == RPARENT) )) {
		error_set(UNEXP_TOK, NULL, list[n - 1].str, list[n - 1].len);
		return 1;
	}
	return 0;
}



static struct token * find_last_function(struct stack * scope) {
	struct token dump; 	
	while (!stack_empty(scope)) {

		struct token * top = (struct token *) stack_peek(scope);
		if (top->type == FUNC_NAME) {
			return top;
		}
		stack_pop(scope, &dump);
	}
	return NULL;
}

// assume `correct_parenthesis` to not check this again
//    and `check_token_order` to assure FUNC_NAME is followed by LPARENT
// check ARG_SEP are in the right penrenthesis scope (using stack)
static int check_arg_sep(int n, const struct token * list) {
	assert(correct_parenthesis(n, list) == 0);
	assert(check_token_order(n, list) == 0);

	struct stack * scope = TOKEN_STACK(n); // scope are functions or parenthesis
	struct token dump;

	int i = 0;
	while (i < n) {
		switch (list[i].type) {

			case FUNC_NAME:
				stack_push(scope, &list[i]); 		// function scope
				i += 2; 							// skip the (
				break;

			case LPARENT:
				stack_push(scope, &list[i]);		// math expression (not a function call)
				i++;
				break;

			case RPARENT:
				stack_pop(scope, &dump);			// no error thanks to correct_parenthesis
				i++;
				break;

			case ARG_SEP: {							// check that the last scope is a function
				if (stack_empty(scope) || (((struct token *) stack_peek(scope))->type != FUNC_NAME)) {
					
					struct token * func = find_last_function(scope);
					if (func != NULL) {
						error_set(MIS_ARG_SEP, list[i].str, func->str, func->len);
					} else {
						error_set(MIS_ARG_SEP, list[i].str, NULL, 0);
					}
					stack_free(scope);
					return 1;
				}
				i++;
				break;

			default:
				i++;
				break;
			}
		}
	}
	assert(stack_empty(scope));
	stack_free(scope);
	return 0;
}



/*
	check_syntax
*/

int parser_check_syntax(const struct expr e) {

	if (check_parenthesis(e.len, e.list)) {
		return 1;
	}
	if (check_token_order(e.len, e.list)) {
		return 1;
	}
	// and sorry, I failed to check the syntaxe without additional allocation
	if (check_arg_sep(e.len, e.list)) {
		return 1;
	}
	return 0;
}


/*
	TEST SECTION
*/

void test_parser() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else
	printf("PARSER: \n");


	printf(" is_binary_op\n");
	struct expr lex1 = lexer("-3 + -2 * f(-1)");
	assert(!is_binary_op(0, lex1.list));
	assert(is_binary_op (2, lex1.list));
	assert(!is_binary_op(3, lex1.list));
	assert(is_binary_op (5, lex1.list));
	assert(!is_binary_op(8, lex1.list));
	token_free_expr(&lex1);


	printf(" convert_token\n");
	struct expr lex2 = lexer("12 + -(13.0 * +var_1 - max(-1, 2))");
	assert(error_get() == NO_ERROR);
	assert(lex2.len == 17);

	int size2 = lex2.len;
	assert(convert_token(0,  size2, lex2.list) == NUM_OPERAND);
	assert(convert_token(1,  size2, lex2.list) == PLUS);
	assert(convert_token(2,  size2, lex2.list) == UNARY_MINUS);
	assert(convert_token(3,  size2, lex2.list) == LPARENT);
	assert(convert_token(4,  size2, lex2.list) == NUM_OPERAND);
	assert(convert_token(5,  size2, lex2.list) == ASTERISK);
	assert(convert_token(6,  size2, lex2.list) == UNARY_PLUS);
	assert(convert_token(7,  size2, lex2.list) == VAR_OPERAND);
	assert(convert_token(8,  size2, lex2.list) == MINUS);
	assert(convert_token(9,  size2, lex2.list) == FUNC_NAME);
	assert(convert_token(10, size2, lex2.list) == LPARENT);
	assert(convert_token(11, size2, lex2.list) == UNARY_MINUS);
	assert(convert_token(12, size2, lex2.list) == NUM_OPERAND);
	assert(convert_token(13, size2, lex2.list) == ARG_SEP);
	assert(convert_token(14, size2, lex2.list) == NUM_OPERAND);
	assert(convert_token(15, size2, lex2.list) == RPARENT);
	assert(convert_token(16, size2, lex2.list) == RPARENT);
	token_free_expr(&lex2);


	printf(" correct_parenthesis\n");
	struct expr lex;
	struct expr pars;
	int size;

	lex  = lexer("() () () ( () () )");
	size = lex.len;
	pars = lexer_to_parser(&lex);
	token_free_expr(&lex);
	assert(correct_parenthesis(size, pars.list) == 0);
	token_free_expr(&pars);

	lex  = lexer("() () () ( () () )");
	size = lex.len;
	pars = lexer_to_parser(&lex);
	token_free_expr(&lex);
	assert(correct_parenthesis(size, pars.list) == 0);
	token_free_expr(&pars);

	lex  = lexer("( () )) ((()))");
	size = lex.len;
	pars = lexer_to_parser(&lex);
	token_free_expr(&lex);
	assert(correct_parenthesis(size, pars.list) == 3);
	token_free_expr(&pars);

	lex  = lexer("( () (()))");
	size = lex.len;
	pars = lexer_to_parser(&lex);
	token_free_expr(&lex);
	assert(correct_parenthesis(size, pars.list) == 0);
	token_free_expr(&pars);


	printf("done\n\n");
	#endif
}



