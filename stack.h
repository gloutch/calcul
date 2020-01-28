#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"


// create stack
typedef void (*stack_copy_elem)(const void * const src, void * const dst);

struct stack * stack_malloc(int elem_size, int max_elem, stack_copy_elem copy);


// basic operation
void stack_push(struct stack * s, const void * elem);

void stack_pop(struct stack * s, void * dst);

void * stack_peek(const struct stack * s);

void stack_reverse(struct stack * s);


// about size
int stack_empty(const struct stack * s);

int stack_full(const struct stack * s);

int stack_size(const struct stack * s);


// print
typedef void (*stack_print_elem)(const void * const elem);

void stack_print(const struct stack * s, stack_print_elem print); // top to bottom


// free
void stack_free(struct stack * s);


// test
void test_stack();


#endif // STACK_H