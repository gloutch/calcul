#include "lexer.h"



void print_lexer_token(const struct lexer_token * const tok) {

	switch (tok->type) {

		case NUMBER:
			printf("NUMBER  [%d] %.*s \n", tok->len, tok->len, tok->str);
			break;
		case VAR:
			printf("VAR     [%d] %.*s \n", tok->len, tok->len, tok->str);
			break;
		case SYMBOL:
			printf("SYMBOL  %.*s \n", tok->len, tok->str);
			break;
		case LPAREN:
		case RPAREN:
			printf("PAREN   %.*s \n", tok->len, tok->str);
			break;
		case COMMA:
			printf("COMMA   %.*s \n", tok->len, tok->str);
			break;
		case END_LEXER:
			printf("END \n");
			break;
		case UNKNOWN:
			printf("UNKNOWN [%d] %.*s \n", tok->len, tok->len, tok->str);
			break;
		default:
			assert(0);
			break;
	}
}


static const char * eat_whitespace(const char * string) {
	int i = 0;
	while (isspace(string[i])) {
		i++;
	}
	return &string[i];
}


static int try_special(const char * str, struct lexer_token * token) {

	switch (str[0]) {
		case '(':
			token->type = LPAREN;
			break;
		case ')':
			token->type = RPAREN;
			break;
		case ',':
			token->type = COMMA;
			break;
		case '\0':
			token->type = END_LEXER;
			break;
		default:
			return 0;
	}
	token->str = str;
	token->len = 1;
	return 1;
}


static int try_symbole(const char * str, struct lexer_token * token) {

	switch (str[0]) {
		case '+':
		case '*':
		case '-':
		case '/':
			token->type = SYMBOL;
			break;
		default:
			return 0;
	}
	token->str = str;
	token->len = 1;
	return 1;
}


static int try_variable(const char * str, struct lexer_token * token) {

	if (!( isalpha(str[0]) || (str[0] == '_') )) { // begin with alpha or '_'
		return 0;
	}

	int i = 1;
	while (isalpha(str[i]) || (str[i] == '_') || isdigit(str[i])) { // following by alpha, '_', digit
		i++;
	}
	token->type = VAR;
	token->str  = str;
	token->len  = i;
	return 1;
}


static int try_number(const char * str, struct lexer_token * token) {

	if (!isdigit(str[0])) {
		return 0;
	}

	int i = 1;
	while (isdigit(str[i])) { // integer part
		i++;
	}

	if (str[i] != '.') { // not a float
		token->type = NUMBER;
		token->str  = str;
		token->len  = i;
		return 1;
	}

	i++; // pass the '.' index
	while (isdigit(str[i])) { // decimal part
		i++;
	}

	token->type = NUMBER;
	token->str = str;
	token->len = i;
	return 1;
}


static void next_token(const char * string, struct lexer_token * token) {

	const char *str = eat_whitespace(string);
	assert(str == eat_whitespace(str));

	if (try_special(str, token)) {
		return;
	}
	if (try_symbole(str, token)) {
		return;
	}
	if (try_variable(str, token)) {
		return;
	}
	if (try_number(str, token)) {
		return;
	}
	// Assume the token is unknown
	token->type = UNKNOWN;
	token->str  = str;
	token->len  = 1;
}


static int count_token(const char * string) {

	struct lexer_token token;
	next_token(string, &token);
	int count = 1; // number of token, at least one (END_LEXER or UNKNOWN)

	while ((token.type != END_LEXER) && (token.type != UNKNOWN)) {
		string = &(token.str[token.len]);
		next_token(string, &token);
		count++;
	}
	return count;
}


struct lexer_result lexer(const char * string) {
	assert(string != NULL);

	int count = count_token(string);

	struct lexer_token * tarray = malloc(sizeof(struct lexer_token) * count);
	CHECK_MALLOC(tarray, "NULL malloc in lexer (lexer.c)\n");

	struct lexer_token token;

	for (int i = 0; i < count; i++) {
		next_token(string, &token);
		tarray[i] = token;
		string = &(token.str[token.len]);
	}

	struct lexer_result res;
	res.tarray = tarray;
	res.token_count = count;
	return res;
}


void print_lexer_result(const struct lexer_result * res) {

	printf("[%d]\n", res->token_count);
	for (int i = 0; i < res->token_count; i++) {
		printf("%3d ", i);
		print_lexer_token(&(res->tarray[i]));
	}
}


void free_lexer_result(struct lexer_result res) {
	free((void *) res.tarray);
}


/*
	TEST
*/


void test_lexer() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else
	printf("LEXER: \n");


	printf(" eat_whitespace\n");
	char *str1 = " a";
	const char *str2 = eat_whitespace(str1);
	assert(str2 == (str1 + 1));

	char *str3 = " 	 a"; // space, tab, stace, 'a'
	const char *str4 = eat_whitespace(str3);
	assert(str4 == (str3 + 3));


	printf(" try_variable\n");
	struct lexer_token t1;

	assert(try_variable("abc", &t1));
	assert(t1.type == VAR);
	assert(t1.len  == 3);

	assert(try_variable("_abc", &t1));
	assert(t1.type == VAR);
	assert(t1.len  == 4);

	assert(try_variable("_Abc2", &t1));
	assert(t1.type == VAR);
	assert(t1.len  == 5);

	assert(!try_variable("123", &t1));
	assert(!try_variable("1bc", &t1));


	printf(" try_number\n");
	struct lexer_token t2;

	assert(try_number("123", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 3);

	assert(try_number("123aa", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 3);

	assert(try_number("123.", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 4);

	assert(try_number("123.45", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 6);

	assert(!try_number(".123", &t2));


	printf(" next_token\n");
	struct lexer_token t3;

	next_token(" 123", &t3);
	assert(t3.type == NUMBER);
	assert(t3.len  == 3);

	next_token("  123.0", &t3);
	assert(t3.type == NUMBER);
	assert(t3.len  == 5);

	next_token(" _Aa1", &t3);
	assert(t3.type == VAR);
	assert(t3.len  == 4);

	next_token(" +", &t3);
	assert(t3.type == SYMBOL);
	assert(t3.len  == 1);

	next_token(" *", &t3);
	assert(t3.type == SYMBOL);
	assert(t3.len  == 1);

	next_token("-", &t3);
	assert(t3.type == SYMBOL);
	assert(t3.len  == 1);

	next_token("   (", &t3);
	assert(t3.type == LPAREN);
	assert(t3.len  == 1);

	next_token(")", &t3);
	assert(t3.type == RPAREN);
	assert(t3.len  == 1);

	next_token("   ,", &t3);
	assert(t3.type == COMMA);
	assert(t3.len  == 1);

	next_token("  ", &t3);
	assert(t3.type == END_LEXER);
	assert(t3.len  == 1);

	next_token(" . ", &t3);
	assert(t3.type == UNKNOWN);
	assert(t3.len  == 1);


	printf(" count_token\n");
	assert(count_token("1 2 3") == 4); // (3 * INT_NUM) + END_LEXER
	assert(count_token("1 . 3") == 2); // stops at . UNKNONW


	printf(" lexer\n");
	struct lexer_result res = lexer("12 +  ( 3.0 * 4 +   18.18)  + (3*4 )");

	assert(res.token_count == 16);
	assert(res.tarray[0].type  == NUMBER);
	assert(res.tarray[1].type  == SYMBOL);
	assert(res.tarray[2].type  == LPAREN);
	assert(res.tarray[3].type  == NUMBER);
	assert(res.tarray[4].type  == SYMBOL);
	assert(res.tarray[5].type  == NUMBER);
	assert(res.tarray[6].type  == SYMBOL);
	assert(res.tarray[7].type  == NUMBER);
	assert(res.tarray[8].type  == RPAREN);
	assert(res.tarray[9].type  == SYMBOL);
	assert(res.tarray[10].type == LPAREN);
	assert(res.tarray[11].type == NUMBER);
	assert(res.tarray[12].type == SYMBOL);
	assert(res.tarray[13].type == NUMBER);
	assert(res.tarray[14].type == RPAREN);
	assert(res.tarray[15].type == END_LEXER);

	free_lexer_result(res);


	printf("done\n");
	#endif
}


