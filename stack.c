#include "stack.h"


struct stack {
	int elem_size;
	int max_elem;
	func_cpy copy;
	void * start;
	void * current; // pointer to the next space to push
};


struct stack * const stack_malloc(int elem_size, int max_elem, func_cpy cpy_elem) {

	// store the stack on the heap {struct stack, [elem_size * max_elem]}
	struct stack * s = malloc(sizeof(struct stack) + (elem_size * max_elem));
	CHECK_MALLOC(s, "NULL malloc in stack_malloc\n");

	// init the stack
	s->elem_size = elem_size;
	s->max_elem  = max_elem;
	s->copy      = cpy_elem;
	s->start     = (void *) &(s[1]); // the memory juste after the struct stack
	s->current   = s->start;

	return s;
}


// basic operation
void stack_push(struct stack * const s, void const * const elem) {
	s->copy(elem, s->current);
	s->current += s->elem_size;
}

void stack_pop(struct stack * const s, void * const dst) {
	s->current -= s->elem_size;
	s->copy(s->current, dst);
}


// about size
int stack_empty(struct stack const * const s) {
	return (s->start == s->current);
}

int stack_full(struct stack const * const s) {
	return ((s->current - s->start) == (s->elem_size * s->max_elem));
}

int stack_size(struct stack const * const s) {
	return ((s->current - s->start) / s->elem_size);
}


// print
void stack_print(struct stack const * const s, func_print print) {
	int count = stack_size(s);
	for (int i = 0; i < count; i++) {
		print(s->start + (s->elem_size * i));
	}
}


// free
void stack_free(struct stack * const s) {
	s->copy    = NULL;
	s->start   = NULL;
	s->current = NULL;
	free(s);
}


/*
	TEST SECTION
*/


// cast (void *) to (int *), then copy src to dst unsing '='
static void cpy_int(void const * const src, void * const dst) {
	*((int *) dst) = *((int *) src);
}

static void print_int(void const * const elem) {
	printf("%d ", *((int *) elem));
}

void test_stack() {

	printf("TEST stack: ");
	int count = 18;

	struct stack * const s = stack_malloc(sizeof(int), count, cpy_int);

	assert(stack_empty(s));
	assert(stack_size(s) == 0);
	assert(!stack_full(s));

	// fill
	for (int i = 0; i < count; i++) {

		assert(stack_size(s) == i);
		stack_push(s, &i);
		assert(!stack_empty(s));
		assert(stack_size(s) == i + 1);
	}

	// stack is full
	assert(stack_full(s));
	assert(stack_size(s) == count);

	// print
	// printf("\nThe stack: ");
	// stack_print(s, print_int);
	// printf("\n");

	// pop all
	for (int i = count - 1; 0 <= i; i--) {

		int tmp;
		stack_pop(s, &tmp);
		assert(stack_size(s) == i);
		assert(!stack_full(s));
		assert(tmp == i);
	}

	stack_free(s);

	printf("done\n");
}



