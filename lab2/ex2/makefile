SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: find 
	echo "run"
	./find "./alal" -mtime +12 -atime -2 -maxdepth 2


%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

find: $(OBJ)
	$(CC) -o find $(OBJ)

clean: 
	rm -r *.o io ./tests/*

.PHONY:
	clean run