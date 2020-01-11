
CC=cc

RELEASE?=no
ifeq ($(RELEASE),yes)
	# release = remove assert
	CFLAGS=-std=c99 -Wall -DNDEBUG
else
	CFLAGS=-std=c99 -Wall
endif


EXEC=main
TEST=test

SRC=$(wildcard *.c)
SRC_EXEC=$(filter-out test.c, $(SRC))
SRC_TEST=$(filter-out main.c, $(SRC))
OBJ_EXEC=$(SRC_EXEC:.c=.o)
OBJ_TEST=$(SRC_TEST:.c=.o)


compile:$(EXEC)

run: $(EXEC)
	./$(EXEC)

$(EXEC): $(OBJ_EXEC)
	$(CC) $(CFLAGS) $^ -o $@

tst: $(TEST)
	./$(TEST)

$(TEST): $(OBJ_TEST)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h config.h
	$(CC) $(CFLAGS) -c $<



.PHONY: clean clear mrproper

clean:
	rm -rvf *.o

clear: clean
	@ clear

mrproper: clean
	rm -rvf $(EXEC) $(TEST)