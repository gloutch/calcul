
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

SRC=$(wildcard src/*.c)
SRC_EXEC=$(filter-out src/test.c, $(SRC))
SRC_TEST=$(filter-out src/main.c, $(SRC))
OBJ_EXEC=$(SRC_EXEC:.c=.o)
OBJ_TEST=$(SRC_TEST:.c=.o)


$(EXEC): $(OBJ_EXEC)
	$(CC) $(CFLAGS) $^ -o $@

run: $(EXEC)
	rlwrap ./$(EXEC)

$(TEST): $(OBJ_TEST)
	$(CC) $(CFLAGS) $^ -o $@

tst: $(TEST)
	./$(TEST)

%.o: %.c %.h config.h
	$(CC) $(CFLAGS) -c $<
	

.PHONY: clean mrproper

clean:
	rm -rvf src/*.o

mrproper: clean
	rm -rvf $(EXEC) $(TEST)
