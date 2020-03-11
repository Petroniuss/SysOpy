SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ := $(patsubst %.c, %.o, $(SRC))
CC=gcc -Wall 
# Here we can add optimization flags O O1 O2 O3 O0 Os as it was specified in 3rd exercise.

run: 
	echo "Testing with flags: $(CC)" >> raport2.txt
	make static_test
	make dynamic_test
	make shared_test

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< 


static: $(OBJ) $(HEADERS) 
	ar crs liblib.a lib.o string_lib.o
	$(CC) main.c -o main -L . -l lib

shared:
	$(CC) -fPIC -c string_lib.c
	$(CC) -fPIC -c lib.c
	$(CC) -shared -fPIC -o liblib.so lib.o string_lib.o
	$(CC) main.c -o main -L. -l lib -Wl,-rpath=`pwd`


small_test:
	echo "Small Test"
	echo "Small Test (4 files - small files) a.txt b.txt cp.html grammar.lsp" >> raport2.txt
	./main "create_table" "4" "compare_pairs" "4" "a.txt" "b.txt" "alice29.txt" "grammar.lsp" "remove_operation" "1" "2" "remove_operation" "1" "0" "remove_block" "0" "1000"

medium_test:
	echo "Medium Test"
	echo "Medium Test (10 files - medium-sized/large files) a.txt b.txt cp.html grammar.lsp lcet10.txt asyoulik.txt aaa.txt alphabet.txt"
	echo "Medium Test (10 files - medium-sized/large files) a.txt b.txt cp.html grammar.lsp lcet10.txt asyoulik.txt aaa.txt alphabet.txt" >> raport2.txt
	./main "create_table" "10" "compare_pairs" "10" "a.txt" "b.txt" "alice29.txt" "grammar.lsp" "lcet10.txt" "asyoulik.txt" "aaa.txt" "alphabet.txt" "E.coli" "b.txt" "remove_block" "0" "1000" "remove_operation" "1" "2" "remove_operation" "1" "0" "remove_operation" "1" "4"


huge_test:
	echo "Huge Test"
	echo "Huge Test (20 files - large/very large) E.coli bible.txt pi.txt kennedy.xls plrbn12.txt xargs.1 alphabet.txt alice29.txt a.txt world192.txt cp.html grammar.lsp lcet10.txt asyoulik.txt aaa.txt alphabet.txt" 
	echo "Huge Test (20 files - large/very large) E.coli bible.txt pi.txt kennedy.xls plrbn12.txt xargs.1 alphabet.txt alice29.txt a.txt world192.txt cp.html grammar.lsp lcet10.txt asyoulik.txt aaa.txt alphabet.txt" >> raport2.txt
	./main "create_table" "20" "compare_pairs" "20" "bible.txt" "pi.txt" "random.txt" "alphabet.txt" "E.coli" "asyoulik.txt" "a.txt" "b.txt" "alice29.txt" "grammar.lsp" "aaa.txt" "random.txt" "alphabet.txt" "a.txt" "b.txt" "alice29.txt" "grammar.lsp" "asyoulik.txt" "cp.html" "grammar.lsp" "remove_operation" "1" "2" "remove_operation" "1" "0" "remove_operation" "1" "4"

static_test:
	echo "Static library " >> raport2.txt
	make shared
	make small_test 
	make medium_test
	make huge_test

shared_test:
	echo "Shared library " >> raport2.txt
	make shared
	make small_test 
	make medium_test
	make huge_test

dynamic_test:
	echo "Shared library, dynamically loaded " >> raport2.txt
	make shared
	$(CC) dynamic_main.c -o dynamic_main -ldl -L . -D DYNAMIC
	make small_test 
	make medium_test
	make huge_test

clean:
	rm *.o *.a *.so main temp*.txt
