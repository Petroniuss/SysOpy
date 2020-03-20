SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

## Add some parameters to it!
run: parallel
	./parallel  

compile: parallel
	make parallel

parallel: $(OBJ)
	$(CC) -o parallel $(OBJ)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

clean: 
	rm -r *.o find

.PHONY:
	clean run