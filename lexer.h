#ifndef LEXER_H
#define LEXER_H


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"


/*
	The lexer return a `struct lexer_result` which is an malloced array of token.
	The lexer stops until it finds a END_LEXER or UNKNOWN token (last elem of the array)
*/

enum lexer_token_type {
	// num
	NUMBER,
	// var
	VAR,
	// symbol
	PLUS,
	ASTERISK,
	MINUS,
	// parenthesis
	LPAREN,
	RPAREN,
	// ,
	COMMA,
	// everthing else
	UNKNOWN,
	// end of token
	END_LEXER
};

struct lexer_token {
	enum lexer_token_type type;
	const char *str;
	int len;
};

struct lexer_result {
	const struct lexer_token * tarray;
	int token_count;
};


struct lexer_result lexer(const char * string);

void print_lexer_token(const struct lexer_token * const tok);

void print_lexer_result(const struct lexer_result * res);

void free_lexer_result(struct lexer_result res);

void test_lexer();


#endif // LEXER_H