#include "stack.h"


struct stack {
	int elem_size;
	int max_elem;
	stack_copy_elem copy;
	void * start;
	void * current; // pointer to the next space to push
};


struct stack * stack_malloc(int elem_size, int max_elem, stack_copy_elem copy) {

	// store the stack on the heap {struct stack, [elem_size * max_elem]}
	struct stack * s = malloc(sizeof(struct stack) + (elem_size * max_elem));
	CHECK_MALLOC(s, "stack_malloc");

	// init the stack
	s->elem_size = elem_size;
	s->max_elem  = max_elem;
	s->copy      = copy;
	s->start     = (void *) &(s[1]); // the memory juste after the struct stack
	s->current   = s->start;

	return s;
}


// basic operation
void stack_push(struct stack * s, const void * elem) {
	assert(elem != NULL);
	assert(!stack_full(s));
	s->copy(elem, s->current);
	s->current += s->elem_size;
}

void stack_pop(struct stack * s, void * dst) {
	assert(dst != NULL);
	assert(!stack_empty(s));
	s->current -= s->elem_size;
	s->copy(s->current, dst);
}

void * stack_peek(const struct stack * s) {
	assert(!stack_empty(s));
	return (s->current - s->elem_size);
}

// yes, that's not a basic operation, 
// but that's so easy to write and so much efficient that without the struct
void stack_reverse(struct stack * s) {

	int count = stack_size(s);
	char tmp[s->elem_size]; // tmp memory to store one elem using copy

	for (int i = 0; i < (count / 2); i++) {

		void * i1 = s->start + (s->elem_size * i);
		void * i2 = s->start + (s->elem_size * (count - 1 - i));
		s->copy(i1, (void *) &tmp);
		s->copy(i2, i1);
		s->copy((void *) &tmp, i2);
	}
}


// about size
int stack_empty(const struct stack * s) {
	return (s->start == s->current);
}

int stack_full(const struct stack * s) {
	return ((s->current - s->start) == (s->elem_size * s->max_elem));
}

int stack_size(const struct stack * s) {
	return ((s->current - s->start) / s->elem_size);
}


// print
void stack_print(const struct stack * s, stack_print_elem print) {
	int count = stack_size(s);
	for (int i = count - 1; i >= 0; i--) {
		print(s->start + (s->elem_size * i));
	}
}


// free
void stack_free(struct stack * s) {
	s->copy    = NULL;
	s->start   = NULL;
	s->current = NULL;
	LOG_FREE(s);
	free(s);
}


/*
	TEST SECTION
*/


static void copy_int(const int * const src, int * const dst) {
	*dst = *src;
}

static void print_int(const int * const elem) {
	printf("%d.", *elem);
}

void test_stack() {

	#ifdef NDEBUG
	printf("COMPILE ERROR: test should NOT be compile with '-DNDEBUG'\n\n");
	exit(1);
	#else

	printf("STACK: \n");
	int count = 7;

	struct stack * const s = stack_malloc(sizeof(int), count, (stack_copy_elem) copy_int);

	assert(stack_empty(s));
	assert(stack_size(s) == 0);
	assert(!stack_full(s));

	// fill
	for (int i = 0; i < count; i++) {

		assert(stack_size(s) == i);
		stack_push(s, &i);
		assert(!stack_empty(s));
		assert(stack_size(s) == i + 1);

		// peek
		int *ptr = stack_peek(s);
		assert(*ptr == i);
	}

	// stack is full
	assert(stack_full(s));
	assert(stack_size(s) == count);

	printf(" ");
	stack_print(s, (stack_print_elem) print_int); // print
	stack_reverse(s); // reverse
	printf(" - ");
	stack_print(s, (stack_print_elem) print_int); // print again

	// pop all
	for (int i = 0; i < count; i++) {

		assert(stack_size(s) == count - i);
		int tmp;
		stack_pop(s, &tmp);
		assert(tmp == i);

		assert(!stack_full(s));
	}

	stack_free(s);

	printf("\ndone\n\n");
	#endif
}



