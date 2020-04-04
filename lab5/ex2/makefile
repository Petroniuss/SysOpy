SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

## runs test which creates 5 producers and 1 consument (see test directory)
run: runner
	./runner

compile: producer consumer runner
	echo "Compiled"

runTestConsumer: consumer
	./consumer "test/sh-pipe-command-line" "test/sh-out.txt" 5

runTestProducer1: producer
	./producer "test/sh-pipe-command-line" "test/sh1.txt" 3

runTestProducer2: producer
	./producer "test/sh-pipe-command-line" "test/sh2.txt" 7

consumer: $(OBJ)
	$(CC) -o consumer consumer.o utils_lib.o

producer: $(OBJ)
	$(CC) -o producer producer.o utils_lib.o

runner: $(OBJ) consumer producer
	$(CC) -o runner runner.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o producer consumer "test/sh-pipe"

.PHONY:
	clean run 


### SIDE NOTE
# When creating commands remember to enter newline after each one.
# If a line does not end with \n it is ignored!