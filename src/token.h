#ifndef TOKEN_H
#define TOKEN_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"


enum token_type {
	// LEXER only
	NUMBER,
	NAME,
	SYMBOL, // operator
	END,

	// PARSER only
	NUM_OPERAND,
	VAR_OPERAND,
	FUNC_NAME,
	PLUS, // binary operator
	MINUS,
	ASTERISK,
	POW,
	UNARY_PLUS, // unary operator
	UNARY_MINUS,
	UNKNOWN,
	
	// specials (common)
	LPARENT,
	RPARENT,
	ARG_SEP,
};

struct token {
	enum token_type type;
	const char * str;
	int len;
};

void token_print(const struct token * const t);

void token_copy(const struct token * const src, struct token * const dst);



struct expr {
	struct token * list;
	int len;
};

struct expr token_expr(int len);

void token_print_expr(const struct expr * const e);

void token_free_expr(struct expr * e);



#endif // TOKEN_H