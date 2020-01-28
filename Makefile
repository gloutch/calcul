
CC=cc
RELEASE?=no
LOG_LEVEL?=LOG_WARN
ifeq ($(RELEASE),yes)
	# release = remove assert, log quiet
	CFLAGS=-std=c99 -Wall -DNDEBUG
else
	# assert + set log level
	CFLAGS=-std=c99 -Wall -DLOG_USE_COLOR -DLOG_LEVEL=$(LOG_LEVEL)
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

rlwrap: $(EXEC) # just like `run` but with `rlwrap` as wrapper
	rlwrap ./$(EXEC)

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

