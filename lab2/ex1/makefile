SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC = gcc -Wall 

compile: io
	make io

# Tests!

run: io
	make test1
	make test4
	make test512
	make test1024
	make test4096
	make test8192
	
test1: io
	./io generate "tests/test_1.txt" 100 1
	./io copy 	  "tests/test_1.txt" "tests/test_1-sys.txt" 100 1 -sys
	./io copy 	  "tests/test_1.txt" "tests/test_1-lib.txt" 100 1 -lib
	./io sort 	  "tests/test_1-lib.txt" 100 1 -lib
	./io sort 	  "tests/test_1-sys.txt" 100 1 -sys

test4: io
	./io generate "tests/test_4.txt" 100  4
	./io copy 	  "tests/test_4.txt" "tests/test_4-sys.txt" 100  4 -sys
	./io copy 	  "tests/test_4.txt" "tests/test_4-lib.txt" 100  4 -lib
	./io sort 	  "tests/test_4-lib.txt" 100 4 -lib
	./io sort 	  "tests/test_4-sys.txt" 100  4 -sys

test512: io
	./io generate "tests/test_512.txt" 15000 512
	./io copy 	  "tests/test_512.txt" "tests/test_512-sys.txt" 15000 512 -sys
	./io copy 	  "tests/test_512.txt" "tests/test_512-lib.txt" 15000 512 -lib
	./io sort 	  "tests/test_512-lib.txt" 15000 512 -lib
	./io sort 	  "tests/test_512-sys.txt" 15000 512 -sys

test1024: io
	./io generate "tests/test_1024.txt" 15000 1024
	./io copy 	  "tests/test_1024.txt" "tests/test_1024-sys.txt" 15000 1024 -sys
	./io copy 	  "tests/test_1024.txt" "tests/test_1024-lib.txt" 15000 1024 -lib
	./io sort 	  "tests/test_1024-lib.txt" 15000 1024 -lib
	./io sort 	  "tests/test_1024-sys.txt" 15000 1024 -sys

test4096: io
	./io generate "tests/test_4096.txt" 15000 4096
	./io copy 	  "tests/test_4096.txt" "tests/test_4096-sys.txt" 15000 4096 -sys
	./io copy 	  "tests/test_4096.txt" "tests/test_4096-lib.txt" 15000 4096 -lib
	./io sort 	  "tests/test_4096-lib.txt" 15000 4096 -lib
	./io sort 	  "tests/test_4096-sys.txt" 15000 4096 -sys

test8192: io
	./io generate "tests/test_8192.txt" 15000 8192
	./io copy 	  "tests/test_8192.txt" "tests/test_8192-sys.txt" 15000 8192 -sys
	./io copy 	  "tests/test_8192.txt" "tests/test_8192-lib.txt" 15000 8192 -lib
	./io sort 	  "tests/test_8192-lib.txt" 15000 8192 -lib
	./io sort 	  "tests/test_8192-sys.txt" 15000 8192 -sys


%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 

io: $(OBJ)
	$(CC) -o io $(OBJ)

# Example commands!

generate: io
	./io generate "tests/a.txt" 24 1

copy: io
	./io copy "tests/a.txt" "tests/a-copy.txt" 24 1 -lib

sort: io
	./io sort "tests/a.txt" 24 1 -lib

clean: 
	rm -r *.o io ./tests/*

.PHONY:
	generate copy sort clean test1 test4 test512 test1024 test4096 test8192 run