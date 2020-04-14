SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

runServer: server
	./server

runClient: client 
	./client

client: $(OBJ) 
	$(CC) -o client client.o shared.o

server: $(OBJ) 
	$(CC) -o server server.o shared.o

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o server client

.PHONY:
	clean runServer
