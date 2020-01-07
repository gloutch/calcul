#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdlib.h>
#include "config.h"


// create stack
typedef void (*stack_copy_elem)(void const * const src, void * const dst);

struct stack * const stack_malloc(int elem_size, int max_elem, stack_copy_elem copy);


// basic operation
void stack_push(struct stack * const s, void const * const elem);

void stack_pop(struct stack * const s, void * const dst);


// about size
int stack_empty(struct stack const * const s);

int stack_full(struct stack const * const s);

int stack_size(struct stack const * const s);


// print
typedef void (*stack_print_elem)(void const * const elem);

void stack_print(struct stack const * const s, stack_print_elem print);


// free
void stack_free(struct stack * const s);


// test
void test_stack();


#endif // STACK_H