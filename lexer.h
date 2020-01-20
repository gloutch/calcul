#ifndef LEXER_H
#define LEXER_H


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"


/*
	The `lexer` returns a `struct lexer_result` which is an malloced array of token.

The lexer stops until it finds an UNKNOWN token or it reachs the end of the string END_LEXER
Thus, tarray[size] is either UNKNOWN or END_LEXER and from O to (size -1) it a regular token array

*/

enum lexer_token_type {
	NUMBER,
	NAME,
	SYMBOL, // (operator)
	// specials
	LPAREN,
	RPAREN,
	COMMA,
	END_LEXER,
	// everthing else
	UNKNOWN,
};

struct lexer_token {
	enum lexer_token_type type;
	const char * str;
	int len;
};

void print_lexer_token(const struct lexer_token * const tok);



struct lexer_result {
	const struct lexer_token * tarray;
	int size;
};

struct lexer_result lexer(const char * string);

void print_lexer_result(const struct lexer_result * res);

void free_lexer_result(struct lexer_result res);


void test_lexer();


#endif // LEXER_H