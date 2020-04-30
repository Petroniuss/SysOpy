SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

runFirstType: main
	./main 3 sign "./resources/baboon.ascii.pgm" "./resources/output"

runSecondType: main
	./main 3 block "./resources/baboon.ascii.pgm" "./resources/output"

runThirdType: main
	./main 3 interleaved "./resources/baboon.ascii.pgm" "./resources/output"

main: $(OBJ) 
	$(CC) -o main main.o utils.o -lpthread -lm

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o main 

.PHONY:
	clean runServer
