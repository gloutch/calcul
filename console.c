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
		return 1;
	}
	return 0;
}


static void print_leave_msg() {
	printf(CONSOLE_QUIT_MSG);
}


// static void print_cursor(char const * input, char const * cursor) {
// 	// print '^' under the charactere pointed by `cursor` in `input`
// 	int offset = cursor - input + strlen(CONSOLE_PROMPT);
// 	// TODO find a format srting that does the loop
// 	for (int i = 0; i < offset; i++) {
// 		printf(" ");
// 	}
// 	printf("^\n");
// }


/*
	PARSER ERROR
*/


// static void print_parser_error(char const * input, struct parser_result error) {

// 	// get the token
// 	struct token error_token;
// 	if (error.RPN_stack != NULL) {
// 		stack_pop(error.RPN_stack, &error_token);
// 		stack_free(error.RPN_stack);
// 	}

// 	switch (error.type) {

// 		case ERR_NULL:
// 			printf("NULL string \\_(^.^)_/ \n");
// 			break;

// 		case ERR_TOKEN:
// 			assert(error_token.type == ERROR);
// 			print_cursor(input, error_token.str);
// 			printf("Unknown token '%.*s'\n", error_token.len, error_token.str);
// 			break;

// 		case ERR_PARENT:
// 			assert((error_token.type == LP) || (error_token.type == RP));
// 			print_cursor(input, error_token.str);
// 			printf("Mismatch parenthesis '%.*s'\n", error_token.len, error_token.str);
// 			break;

// 		case CORRECT:
// 			assert(0);
// 			break;
// 	}
// }






void console() {

	size_t linecap = CONSOLE_DEFAULT_SIZE;
	char *line = malloc(linecap);
	CHECK_MALLOC(line, "NULL malloc in console\n");

	print_intro_msg();

	while (1) {

		print_prompt();
		getline(&line, &linecap, stdin);
		if (check_leave_cmd(line)) {
			free(line);
			break;
		}

		struct lexer_result lex = lexer(line);

		printf("> lexer \n");
		print_lexer_result(&lex);
		
		struct parser_token * token = convert_token(lex);

		printf("> parser\n");
		for (int i = 0; i < (lex.token_count - 1); i++) {
			print_parser_token(token[i]);
		}

		printf("> clean\n");
		free((void *) token);
		free_lexer_result(lex);
		printf("\n");

		// struct parser_result res = parser(line);
		// if (res.type != CORRECT) {
		// 	print_parser_error(line, res);
		// 	continue;
		// }

		// TODO
		// printf("Reverse Polish Notation ");
		// print_RPN_stack(res.RPN_stack);
		// printf("\n");
		// stack_free(res.RPN_stack);

	}
	print_leave_msg();
}

