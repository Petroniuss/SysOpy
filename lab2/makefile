SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC=gcc -Wall 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

io: $(OBJ)
	$(CC) -o io $(OBJ)

generate: io
	./io generate "files/a.txt" 5 10

clean: 
	rm -r *.o io

.PHONY:
	generate clean