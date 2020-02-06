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
		log_debug("console leave command '" CONSOLE_QUIT_WORD "' found");
		return 1;
	}
	return 0;
}

static void print_leave_msg() {
	printf(CONSOLE_QUIT_MSG);
}

static void print_error(const char * input_line) {

	int n = strlen(CONSOLE_PROMPT);
	for (int i = 0; i < n; i++) {
		printf(" ");
	}
	
	error_underline(input_line);
	printf("\n");
	error_message();
	printf("\n\n");
}


/*
	CONSOLE
*/

void console() {

	size_t linecap = CONSOLE_LINE_SIZE;
	char *line = malloc(linecap);
	CHECK_MALLOC(line, "line in console\n");

	print_intro_msg();

	while (1) {

		print_prompt();
		if (getline(&line, &linecap, stdin) < 0) {
			log_fatal("getline error");
			break;
		}
		if (check_leave_cmd(line)) {
			break;
		}

		// lexer
		struct expr e1 = lexer(line);
		if (error_get()) {
			print_error(line);
			token_free_expr(&e1);
			continue;
		}
		if (e1.len == 0) {
			token_free_expr(&e1);
			continue;
		}

		// parser
		struct expr e2 = lexer_to_parser(&e1);
		token_free_expr(&e1);
		if (error_get()) {
			print_error(line);
			token_free_expr(&e2);
			continue;
		}
		if (parser_check_syntax(e2)) {
			print_error(line);
			token_free_expr(&e2);
			continue;
		}

		// eval
		struct number result = eval(e2);
		token_free_expr(&e2);
		if (error_get()) {
			print_error(line);
			number_free(result);
			continue;
		}

		number_print(&result);
		number_free(result);
		printf("\n\n");
	}
	print_leave_msg();
	LOG_FREE(line);
	free(line);
}

