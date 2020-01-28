#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "lexer.h"
#include "stack.h"
#include "string.h"

/*

The utimate result of the parser is a `struct parser_result`

If everything went well,
`type == CORRECT`
`tarray` is an array of `parer_token`

Otherwise,
`type` describes the error
`tarray` contains a token which explicit the error

*/

enum parser_token_type {
	NUM_OPERAND,
	VAR_OPERAND,
	FUNC_NAME,
	// binary operator
	PLUS,
	MINUS,
	ASTERISK,
	// unary operator
	UNARY_PLUS,
	UNARY_MINUS,
	// specials
	LPARENT,
	RPARENT,
	ARG_SEP,
	ERROR,
};

struct parser_token {
	enum parser_token_type type;
	const char * str;
	int len;
};

void print_parser_token(const struct parser_token * const token);

void copy_parser_token(const struct parser_token * const src, struct parser_token * const dst);


enum result_type {
	CORRECT,		// everything turned great :D
	ERR_SYM,		// lexer unknown symbol  
	ERR_TOKEN,		// parser unknown token
	ERR_PARENT,		// missing parenthesis
	ERR_ARG_SEP,	// missplaced comma
	ERR_UNEXPECT,	// unexpected token
};

struct parser_result {
	enum result_type type;
	struct parser_token * tarray;
	int size;
};

struct parser_result parser(char const * string);

void free_parser_result(struct parser_result res);


void test_parser();


#endif // PARSER_H
