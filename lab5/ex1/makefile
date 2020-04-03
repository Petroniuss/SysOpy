SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: interpreter
	echo "Compiled"

test: interpreter
	./interpreter "test/sh-1.txt" -pipe

interpreter: $(OBJ)
	$(CC) -o interpreter interpreter.o utils_lib.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o interpreter

.PHONY:
	clean run 