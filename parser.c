#include "parser.h"


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

// retrun a malloced array of token
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






/*
	TEST SECTION
*/



void test_lexer() {

	printf("TEST lexer: ");


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
	struct token *array = lexer("12 + ( 3 * 4 + 18) + (3*4)");

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


	printf("done\n");
}













