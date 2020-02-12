#include "error.h" 


// global variable for error management

enum error_type   error = NO_ERROR;

struct error_expr {
	const char * character;
	const char * word;
	int length;
} err_data;




int error_get() {
	return (int) error;
}

void error_set(enum error_type err, const char * cursor, const char * word, int len) {
	error = err;
	err_data.character = cursor;
	err_data.word      = word;
	err_data.length    = len;
}

void error_reset() {
	error_set(NO_ERROR, NULL, NULL, 0);
}


static void whitespace(int space_len) {
	for (int i = 0; i < space_len; i++) {
		printf(" ");
	}
}

static int word(int id_word, int len) {
	whitespace(id_word);
	for (int i = 0; i < len; i++) {
		printf("~");
	}
	return id_word + len;
}

static int cursor(int id_cursor) {
	whitespace(id_cursor);
	printf("^");
	return id_cursor + 1;
}

static void underline_all(int idc, int idw, int len) {

	if (idc < idw) {
		idw -= cursor(idc);
		word(idw, len);
		return;
	}
	if (idw + len < idc) {
		idc -= word(idw, len);
		cursor(idc);
		return;
	}
	word(idw, idc - idw);
	cursor(0);
	word(idc + 1, idw + len - idc - 1);
}

void error_underline(const char * input) {

	if (err_data.character != NULL) {
		if (err_data.word != NULL) {
			underline_all(err_data.character - input, err_data.word - input, err_data.length);
			return;
		}
		cursor(err_data.character - input);
		return;
	} 
	if (err_data.word != NULL) {
		word(err_data.word - input, err_data.length);
	}
}


void error_message() {

	switch (error) {
		case NO_ERROR:
			printf("NO error has occured");
			break;
		// lexer
		case UNKNOWN_SYM:
			printf("Lexer: Unknown symbol '%.*s'", 1, err_data.character);
			break;
		case WRONG_BASE:
			printf("Lexer: Number %c-based contains digit '%c' (wrong base)", *err_data.word, *err_data.character);
			break;
		// parser
		case UNKNOWN_TOK:
			printf("Parser: Unknown token '%.*s'", err_data.length, err_data.word);
			break;
		case MIS_PARENT:
			printf("Syntax: Mismatch parenthesis %.*s", 1, err_data.character);
			break;
		case UNEXP_TOK:
			printf("Syntax: Unexpected token '%.*s'", err_data.length, err_data.word);
			break;
		case MIS_ARG_SEP:
			printf("Syntax: Misplace comma");
			if (err_data.word != NULL) {
				printf(" in function '%.*s'", err_data.length, err_data.word);
			}
			break;
		// eval
		case UNMANAGED:
			printf("Eval: Unmanaged feature");
			if (err_data.word != NULL) {
				printf(" '%.*s' ", err_data.length, err_data.word);
			} else if (err_data.character) {
				printf(" '%c' ", *err_data.character);
			}
			printf("(yet)");
			break;
		case POW_BIG:
			printf("Eval: exponent too big, it must fit in a long integer");
			break;
		case POW_NEG:
			printf("Eval: negative exponent isn't allowed");
			break;
	}
}
