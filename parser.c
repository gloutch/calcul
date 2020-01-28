#include "parser.h"

#define TOKEN_STACK(size) stack_malloc(sizeof(struct parser_token), size, (stack_copy_elem) copy_parser_token);
#define TOKEN_ARRAY(size) (struct parser_token *) malloc(sizeof(struct parser_token) * (size));



/*
	TOKEN utilitary
*/

void print_parser_token(const struct parser_token * const token) {

	switch (token->type) {

		case NUM_OPERAND:
		case VAR_OPERAND:
			printf("OPERAND    [%d] %.*s", token->len, token->len, token->str);
			return;
		case FUNC_NAME:
			printf("FUNCTION   [%d] %.*s", token->len, token->len, token->str);
			return;
		case PLUS:
		case MINUS:
		case ASTERISK:
			printf("BINARY OP  %.*s", token->len, token->str);
			return;
		case UNARY_PLUS:
		case UNARY_MINUS:
			printf("UNARY OP   %.*s", token->len, token->str);
			return;
		case LPARENT:
		case RPARENT:
			printf("PARENT     %.*s", token->len, token->str);
			return;
		case ARG_SEP:
			printf("ARG SEP    %.*s", token->len, token->str);
			return;
		case ERROR:
			printf("UNKNOWN    %.*s", token->len, token->str);
			return;
	}
	assert(0);
}

void copy_parser_token(const struct parser_token * const src, struct parser_token * const dst) {
	*dst = *src;
}



/*
	INTERFACE `lexer_token`/`parser_token`
*/

static int is_binary_op(int i, const struct lexer_token * lex) {
	assert(lex[i].type == SYMBOL);

	if (i == 0) { // first token
		return 0;
	}
	enum lexer_token_type previous = lex[i - 1].type;
	return ((previous == NUMBER) || (previous == NAME)) || (previous == RPAREN); // juste AFTER is an operand
}

static enum parser_token_type lexer_token_to_parser_token(int i, int size, const struct lexer_token * lex) {

	struct lexer_token token = lex[i];
	switch (token.type) {

		case NUMBER:
			return NUM_OPERAND;
		case NAME:
			if ((i + 1 < size) && (lex[i + 1].type == LPAREN)) { // if there is a '(' after NAME
				return FUNC_NAME;
			}
			return VAR_OPERAND;

		case SYMBOL: {
			if (strncmp(token.str, "+", token.len) == 0) {
				return (is_binary_op(i, lex) ? PLUS : UNARY_PLUS);
			}
			if (strncmp(token.str, "-", token.len) == 0) {
				return (is_binary_op(i, lex) ? MINUS : UNARY_MINUS);
			}
			if (strncmp(token.str, "*", token.len) == 0) {
				return ASTERISK;
			}
			return ERROR;
		}
		case LPAREN:
			return LPARENT;
		case RPAREN:
			return RPARENT;
		case COMMA:
			return ARG_SEP;
		default:
			return ERROR;
	}
}

static struct parser_result convert_token(const struct lexer_result * lex) {

	struct parser_token * token_array = TOKEN_ARRAY(lex->size);
	CHECK_MALLOC(token_array, "convert_token");
	struct parser_token token;

	for (int i = 0; i < lex->size; i++) {

		token.type = lexer_token_to_parser_token(i, lex->size, lex->tarray);
		token.str  = lex->tarray[i].str;
		token.len  = lex->tarray[i].len;
		token_array[i] = token;

		if (token.type == ERROR) { // stops if an error occurs
			break;
		}
	}

	struct parser_result res;
	res.type   = CORRECT; // for now
	res.tarray = token_array;
	res.size   = lex->size;
	return res;
}



/*
	CHECK functions

Returns 0 if NO error is detected
Otherwise, returns 1 and edits the `struct parser_result` with the corresponding error.

*/

static int create_parser_err(struct parser_result * res, enum result_type err_type, struct parser_token err_token) {
	if (res->tarray != NULL) {
		LOG_FREE(res->tarray);
		free(res->tarray);
	}
	res->type    = err_type;
	res->tarray  = TOKEN_ARRAY(1);
	CHECK_MALLOC(res->tarray, " create_parser_err (parser.c) ");
	res->tarray[0] = err_token;
	res->size      = 1;
	return 1;
}

// check if lexer found an UNKNOWN token
static int check_lexer_err(struct parser_result * res, const struct lexer_result lex) {
	log_trace("check lexer error");

	if (lex.tarray[lex.size].type != UNKNOWN) {
		return 0;
	}
	struct lexer_token lexer_err = lex.tarray[lex.size];
	// create the `parser_token` from the `lexer_token`
	struct parser_token parser_err;
	parser_err.type = ERROR;
	parser_err.str  = lexer_err.str;
	parser_err.len  = lexer_err.len;
	return create_parser_err(res, ERR_SYM, parser_err);
}

// check if parser didn't recognise a token and typed it as ERROR
static int check_parser_token_err(struct parser_result * res) {
	log_trace("check parser unknown symbol");

	int n = res->size;
	const struct parser_token * token = res->tarray;

	for (int i = 0; i < n; i++) {
		if (token[i].type == ERROR) {
			return create_parser_err(res, ERR_TOKEN, token[i]);
		}
	}
	return 0;
}


// check parenthesis by counting them (no allocation)
// return 0 if correct, otherwise the max(lp, rp)
static int correct_parenthesis(int n, const struct parser_token * token) {
	int lp = 0;
	int rp = 0;

	for (int i = 0; i < n; i++) {

		if (token[i].type == LPARENT) {
			lp++;
		}
		else if (token[i].type == RPARENT) {
			rp++;
			if (rp > lp) { // more RP than LP
				return rp;
			}
		}
	}
	return (lp == rp ? 0 : (lp > rp ? lp : rp));
}

// call `correct_parenthesis` and then find the wrong one (using stack)
static int check_parenthesis(struct parser_result * res) {
	log_trace("check parser parenthesis");

	int n = res->size;
	const struct parser_token * token = res->tarray;

	int size = correct_parenthesis(n, token);
	if (size == 0) {
		return 0;
	}

	struct stack * stack = TOKEN_STACK(size);
	CHECK_MALLOC(stack, " check_parenthesis (parser.c) ");
	struct parser_token tmp;
	for (int i = 0; i < n; i++) {

		if (token[i].type == LPARENT) {
			stack_push(stack, &token[i]);
		}
		else if (token[i].type == RPARENT) {
			if (stack_empty(stack)) { // no corresponding LPARENT
				stack_push(stack, &token[i]);
				break;
			}
			stack_pop(stack, &tmp);
		}
	}
	assert(!stack_empty(stack)); //because parenthesis are wrong

	stack_pop(stack, &tmp);
	create_parser_err(res, ERR_PARENT, tmp);
	return 1;
}


#define OPERAND(to)  (((to) == NUM_OPERAND) || ((to) == VAR_OPERAND))
#define BINARY(op)   (((op) == PLUS) || ((op) == MINUS) || ((op) == ASTERISK))
#define UNARY(op)    (((op) == UNARY_PLUS) || ((op) == UNARY_MINUS))

// check if the next token is correct thanks to the current one
static int correct_next_token(enum parser_token_type curr, enum parser_token_type next) {

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
			assert(0);
			return 0;
	}
}

// check basic rules about order of token of a math expression
static int check_token_order(struct parser_result * res) {
	log_trace("check parser token order");

	int n = res->size;
	const struct parser_token * token = res->tarray;

	if (n <= 0) { // no token to check
		return 0;
	}

	enum parser_token_type init = token[0].type;
	if (!( OPERAND(init) || UNARY(init) || (init == FUNC_NAME) || (init == LPARENT) )) { // check first token
		return create_parser_err(res, ERR_UNEXPECT, token[0]);
	}

	for (int i = 1; i < n; i++) {
		const struct parser_token t1 = token[i - 1];
		const struct parser_token t2 = token[i];
		if (!correct_next_token(t1.type, t2.type)) {
			return create_parser_err(res, ERR_UNEXPECT, t2);
		}
	}

	enum parser_token_type last = token[n - 1].type; // check last token
	if (!( OPERAND(last) || (last == RPARENT) )) {
		return create_parser_err(res, ERR_UNEXPECT, token[n - 1]);
	}
	return 0;
}


// assume `correct_parenthesis` to not check this again
//    and `check_token_order` to assure FUNC_NAME is followed by LPARENT
// check ARG_SEP are in the right penrenthesis scope (using stack)
static int check_arg_sep(struct parser_result * res) {
	log_trace("check parser comma in function arg");

	int n = res->size;
	const struct parser_token * token = res->tarray;

	assert(correct_parenthesis(n, token) == 0);
	assert(check_token_order(res) == 0);

	struct stack * scope = TOKEN_STACK(n); // scope are functions or parenthesis
	struct parser_token dump;

	int i = 0;
	while (i < n) {
		switch (token[i].type) {

			case FUNC_NAME:
				stack_push(scope, &token[i]); 		// function scope
				i += 2; 							// skip the (
				break;

			case LPARENT:
				stack_push(scope, &token[i]);		// math expression (not a function call)
				i++;
				break;

			case RPARENT:
				stack_pop(scope, &dump);
				i++;
				break;

			case ARG_SEP: {							// check that the last scope is a function
				if (stack_empty(scope) || (((struct parser_token *) stack_peek(scope))->type != FUNC_NAME)) {
					create_parser_err(res, ERR_ARG_SEP, token[i]);
					free(scope);
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
	stack_free(scope);
	return 0;
}



/*
	PARSER
*/


struct parser_result parser(char const * string) {

	struct parser_result res;
	res.tarray = NULL;

	// get lexer_token
	struct lexer_result lex = lexer(string);
	if (check_lexer_err(&res, lex)) {
		free_lexer_result(lex);
		return res;
	}
	
	// lexer_token => parser_token
	res = convert_token(&lex);
	free_lexer_result(lex);

	// now, work on parser_token
	if (check_parser_token_err(&res)) {
		return res;
	}
	if (check_parenthesis(&res)) {
		return res;
	}
	if (check_token_order(&res)) {
		return res;
	}
	// and sorry, I failed to check the syntaxe without additional allocation
	if (check_arg_sep(&res)) {
		return res;
	}
	return res;
}

void free_parser_result(struct parser_result res) {
	res.size = 0;
	LOG_FREE(res.tarray);
	free(res.tarray);
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
	struct lexer_result lex1 = lexer("-3 + -2 * f(-1)");
	assert(!is_binary_op(0, lex1.tarray));
	assert(is_binary_op (2, lex1.tarray));
	assert(!is_binary_op(3, lex1.tarray));
	assert(is_binary_op (5, lex1.tarray));
	assert(!is_binary_op(8, lex1.tarray));
	free_lexer_result(lex1);


	printf(" lexer_token_to_parser_token\n");
	struct lexer_result lex2 = lexer("12 + -(13.0 * +var_1 - max(-1, 2)) §");
	assert(lex2.size == 17);

	int size2 = lex2.size;
	assert(lexer_token_to_parser_token(0,  size2, lex2.tarray) == NUM_OPERAND);
	assert(lexer_token_to_parser_token(1,  size2, lex2.tarray) == PLUS);
	assert(lexer_token_to_parser_token(2,  size2, lex2.tarray) == UNARY_MINUS);
	assert(lexer_token_to_parser_token(3,  size2, lex2.tarray) == LPARENT);
	assert(lexer_token_to_parser_token(4,  size2, lex2.tarray) == NUM_OPERAND);
	assert(lexer_token_to_parser_token(5,  size2, lex2.tarray) == ASTERISK);
	assert(lexer_token_to_parser_token(6,  size2, lex2.tarray) == UNARY_PLUS);
	assert(lexer_token_to_parser_token(7,  size2, lex2.tarray) == VAR_OPERAND);
	assert(lexer_token_to_parser_token(8,  size2, lex2.tarray) == MINUS);
	assert(lexer_token_to_parser_token(9,  size2, lex2.tarray) == FUNC_NAME);
	assert(lexer_token_to_parser_token(10, size2, lex2.tarray) == LPARENT);
	assert(lexer_token_to_parser_token(11, size2, lex2.tarray) == UNARY_MINUS);
	assert(lexer_token_to_parser_token(12, size2, lex2.tarray) == NUM_OPERAND);
	assert(lexer_token_to_parser_token(13, size2, lex2.tarray) == ARG_SEP);
	assert(lexer_token_to_parser_token(14, size2, lex2.tarray) == NUM_OPERAND);
	assert(lexer_token_to_parser_token(15, size2, lex2.tarray) == RPARENT);
	assert(lexer_token_to_parser_token(16, size2, lex2.tarray) == RPARENT);
	assert(lexer_token_to_parser_token(17, size2, lex2.tarray) == ERROR);
	free_lexer_result(lex2);


	printf(" check_lexer_err\n");
	struct lexer_result lex3 = lexer("12 + 3 -"); // count 5
	struct parser_result res3;
	res3.type   = CORRECT;
	res3.tarray = NULL;
	res3.size   = 0;

	assert(check_lexer_err(&res3, lex3) == 0);
	free_lexer_result(lex3);
	lex3 = lexer("12 + 3 §"); // count 4 
	assert(check_lexer_err(&res3, lex3) == 1);
	free_parser_result(res3);
	free_lexer_result(lex3);


	// printf(" check_parser_token_err\n");
	// TODO


	printf(" correct_parenthesis\n");
	struct lexer_result lex4 = lexer("() () () ( () () )");
	assert(check_lexer_err(NULL, lex4) == 0);
	int size4 = lex4.size;
	struct parser_result pars4 = convert_token(&lex4);
	free_lexer_result(lex4);
	assert(correct_parenthesis(size4, pars4.tarray) == 0);
	free_parser_result(pars4);

	lex4 = lexer("( () )) ((()))");
	assert(check_lexer_err(NULL, lex4) == 0);
	size4 = lex4.size;
	pars4 = convert_token(&lex4);
	free_lexer_result(lex4);
	assert(correct_parenthesis(size4, pars4.tarray) == 3);
	free_parser_result(pars4);

	lex4 = lexer("( () (()))");
	assert(check_lexer_err(NULL, lex4) == 0);
	size4 = lex4.size;
	pars4 = convert_token(&lex4);
	free_lexer_result(lex4);
	assert(correct_parenthesis(size4, pars4.tarray) == 0);
	free_parser_result(pars4);


	printf("done\n\n");
	#endif
}



