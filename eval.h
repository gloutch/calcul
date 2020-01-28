#ifndef EVAL_H
#define EVAL_H

#include <assert.h>
#include <stdlib.h>
#include "log.h"
#include "number.h"
#include "shunting_yard.h"
#include "stack.h"


struct number eval(struct parser_result exp);


#endif // EVAL_H