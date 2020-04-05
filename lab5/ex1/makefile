SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: interpreter
	echo "Compiled"

# Example using prepared file with commands (executing using exec/fork/pipe)
test: interpreter
	./interpreter "test/sh-1.txt" 

# Another example interpreting different file 
another: interpreter
	./interpreter "test/sh-2.txt" 

interpreter: $(OBJ)
	$(CC) -o interpreter interpreter.o utils_lib.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o interpreter

.PHONY:
	clean run 


### SIDE NOTE
# When creating commands remember to enter newline after each one.
# If a line does not end with \n it is ignored!