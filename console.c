#include "console.h"


/*
	COSMETIC
*/

static void print_intro_msg() {
	printf(CONSOLE_INTRO_MSG);
}

static void print_prompt() {
	printf(CONSOLE_PROMPT);
}

static int check_leave_cmd(char const * const input) {
	if (strncmp(input, CONSOLE_QUIT_WORD, strlen(CONSOLE_QUIT_WORD)) == 0) {
		log_info("console leave command '" CONSOLE_QUIT_WORD "' found");
		return 1;
	}
	return 0;
}

static void print_leave_msg() {
	printf(CONSOLE_QUIT_MSG);
}

static void print_cursor(char const * input, char const * cursor) {
	// print '^' under the charactere pointed by `cursor` in `input`
	int offset = cursor - input + strlen(CONSOLE_PROMPT);
	// TODO find a format srting that does the loop
	for (int i = 0; i < offset; i++) {
		printf(" ");
	}
	printf("^\n");
}


/*
	PARSER ERROR
*/

static void print_parser_error(char const * input, struct parser_result error) {

	// get the first token
	assert(error.size > 0);
	struct parser_token t1 = error.tarray[0];

	print_cursor(input, t1.str);
	switch (error.type) {

		case ERR_SYM:
			assert(t1.type == ERROR);
			printf("Lexer: Unknown symbol '%.*s' \n", t1.len, t1.str);
			break;

		case ERR_TOKEN:
			assert(t1.type == ERROR);
			printf("Parser: Unknown token '%.*s' \n", t1.len, t1.str);
			break;

		case ERR_PARENT:
			assert((t1.type == LPARENT) || (t1.type == RPARENT));
			printf("Parser: Mismatch parenthesis '%.*s' \n", t1.len, t1.str);
			break;

		case ERR_ARG_SEP: {
			assert(t1.type == ARG_SEP);
			printf("Parser: Misplaces argument separator '%.*s' \n", t1.len, t1.str);
			break;
		}
		case ERR_UNEXPECT:
			printf("Parser: Unexpected token \"");
			print_parser_token(&t1);
			printf("\"\n");
			break;

		case CORRECT:
			assert(0);
			break;
	}
	printf("\n");
}



/*
	CONSOLE
*/

void console() {

	size_t linecap = CONSOLE_DEFAULT_SIZE;
	char *line = malloc(linecap);
	CHECK_MALLOC(line, "NULL malloc in console\n");

	print_intro_msg();

	while (1) {

		print_prompt();
		getline(&line, &linecap, stdin);
		if (check_leave_cmd(line)) {
			break;
		}

		struct parser_result exp = parser(line);
		if (exp.type != CORRECT) {
			print_parser_error(line, exp);
			free_parser_result(exp);
			continue;
		}

		struct number n = eval(exp);
		number_print(&n);
		free_parser_result(exp);
		printf("\n\n");

	}
	print_leave_msg();
	free(line);
	LOG_FREE(line);
}

