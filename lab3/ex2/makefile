SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

.PHONY:
	test clean run

## Params: file-with-matrices number-of-workers time-limit -flag (inidicating how to run program: -distinctFiles or -commonFile)
run: parallel
	./parallel test/args 2 5 -commonFile

#  Params: TestNumber Min Max, where min and max refer to number of rows/cols 
test: tester
	./tester 5 2 5

tester: parallel tester.o
	$(CC) -o tester tester.o utils_lib.o matrix_lib.o

parallel: $(OBJ)
	$(CC) -o parallel parallel.o utils_lib.o matrix_lib.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o parallel tester test/*.tst 

