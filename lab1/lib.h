#include <stdio.h>

struct Pair {
    char* firstFile;
    char* secondFile;
} typedef Pair;

void initializeTable(int size);

void initializeSequence(int filesCount, char* files[]);

void comparePair(Pair* pair, int i);

char* tempFilename(int i);

int createBlock(int i);

int operationsForBlock(int i);

void deleteOperation(int blockIndex, int operationIndex);

void deleteBlock(int i);

void compareAll(int filesCount, char* files[]);

char* getOperation(int block, int opIndex);
