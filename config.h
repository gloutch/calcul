#ifndef CONFIG_H
#define CONFIG_H

/* 
	This config file aimed to centralize some configurations, macros
*/

#include <stdio.h>


#define CONSOLE_PROMPT ">>> "
#define CONSOLE_DEFAULT_SIZE 256
#define CONSOLE_QUIT_WORD "q"
#define CONSOLE_INTRO_MSG "Hi!\nJust type '"CONSOLE_QUIT_WORD"' to leave the program\n"
#define CONSOLE_QUIT_MSG  "Bye!\n"


// This macro whould be called right after every malloc
#define CHECK_MALLOC(ptr, msg) {	\
	if ((ptr) == NULL) {			\
		perror((msg));				\
		exit(1);					\
	}								\
}


#endif // CONFIG_H