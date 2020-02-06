#ifndef EVAL_H
#define EVAL_H

#include <assert.h>
#include <stdlib.h>
#include "error.h"
#include "log.h"
#include "number.h"
#include "shunting_yard.h"
#include "stack.h"


struct number eval(const struct expr e);


#endif // EVAL_H