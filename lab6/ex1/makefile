SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

runServer: server
	./server

client: $(OBJ) 
	$(CC) -o client client.o

server: $(OBJ) 
	$(CC) -o server server.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o server client

.PHONY:
	clean runServer
