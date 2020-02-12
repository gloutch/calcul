#include "token.h"

void token_print(const struct token * const t) {

	switch(t->type) {

		// LEXER
		case NUMBER:
			printf("NUMBER    [%d] %.*s", t->len, t->len, t->str);
			return;
		case NAME:
			printf("NAME      [%d] %.*s", t->len, t->len, t->str);
			return;
		case SYMBOL:
			printf("SYMBOL    %.*s", t->len, t->str);
			return;

		// PARSER
		case NUM_OPERAND:
		case VAR_OPERAND:
			printf("OPERAND   [%d] %.*s", t->len, t->len, t->str);
			return;
		case FUNC_NAME:
			printf("FUNCTION  [%d] %.*s", t->len, t->len, t->str);
			return;
		case PLUS:
		case MINUS:
		case ASTERISK:
		case POW:
			printf("BINARY OP %.*s", t->len, t->str);
			return;
		case UNARY_PLUS:
		case UNARY_MINUS:
			printf("UNARY OP  %.*s", t->len, t->str);
			return;

		// SPECIAL
		case LPARENT:
		case RPARENT:
			printf("PARENT    %.*s", t->len, t->str);
			return;
		case ARG_SEP:
			printf("ARG_SEP   %.*s", t->len, t->str);
			return;
		case UNKNOWN:
			printf("UNKNOWN   %.*s", t->len, t->str);
			return;
		case END:
			printf("END");
			return;
	}
}

void token_copy(const struct token * const src, struct token * const dst) {
	*dst = *src;
}


struct expr token_expr(int len) {
	assert(len >= 0);

	struct expr e;
	if (len == 0) {
		e.list = NULL;
		e.len  = 0;
		return e;
	}

	e.list = (struct token *) malloc(sizeof(struct token) * len);
	CHECK_MALLOC(e.list, "token_expr");
	e.len = len;
	return e;
}

void token_print_expr(const struct expr * const e) {

	printf("expr [%p] \n", e->list);
	for (int i = 0; i < e->len; i++) {
		printf("%3d ", i);
		token_print(&(e->list[i]));
		printf("\n");
	}
}

void token_free_expr(struct expr * e) {
	e->len = 0;
	if (e->list != NULL) {
		LOG_FREE(e->list);
		free(e->list);
		e->list = NULL;
	}
}