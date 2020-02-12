#ifndef ERROR_H
#define ERROR_H

#include <assert.h>
#include "log.h"


enum error_type {
	NO_ERROR = 0,
	// lexer
	UNKNOWN_SYM,
 	WRONG_BASE,
	// parser
	UNKNOWN_TOK,
	MIS_PARENT,
	UNEXP_TOK,
	MIS_ARG_SEP,
	// eval
	UNMANAGED,
	POW_BIG,
	POW_NEG,
};


int error_get();

void error_set(enum error_type err, const char * cursor, const char * word, int len);

void error_reset();

void error_underline(const char * input_expr);

void error_message();



#endif // ERROR_H