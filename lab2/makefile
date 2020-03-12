SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

io: $(OBJ)
	$(CC) -o io $(OBJ)

generate: io
	./io generate "files/a.txt" 3 5

copy: io
	./io copy "files/a.txt" "files/a-copy.txt" 3 5 -sys

clean: 
	rm -r *.o io

.PHONY:
	generate clean