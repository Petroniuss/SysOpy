SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: find 
	./find "./tests" 

compile: find
	make find

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

find: $(OBJ)
	$(CC) -o find $(OBJ)

clean: 
	rm -r *.o find

.PHONY:
	clean run