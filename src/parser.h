#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "error.h"
#include "lexer.h" // for test
#include "log.h"
#include "stack.h"
#include "token.h"


struct expr lexer_to_parser(const struct expr * e);

// return 1 if syntax is fine, 1 othewise
int parser_check_syntax(const struct expr e);


void test_parser();


#endif // PARSER_H
