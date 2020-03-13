SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: find 
	echo "run"
	./find "./tests" -mtime -1 -atime -1 -maxdepth 2


%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

find: $(OBJ)
	$(CC) -o find $(OBJ)

clean: 
	rm -r *.o find

.PHONY:
	clean run