#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_lib.h"
#include "lib.h"

const int MAX_LINE_WIDTH = 1000;

char*** table           = NULL;
int*    blockOperations = NULL; 
Pair**  sequence        = NULL;
int     seqSize         = -1;

// BASIC OPERATIONS -------------------------------------

// Initializes `table` with given size.
void initializeTable(int size) {
    if (table != NULL)
        free(table);

    table   = (char***) calloc(size, sizeof(char**));
    seqSize = size;
    blockOperations = (int*) calloc(size, sizeof(int));
}

// Initializes sequence of pairs of files and stores them into `sequence`.
void initializeSequence(int filesCount, char* files[]) {
    if (sequence != NULL) {
        for(int i = 0; i < seqSize; i++) {
            free(sequence[i]);
        }
        free(sequence);
    }

    if (filesCount % 2 != 0) {
        printf("Sequence must contain even number of files!");
        exit(-1);
    }

    sequence = (Pair**) calloc(filesCount / 2, sizeof(Pair*)); 
    
    for (int i = 0; i < filesCount; i += 2) {
        Pair* pair = (Pair*) malloc(sizeof(Pair));
        pair -> firstFile  = files[i];
        pair -> secondFile = files[i + 1];

        sequence[i / 2] = pair;
    }

    seqSize = filesCount / 2;
}

// Pipes output of `diff` performed on pair to temp`i`.txt.
// In other words it creaters temporary file which stores output of diff.
void comparePair(Pair* pair, int i) {
    char* args           = concatWithSeparator(pair -> firstFile, pair -> secondFile, " ");
    char* cmdWithArgs    = concatWithSeparator("diff", args, " ");
    char* indexedTmpFile = concat("temp", numberToString(i));
    char* generatedFile  = concat(indexedTmpFile, ".txt");
    char* fullCommand    = concatWithSeparator(cmdWithArgs, generatedFile, " > ");
    
    system(fullCommand);

    free(args);
    free(cmdWithArgs);
    free(indexedTmpFile);
    free(fullCommand); 
}

// Returns filename of i'th temporary file.
char* tempFilename(int i) {
    if (i < 0 || i >= seqSize || seqSize == -1) {
        printf("There's no such tmp file: %i \n", i);
        exit(-1);
    }

    char* iStr = numberToString(i);
    char* res  = concat("temp", iStr);
    char* filename = concat(res, ".txt");

    free(iStr);
    free(res);

    return filename;
}

// For temporary file it does all of the crazy stuff.
// We start off by traversing through the file and estimating how much memory we need to allocate, then we save editorial changes.
int createBlock(int i) {
    if (table[i] != NULL) {
        deleteBlock(i);
    }

    char* filename = tempFilename(i);
    char* buffer   = (char*) calloc(MAX_LINE_WIDTH, sizeof(char));
    int   size     = 0;

    FILE* filePtr  = fopen(filename, "r");

    while (fgets(buffer, MAX_LINE_WIDTH, filePtr)) {
        if (startsWithDigit(buffer)) {
            size++;
        }
    }

    fclose(filePtr);

    table[i] = (char**) calloc(size, sizeof(char*));
    blockOperations[i] = size;
    filePtr = fopen(filename, "r");

    char* editorialBlock = NULL;
    int j = 0;

    while (fgets(buffer, MAX_LINE_WIDTH, filePtr)) {
        if (startsWithDigit(buffer)) {
            if (editorialBlock == NULL) {
                editorialBlock = concat("", buffer);
            } else {
                table[i][j] = editorialBlock;
                // printf("Block num: %d, %s", j, editorialBlock);
                editorialBlock = copyString(buffer);
                j++;
            }
        } else {
            char* conc = concat(editorialBlock, buffer);
            free(editorialBlock);
            editorialBlock = conc;
        }
    }

    // If files are the same then j is 0.
    if (j != 0) {
        table[i][j] = editorialBlock;
        // printf("Block num: %d, %s", j, editorialBlock);
    }

    fclose(filePtr);

    return i;
}

// Returns number of editorial operations for i'th block. 
// It takes into account fact that we might have deleted some of them.
int operationsForBlock(int i) {
    if(table[i] == NULL) {
        return 0;
    }

    int count = 0;
    int max = blockOperations[i];

    for (int j = 0; j < max; j++) {
        if (table[i][j] != NULL) {
            count++;
        }
    }

    return count;
}

// Deletes single operation.
void deleteOperation(int blockIndex, int operationIndex) {
    free(table[blockIndex][operationIndex]);
    table[blockIndex][operationIndex] = NULL;
}

// Deletes i'th block
void deleteBlock(int i) {
    int size = blockOperations[i];
    for (int j = 0; j < size; j++) {
        deleteOperation(i, j);
    }

    free(table[i]);
    table[i] = NULL;
}

// Returns operation stored in table.
char* getOperation(int block, int opIndex) {
    return table[block][opIndex];
}

// Main Functionality ---------------------------------------------------------
// Compares all given files (pairs), and initializes all of the fields.
void compareAll(int filesCount, char* files[]) {
    initializeSequence(filesCount, files);
    initializeTable(seqSize);

    for(int i = 0; i < seqSize; i++) {
        comparePair(sequence[i], i);
        createBlock(i);
    }
}
