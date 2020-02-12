#include "lexer.h"



static const char * eat_whitespace(const char * string) {
	int i = 0;
	while (isspace(string[i])) {
		i++;
	}
	return &string[i];
}


static int try_special(const char * str, struct token * t) {

	switch (str[0]) {
		case '(':
			t->type = LPARENT;
			break;
		case ')':
			t->type = RPARENT;
			break;
		case ',':
			t->type = ARG_SEP;
			break;
		case '\0':
			t->type = END;
			break;
		default:
			return 0;
	}
	t->str = str;
	t->len = 1;
	return 1;
}

static int try_symbol(const char * str, struct token * t) {

	switch (str[0]) {
		case '+':
		case '-':
		case '*':
		case '^':
			t->type = SYMBOL;
			break;
		default:
			return 0;
	}
	t->str = str;
	t->len = 1;
	return 1;
}

static int try_name(const char * str, struct token * t) {

	if (!( isalpha(str[0]) || (str[0] == '_') )) { // begin with alpha or '_'
		return 0;
	}

	int i = 1;
	while (isalpha(str[i]) || (str[i] == '_') || isdigit(str[i])) { // following by alpha, '_', digit
		i++;
	}
	t->type = NAME;
	t->str  = str;
	t->len  = i;
	return 1;
}

/*
	About NUMBER
*/

// return 1 if `c` is a valid digit, 0 otherwise
typedef int (* check_digit)(int c);

static int eat_number(const char * str, check_digit digit, struct token * t) {
	if (!digit(str[0])) {
		return 0;
	}

	int i = 1;
	while (digit(str[i])) { // integer part
		i++;
	}

	if (str[i] != '.') { // not a float
		t->type = NUMBER;
		t->str  = str;
		t->len  = i;
		return 1;
	}
	i++; // skip the '.' index
	
	while (digit(str[i])) { // decimal part
		i++;
	}

	t->type = NUMBER;
	t->str = str;
	t->len = i;
	return 1;
}

static int base = 0;

static int isdigit_base(int c) {
	if (isdigit(c)) {
		if ((c - '0') < base) {
			return 1;	
		}
		log_warn("Wrong digit base '%c' >= %d", c, base);
	}
	return 0;
}

static int try_number(const char * str, struct token * t) {

	if (!isdigit(str[0])) {
		return 0;
	}
	if (str[1] != 'x') { // no prefix, then assume it's a decimal number
		return eat_number(str, isdigit, t);
	}
	
	// retrieve the base of the prefix
	base = str[0] - '0';
	assert((0 <= base) && (base < 10));
	const char * num_core = str + 2;

	if (base == 0) { // hexadicimal
		if (eat_number(num_core, isxdigit, t)) {
			t->str = str;
			t->len = t->len + 2;
			log_debug("Find hex num '%.*s'", t->len, t->str);
			return 1;
		}
		return 0;
	}

	eat_number(num_core, isdigit_base, t);
	// check next char to see if it stops because of base
	if (isxdigit(num_core[t->len])) {
		error_set(WRONG_BASE, &str[2 + t->len], str, 2);
		return 0;
	}
	t->str = str;
	t->len = t->len + 2;
	log_debug("Find base %d num '%.*s'", base, t->len, t->str);
	return 1;
}


// get token from string
static void next_token(const char * string, struct token * t) {

	const char *str = eat_whitespace(string);
	assert(str == eat_whitespace(str));

	if (try_special(str, t)) {
		return;
	}
	if (try_symbol(str, t)) {
		return;
	}
	if (try_name(str, t)) {
		return;
	}
	if (try_number(str, t)) {
		return;
	}
	// Assume the token is unknown
	t->type = END;
	if (!error_get()) {
		error_set(UNKNOWN_SYM, str, NULL, 0);
	}
}


static int count_token(const char * string) {

	struct token token;
	next_token(string, &token);
	int count = 1; // number of token, at least one END

	while (token.type != END) {
		string = &(token.str[token.len]);
		next_token(string, &token);
		count++;
	}
	assert(count > 0);
	return count - 1; // without END
}

struct expr lexer(const char * string) {
	error_reset();

	int count = count_token(string);
	if (error_get()) {
		return token_expr(0);
	}
	log_debug("Lexer %d token found", count);
	struct expr e = token_expr(count);

	struct token tmp;
	for (int i = 0; i < e.len; i++) {
		next_token(string, &tmp);
		string = &(tmp.str[tmp.len]);
		e.list[i] = tmp;
	}

	log_debug("expr, token[] in %p", e.list);
	return e;
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


	printf(" try_name\n");
	struct token t1;

	assert(try_name("abc", &t1));
	assert(t1.type == NAME);
	assert(t1.len  == 3);

	assert(try_name("_abc", &t1));
	assert(t1.type == NAME);
	assert(t1.len  == 4);

	assert(try_name("_Abc2", &t1));
	assert(t1.type == NAME);
	assert(t1.len  == 5);

	assert(!try_name("123", &t1));
	assert(!try_name("1bc", &t1));


	printf(" try_number\n");
	struct token t2;

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

	assert(try_number("0x1234A", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 7);

	assert(try_number("2x010101", &t2));
	assert(t2.type == NUMBER);
	assert(t2.len  == 8);

	assert(!try_number("2x020101", &t2));
	assert(error_get() == WRONG_BASE);
	error_reset();

	assert(try_number("3x0201.01", &t2));
	assert(error_get() == NO_ERROR);
	assert(t2.type == NUMBER);
	assert(t2.len  == 9);


	printf(" next_token\n");
	struct token t3;

	next_token(" 123", &t3);
	assert(t3.type == NUMBER);
	assert(t3.len  == 3);

	next_token("  123.0", &t3);
	assert(t3.type == NUMBER);
	assert(t3.len  == 5);

	next_token(" _Aa1", &t3);
	assert(t3.type == NAME);
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
	assert(t3.type == LPARENT);
	assert(t3.len  == 1);

	next_token(")", &t3);
	assert(t3.type == RPARENT);
	assert(t3.len  == 1);

	next_token("   ,", &t3);
	assert(t3.type == ARG_SEP);
	assert(t3.len  == 1);

	next_token("  ", &t3);
	assert(t3.type == END);
	assert(t3.len  == 1);


	printf(" count_token\n");
	assert(count_token("1 2 3") == 3);
	assert(error_get() == NO_ERROR);
	assert(count_token("1 § 3") == 1); // stops at § UNKNONW
	assert(error_get() == UNKNOWN_SYM);


	printf(" lexer\n");
	struct expr res = lexer("12 +  ( 3.0 * 4 +   18.18)  + (3*4 )");

	assert(error_get() == NO_ERROR);
	assert(res.len == 15);
	assert(res.list[0].type  == NUMBER);
	assert(res.list[1].type  == SYMBOL);
	assert(res.list[2].type  == LPARENT);
	assert(res.list[3].type  == NUMBER);
	assert(res.list[4].type  == SYMBOL);
	assert(res.list[5].type  == NUMBER);
	assert(res.list[6].type  == SYMBOL);
	assert(res.list[7].type  == NUMBER);
	assert(res.list[8].type  == RPARENT);
	assert(res.list[9].type  == SYMBOL);
	assert(res.list[10].type == LPARENT);
	assert(res.list[11].type == NUMBER);
	assert(res.list[12].type == SYMBOL);
	assert(res.list[13].type == NUMBER);
	assert(res.list[14].type == RPARENT);

	token_free_expr(&res);


	printf("done\n\n");
	#endif
}


