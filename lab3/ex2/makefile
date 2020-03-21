SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

## Add some parameters to it!
run: parallel
	./parallel args 2 5 -commonFile

compile: parallel
	make parallel

parallel: $(OBJ)
	$(CC) -o parallel parallel.o utils_lib.o matrix_lib.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o parallel

.PHONY:
	clean run compile