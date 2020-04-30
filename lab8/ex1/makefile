SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

# Example usage, parameters:
# - number of threads.
# - option (how to run) can be any of sign/block/interleaved
# - filename with pgm format
# - output file for calculated histogram.
run: main
	./main 2 interleaved "./resources/baboon.ascii.pgm" "./resources/output"

# Here are examples with 4 threads and all three options.
1: main
	./main 4 sign "./resources/baboon.ascii.pgm" "./resources/output"

2: main
	./main 4 block "./resources/baboon.ascii.pgm" "./resources/output"

3: main
	./main 4 interleaved "./resources/baboon.ascii.pgm" "./resources/output"

# This automates testing.
# It runs main with various parameters and pipes output to Times.txt
runTester: tester
	./tester

tester: $(OBJ) main
	$(CC) -o tester tester.o  

main: $(OBJ) 
	$(CC) -o main main.o utils.o -lpthread -lm

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o main tester

.PHONY:
	clean runServer
