SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

run: main
	./main

main: $(OBJ) worker_packer worker_receiver worker_sender
	$(CC) -o main main.o shared.o

worker_packer: $(OBJ) 
	$(CC) -o worker_packer worker_packer.o shared.o 

worker_receiver: $(OBJ) 
	$(CC) -o worker_receiver worker_receiver.o shared.o 

worker_sender: $(OBJ) 
	$(CC) -o worker_sender worker_sender.o shared.o 

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean: 
	rm -r *.o worker_sender worker_receiver worker_packer main 

.PHONY:
	clean runServer
