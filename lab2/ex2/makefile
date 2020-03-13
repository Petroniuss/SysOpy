SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: 
	echo "run"

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

superior-find: $(OBJ)
	$(CC) -o superior-find $(OBJ)

clean: 
	rm -r *.o io ./tests/*

.PHONY:
	clean run