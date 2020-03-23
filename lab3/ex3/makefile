SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

.PHONY:
	test clean run

## Params: -file-with-matrices -number-of-workers -time-limit -flag -cpuTimeLimit -memoryLimit
# flag indicates how to run program: -distinctFiles or -commonFile
run: parallel worker
	./parallel test/args 2 5 -distinctFiles 3 10

#  Params: -TestNumber -Min -Max -cpuTimeLimit -memoryLimit
#  min and max refer to number of rows/cols 
test: tester
	./tester 5 2 5 3 10

tester: parallel worker tester.o
	$(CC) -o tester tester.o utils_lib.o matrix_lib.o

parallel: $(OBJ) worker
	$(CC) -o parallel parallel.o utils_lib.o matrix_lib.o 

worker: $(OBJ)
	$(CC) -o worker worker.o utils_lib.o matrix_lib.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o parallel tester test/*.tst worker

