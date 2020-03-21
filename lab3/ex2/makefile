SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

## Add some parameters to it!
run: parallel
	./parallel args 2 5 -commonFile

test: tester
	./tester 2 2 4

tester: parallel tester.o
	$(CC) -o tester tester.o utils_lib.o matrix_lib.o

parallel: $(OBJ)
	$(CC) -o parallel parallel.o utils_lib.o matrix_lib.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o parallel tester test/*.tst 

.PHONY:
	clean run