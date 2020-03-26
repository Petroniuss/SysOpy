SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: siginfo
	./siginfo

siginfo: $(OBJ) 
	$(CC) -o siginfo siginfo.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o siginfo

.PHONY:
	clean run 