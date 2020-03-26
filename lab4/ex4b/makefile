SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: sender catcher
	echo "Compiled"

sender: $(OBJ)
	$(CC) -o sender sender.o

catcher: $(OBJ)
	$(CC) -o catcher catcher.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o catcher sender

.PHONY:
	clean run 