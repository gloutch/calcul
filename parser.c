#include "parser.h"

/*
	LEXER
*/

void print_token(struct token const * const tok) {
	switch (tok->type) {

		case INT_NUM:
			printf("integer[%d] %.*s\n", tok->len, tok->len, tok->str);
			break;
		case PLUS:
		case MULT:
			printf("operator %.*s\n", tok->len, tok->str);
			break;
		case LP:
		case RP:
			printf("parenthesis %.*s\n", tok->len, tok->str);
			break;
		case END_TOKEN:
			printf("NO token left\n");
			break;
		case ERROR:
			printf("error Unkown token: '%.*s'\n", tok->len, tok->str);
			break;
		default:
			printf("ERROR, can't print token\n");
			break;
	}
}


void copy_token(struct token const * const src, struct token * const dst) {
	*dst = *src;
}


// the only ERROR can be an 'Unknown token'
static void next_token(char const * const string, struct token * const tok) {

	int i = 0;

	while (isspace(string[i])) {
		i++;
	}
	if (string[i] == 0) { // no token left to return
		tok->type = END_TOKEN;
		tok->str = NULL;
		tok->len = 0;
		return;
	}

	// The result token will be defined from this char
	char c = string[i];
	tok->str = string + i;

	// First try one-length token
	tok->len = 1;

	switch (c) {
		case '(':
			tok->type = LP;
			return;
		case ')':
			tok->type = RP;
			return;
		case '+':
			tok->type = PLUS;
			return;
		case '*':
			tok->type = MULT;
			return;
	}

	// Second, try for integer
	if (isdigit(c)) {
		int len = 1;
		while (isdigit(string[i + len])) {
			len++;
		}
		tok->type = INT_NUM;
		tok->len = len;
		return;
	}

	// assume that's an Unkown token
	tok->type = ERROR;
}


static int count_token(char const *string) {

	int count = 0;
	struct token tok;
	do {
		next_token(string, &tok);
		string = tok.str + tok.len;
		count++;
	}
	while ((tok.type != END_TOKEN) && (tok.type != ERROR));

	return count;
}


// retrun a malloced array of token ended by END_TOKEN or ERROR
struct token *lexer(char const *string) {

	int count = count_token(string);
	struct token * array = malloc(sizeof(struct token) * count);
	CHECK_MALLOC(array, "NULL malloc in lexer\n");
	struct token tmp;

	for (int i = 0; i < count; i++) {
		next_token(string, &tmp);
		array[i] = tmp;
		string = tmp.str + tmp.len;
	}

	assert((array[count - 1].type == END_TOKEN) || (array[count - 1].type == ERROR));
	return array;
}


// return the length on the array returned by `lexer`
static int token_array_len(struct token const * const array) {
	int i = 0;
	while ((array[i].type != END_TOKEN) && (array[i].type != ERROR)) {
		i++;
	}
	return i + 1;
}


/*
	CHANGE SYNTAX
*/


// static int op_is_left_asso(struct token const * const tok) {
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
// 			return 0;
// 	}
// }


// // change the input token as an ERROR if syntaxe is wrong
// // The only error can be a mismatched parentheses
// static struct token * shunting_yard(struct token * const input, struct stack * const operator, struct stack * const output) {


// 	switch (input->type) {

// 		case INT_NUM: // number
// 			stack_push(output, (void *) input);
// 			return input;

// 		case PLUS: // operator
// 		case MULT: {
// 			struct token top;
// 			stack_pop(operator, (void *) &top);

// 			// sorry for the ugly condition
// 			while (
// 				(top.type != LP)
// 					&&
// 					((preced(&top) > preced(input)) 
// 						|| 
// 					(preced(&top) == preced(input) && op_is_left_asso(&top)))) 
// 			{
// 				stack_push(output, &top);
// 			}
// 			stack_push(operator, input);
// 			return input;
// 		}

// 		case LP: // left parenthesis
// 			stack_push(operator, (void *) input);
// 			return input;

// 		case RP: // right parenthesis
// 			return input;

// 		default:
// 			assert(0);
// 			return input;
// 	}
// }



/*
	CHECK FUNCTION

Return 0 if NO error is detected
Otherwise, edits the `struct parser_result` with the corresponding error.

*/



static int check_err_null(struct parser_result * const res, char const * const str) {
	if (str != NULL) {
		return 0;
	}
	res->RPN_stack = NULL;
	res->type      = ERR_NULL;
	return 1;
}


static int check_err_token(struct parser_result * const res, struct token const * const array) {
	
	int last = token_array_len(array) - 1;

	if (array[last].type == END_TOKEN) {
		return 0;
	}
	// array[i].type == ERROR

	struct stack * err = stack_malloc(sizeof(struct token), 1, (stack_copy_elem) copy_token);
	stack_push(err, &array[last]);

	res->RPN_stack = err;
	res->type      = ERR_TOKEN;
	return 1;
}


static int check_err_parent(struct parser_result * const res, struct token const * const array) {

	int const len = token_array_len(array);

	int lp_counter = 0;
	int rp_counter = 0;

	for (int i = 0; i < len; i++) {

		if (array[i].type == LP) {
			lp_counter++;
		}
		else if (array[i].type == RP) {
			rp_counter++;
		}
	}
	if (lp_counter == rp_counter) { // correct parenthesis number
		return 0;
	}

	// Find the parenthesis that doesn't match
	int max_parent = (lp_counter < rp_counter ? rp_counter : lp_counter);
	struct stack * err = stack_malloc(sizeof(struct token), max_parent, (stack_copy_elem) copy_token);
	struct token tmp;

	for (int i = 0; i < len; i++) {

		if (array[i].type == LP) {
			stack_push(err, &array[i]);
		}
		else if (array[i].type == RP) {

			if (stack_empty(err)) { // there is not LP corresponding to this RP
				stack_push(err, &array[i]);
				break;
			}
			stack_pop(err, (void *) &tmp);
		}
	}
	assert(!stack_empty(err)); // the top parenthesis token on the stack mismatch

	res->RPN_stack = err;
	res->type      = ERR_PARENT;
	return 1;
}


/*
	PARSER
*/


// use all function above and check result at each step
struct parser_result parser(char const * string) {

	// init the returned structure
	struct parser_result res;
	res.RPN_stack = NULL;
	res.type      = CORRECT;

	if (check_err_null(&res, string)) {
		return res;
	}

	struct token * tok_array = lexer(string);
	
	if (check_err_token(&res, tok_array)) {
		free(tok_array);
		return res;
	}
	if (check_err_parent(&res, tok_array)) {
		free(tok_array);
		return res;
	}

	free(tok_array);
	return res;
}



/*
	TEST SECTION
*/


static void test_lexer() {

	// next_token
	char *s1 = "12 + ( 12345";
	struct token t1;

	next_token(s1, &t1);
	assert(t1.type == INT_NUM);
	assert(t1.str == s1);
	assert(t1.len == 2);

	next_token(s1 + 2, &t1);
	assert(t1.type == PLUS);
	assert(t1.str == s1 + 3);
	assert(t1.len == 1);

	next_token(s1 + 4, &t1);
	assert(t1.type == LP);
	assert(t1.str == s1 + 5);
	assert(t1.len == 1);

	next_token(s1 + 6, &t1);
	assert(t1.type == INT_NUM);
	assert(t1.str == s1 + 7);
	assert(t1.len == 5);

	next_token(s1 + 12, &t1);
	assert(t1.type == END_TOKEN);

	next_token("aa", &t1);
	assert(t1.type == ERROR);


	// count_token
	assert(count_token("") == 1); // none
	assert(count_token("12") == 2);
	assert(count_token("4 4 4 4") == 5);
	assert(count_token("12 + ( 3 * 4 + 18) + (3*4)") == 16);


	// lexer
	struct token *array = lexer("12 +  ( 3 * 4 +   18)  + (3*4)");

	assert(token_array_len(array) == 16);
	// for (int i = 0; i < 16; i++) {
	// 	print_token(&array[i]);
	// }
	assert(array[0].type == INT_NUM);
	assert(array[1].type == PLUS);
	assert(array[2].type == LP);
	assert(array[3].type == INT_NUM);
	assert(array[4].type == MULT);
	assert(array[5].type == INT_NUM);
	assert(array[6].type == PLUS);
	assert(array[7].type == INT_NUM);
	assert(array[8].type == RP);
	assert(array[9].type == PLUS);
	assert(array[10].type == LP);
	assert(array[11].type == INT_NUM);
	assert(array[12].type == MULT);
	assert(array[13].type == INT_NUM);
	assert(array[14].type == RP);
	assert(array[15].type == END_TOKEN);

	free(array);
}


static void test_check() {

	struct parser_result res;
	res.type = CORRECT;

	// check_err_null
	assert(!check_err_null(&res, ""));
	assert(res.type == CORRECT);
	assert(!check_err_null(&res, "nothing"));
	assert(res.type == CORRECT);
	assert(check_err_null(&res, NULL));
	assert(res.type == ERR_NULL);


	// check_err_token
	res.type = CORRECT;

	struct token *arr1 = lexer("18 + 3");
	assert(!check_err_token(&res, arr1));
	assert(res.type == CORRECT);
	free(arr1);

	struct token *arr2 = lexer("18 + A");
	assert(check_err_token(&res, arr2));
	assert(res.type == ERR_TOKEN);

	struct token e1;
	stack_pop(res.RPN_stack, &e1);
	assert(e1.type == arr2[2].type); // same token, (error token '==' arr2[2])
	assert(e1.str == arr2[2].str);
	assert(e1.len == arr2[2].len);
	free(arr2);


	// check_err_parrnt
	res.type = CORRECT;

	struct token *parent1 = lexer("18 + 3");
	assert(!check_err_parent(&res, parent1));
	assert(res.type == CORRECT);
	free(parent1);

	struct token *parent2 = lexer("18 * (8 * 3  ");
	assert(check_err_parent(&res, parent2));
	assert(res.type == ERR_PARENT);

	struct token p2;
	stack_pop(res.RPN_stack, &p2);
	assert(p2.type == LP);
	assert(*(p2.str) == '(');
	assert(p2.len == 1);
	free(parent2);

	struct token *parent3 = lexer("18 *  8 * 3 )");
	assert(check_err_parent(&res, parent2));
	assert(res.type == ERR_PARENT);

	struct token p3;
	stack_pop(res.RPN_stack, &p3);
	assert(p3.type == RP);
	assert(*(p3.str) == ')');
	assert(p3.len == 1);
	free(parent3);

}


void test_parser() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#endif

	printf("TEST lexer: ");
	test_lexer();
	printf("done\n");

	printf("TEST parser check: ");
	test_check();
	printf("done\n");
}











