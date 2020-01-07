#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "stack.h"


#define PROMPT ">>> "


void prompt() {
	printf(PROMPT);
}

int prompt_len() {
	return strlen(PROMPT);
}


void display_msg(char *str, int cursor) {

	int shift = prompt_len();

	for (int i = 0; i < cursor + shift; i++) {
		printf(" ");
	}
	printf("^\n%s\n", str);
}

void console() {

	char *buffer;

	while(1) {

		prompt();
		getline(&buffer, NULL, stdin);
		// display_msg(buffer, 0);

		lexer(buffer);
	}
}




int main(int argc, char *argv[]) {

	printf("Hello Word!\n");
	// console();

	test_lexer();
	test_stack();

	return 0;
}