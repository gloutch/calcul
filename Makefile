
CC=cc
CFLAGS=-std=c99 -Wall
NODEBUG=-DNDEBUG
EXEC=main


all: $(EXEC)

run: all
	@./$(EXEC)

main: main.c parser.o stack.o console.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<


.PHONY: clean clear

clean:
	rm -rvf $(EXEC)
	rm -rvf *.o

clear: clean
	@ clear
