#ifndef CONFIG_H
#define CONFIG_H

/* 
	This config file aimed to centralize some configurations, macros
*/

#include <stdio.h>
#include "log.h"


#define CONSOLE_PROMPT ">>> "
#define CONSOLE_LINE_SIZE 256
#define CONSOLE_QUIT_WORD "q"
#define CONSOLE_INTRO_MSG "\nHi!\nJust type '"CONSOLE_QUIT_WORD"' to leave the program\n"
#define CONSOLE_QUIT_MSG  "Bye!\n"


// This macro should be called right after every malloc
#define CHECK_MALLOC(ptr, msg) {							\
	log_debug("malloc %p:%s", (ptr), (msg));				\
	if ((ptr) == NULL) {									\
		log_fatal("NULL malloc %p:%s", (ptr), (msg));		\
		perror("NULL malloc: " msg "\n");					\
		exit(1);											\
	}														\
}

// And this macro should be called before free memory
#define LOG_FREE(ptr) log_debug("free   %p", (ptr))

// default log setting
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_WARN
#endif


/*	Macro to check that compiler didn't remove `assert`

#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
#else
	printf("test: ");
	printf("done\n\n");
#endif

*/


#endif // CONFIG_H

