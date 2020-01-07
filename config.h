#ifndef CONFIG_H
#define CONFIG_H

/* 
	This config file aimed to centralize some configurations, macros
*/

#include <stdio.h>


#define CONSOLE_PROMPT ">>> "

// my way to manage malloc error
// This macro whould be called right after every malloc
#define CHECK_MALLOC(ptr, msg) {	\
	if ((ptr) == NULL) {			\
		perror((msg));				\
		exit(1);					\
	}								\
}



#endif // CONFIG_H