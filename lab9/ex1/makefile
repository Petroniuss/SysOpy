SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

# Example usage, parameters:
#   K ->  number of chairs 
#   N ->  number of customers
run: SleepingBarberProblem
	./SleepingBarberProblem 3 5

SleepingBarberProblem: $(OBJ)
	$(CC) -o SleepingBarberProblem SleepingBarberProblem.o -lpthread -lm

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

clean:
	rm -r *.o SleepingBarberProblem

.PHONY:
	clean run
