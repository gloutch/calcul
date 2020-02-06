#ifndef LEXER_H
#define LEXER_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "error.h"
#include "log.h"
#include "token.h"


struct expr lexer(const char * string);

void test_lexer();


#endif // LEXER_H