#ifndef CONFIG_H
#define CONFIG_H

/* 
	This config file aimed to centralize some configurations, macros
*/

#include <stdio.h>


#define CONSOLE_PROMPT ">>> "
#define CONSOLE_DEFAULT_SIZE 256
#define CONSOLE_QUIT_WORD "q"
#define CONSOLE_INTRO_MSG "\nHi!\nJust type '"CONSOLE_QUIT_WORD"' to leave the program\n"
#define CONSOLE_QUIT_MSG  "Bye!\n"


// This macro whould be called right after every malloc
#define CHECK_MALLOC(ptr, msg) {						\
	if ((ptr) == NULL) {								\
		perror("NULL malloc: " msg "\n");				\
		exit(1);										\
	}													\
}


/*	Macro to check that compiler didn't remove `assert`

#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
#else
	printf("test: ");
	printf("done\n");
#endif

*/


#endif // CONFIG_H

