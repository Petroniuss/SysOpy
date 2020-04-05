SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

# Example using prepared file.
test: sort
	./sort "test/sh1.txt" 

compile: sort
	echo "Compiled"

sort: $(OBJ)
	$(CC) -o sort sort.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o sort

.PHONY:
	clean run 