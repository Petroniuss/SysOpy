SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: producer consumer
	echo "Compiled"

runTestConsumer: consumer
	./consumer 

consumer: $(OBJ)
	$(CC) -o consumer consumer.o utils_lib.o

producer: $(OBJ)
	$(CC) -o producer producer.o utils_lib.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o interpreter

.PHONY:
	clean run 


### SIDE NOTE
# When creating commands remember to enter newline after each one.
# If a line does not end with \n it is ignored!