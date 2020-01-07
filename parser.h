#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"


enum token_type {
	INT_NUM,	// number
	PLUS,		// operator
	MULT,
	LP,			// parenthesis	
	RP,
	END_TOKEN,
	ERROR
};

struct token {
	enum token_type type;
	char const *str;
	int len;
};


struct token *lexer(char const *string);

void print_token(struct token const * const tok);

void test_lexer();


#endif // PARSER_H