SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

## Add some parameters to it!
run: parallel
	./parallel args 2 5

compile: parallel
	make parallel

parallel: $(OBJ)
	$(CC) -o parallel $(OBJ)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o parallel

.PHONY:
	clean run compile