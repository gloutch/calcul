#include <stdio.h>

#include "console.h"
#include "log.h"


int main(int argc, char *argv[]) {

	#ifdef NDEBUG // release
	log_set_quiet(1);
	#else // debug
	log_set_quiet(0);
	log_set_level(LOG_LEVEL);
	#endif // NDEBUG

	console();
	
	return 0;
}