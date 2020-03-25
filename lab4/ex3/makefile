SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

inheritance: $(OBJ) child
	$(CC) -o inheritance inheritance.o 

child: $(OBJ)
	$(CC) -o child child.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o inheritance

.PHONY:
	clean run 