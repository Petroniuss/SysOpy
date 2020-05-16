
SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

# Example usage, parameters:
#   - UDP port number
#   - path to UNIX socket 
runServer: server
	./server 8884 .socket

# Client prameters:
# 	- name (hopefully unique) 
#	- connection type (net or local) 
#	- server address ( based on connection type ):
# 		for net:
# 			- ipv4 (127.0.0.1) :)
# 			- server port number (8884)
# 		for local:
# 			- path to UNIX socket 
runClientNet: client
	./client "basia" net 127.0.0.1 8884

runClientLocal: client
	./client "ala" local .socket 

compile: 
	make server
	make client
	echo Compiled

server: $(OBJ)
	$(CC) -o server server.o utils.o -lpthread -lm

client: $(OBJ)
	$(CC) -o client client.o utils.o -lpthread -lm

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean:
	rm -rf *.o server client
	echo "Cleaned"

.PHONY:
	clean run compile