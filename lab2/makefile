SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

io: $(OBJ)
	$(CC) -o io $(OBJ)

generate: io
	./io generate "files/a.txt" 24 1

copy: io
	./io copy "files/a.txt" "files/a-copy.txt" 5 1 -lib

sort: io
	./io sort "files/a.txt" 24 1 -lib

clean: 
	rm -r *.o io

.PHONY:
	generate clean