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

	size_t linecap = 80;
	char *line = malloc(linecap);

	while(1) {

		prompt();
		getline(&line, &linecap, stdin);
		// display_msg(line, 0);

		parser(line);
	}
}




int main(int argc, char *argv[]) {

	printf("Hello Word!\n");
	// console();

	test_parser();
	test_stack();

	return 0;
}