SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: find
	make find

run: find 
	./find "./tests" 

# Tests, we can add -nftw to use nftw functions instead of readdir etc. Output will be the same except for the order.
test1: find
	./find "./tests" -mtime -4 -atime -4 -maxdepth 2 -nftw

test2: find
	./find "./tests" -mtime -1 -atime -1 -maxdepth 3 

test3: find
	./find "./tests" -mtime -5 -maxdepth 12

test4: find
	./find "./tests" -nftw

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

find: $(OBJ)
	$(CC) -o find $(OBJ)

clean: 
	rm -r *.o find

.PHONY:
	clean run