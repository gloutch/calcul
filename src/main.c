#include <stdio.h>

#include "console.h"
#include "config.h"
#include "log.h"


int main(int argc, char *argv[]) {

	#ifdef NDEBUG // release
	log_set_quiet(1);
	#endif
	
	#ifdef LOG_LEVEL
	log_set_quiet(0);
	log_set_level(LOG_LEVEL);
	#endif

	console();
	
	return 0;
}