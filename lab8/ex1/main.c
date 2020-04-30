#include "utils.h"

int m;
char* filename;
PGM* header;

struct SignThreadArg {
    int from;
    int until;
} typedef SignThreadArg;

struct ThreadArg {
    int k;
} typedef ThreadArg;

void* threadSign(void* signThreadArg) {
    SignThreadArg* arg = (SignThreadArg*) signThreadArg;
    int* threadResults = calloc(header -> M + 1, sizeof(int)); 
    printf("Thread: %d until %d\n", arg -> from, arg -> until);

    FILE* filePtr = init(filename);
    int value;
    int count = 0;
    for (int i = 0; i < (header -> witdth) * (header -> height); i++) {
        value = readNext(filePtr);
        if (value >= arg -> from && value < arg -> until) {
            count++;
            threadResults[value] += 1;
        }
    }

    printf("Last: %d\n", count);
    free(arg);
    fclose(filePtr);
    return threadResults;
}

void* threadBlock(void* threadArg) {
    ThreadArg* arg = (ThreadArg*) threadArg;
    int* threadResults = calloc(header -> M + 1, sizeof(int)); 
    FILE* filePtr = init(filename);

    int readK = ceil(header -> witdth / m);
    int ignore = (header -> witdth) - readK; 

    printf("ReadK: %d, ignoreK: %d, firstignore: %d\n", readK, ignore, arg -> k * readK);
    int value;
    for (int j = 0; j < header -> height; j++) {
        if (j == 0) {
           ignoreK(filePtr, arg -> k * readK); 
        } else {
           ignoreK(filePtr, ignore); 
        }

        for (int i = 0; i < readK; i++) {
            value = readNext(filePtr);
            threadResults[value] += 1;
        }
    }
    
    free(arg);
    fclose(filePtr);
    return threadResults;
}

void* threadInterleaved(void* threadArg) {
    ThreadArg* arg = (ThreadArg*) threadArg;
    int* threadResults = calloc(header -> M + 1, sizeof(int)); 
    FILE* filePtr = init(filename);

    int value;
    int leapSize = m - 1;
    int max = header -> witdth * header -> height;

    ignoreK(filePtr, arg -> k);
    value = readNext(filePtr);
    threadResults[value] += 1;
    printf("First %d \n", value);
    for (int i = arg -> k + leapSize + 1; i < max; i += leapSize + 1) {
        ignoreK(filePtr, leapSize);
        value = readNext(filePtr);
        threadResults[value] += 1;
    }

    printf("Last %d \n", value);
    fclose(filePtr);
    free(arg);
    return threadResults;
}

// Try roboto mono!
int main(int charc, char* argv []) {
    if (charc < 5) {
        printf("Not enaugh arguments!\n");
        return -1;
    }

    m = atoi(argv[1]);
    char* flag = argv[2];
    filename = argv[3];
    char* outputFilename = argv[4];

    header = readHeaderPGM(filename);
    pthread_t* threads = malloc(sizeof(pthread_t) * m);

    if (stringEq(flag, "sign")) {
        int reminder = (header -> M + 1) % m;
        int uniform  = (header -> M + 1) / m;
        int limit = 0;
        for (int i = 0; i < m; i++) {
           int from = limit;
           int until   = from + uniform;

           if (reminder > 0) {
               until += 1;
               reminder -= 1;
           }
           limit = until;

           // Spawn thread!
           SignThreadArg* args = malloc(sizeof(SignThreadArg));
           args -> from = from;
           args -> until = until;
           printf("%d %d\n", args -> from, args -> until);

           pthread_create(&threads[i], NULL, threadSign, (void*) args);
        }

    } else if (stringEq(flag, "block")) {
        for (int i = 0; i < m; i++) {
            ThreadArg* arg = malloc(sizeof(ThreadArg));
            arg -> k = i;

            pthread_create(&threads[i], NULL, threadBlock, (void*) arg);
        } 
    } else if (stringEq(flag, "interleaved")) {
        for (int i = 0; i < m; i++) {
            ThreadArg* arg = malloc(sizeof(ThreadArg));
            arg -> k = i;

            pthread_create(&threads[i], NULL, threadInterleaved, (void*) arg);
        } 

    } else {
        printf("Given flag: %s not supported!\n", flag);
    }

    // Wait for threads termination and gather results in a generic way.
    int* results = calloc(header -> M + 1, sizeof(int)); 
    void* threadResults; 
    for (int i = 0; i < m; i++) {
        pthread_join(threads[i], &threadResults);
        int* threadValues = (int*) threadResults;
        for (int p = 0; p < header -> M + 1; p++) {
            results[p] += threadValues[p];
        }

        //printResults(threadValues, header -> M + 1);
        int sum = 0; 
        for (int i = 0; i < header -> M + 1; i++) {
            sum += results[i];
        }
        //printf("%d vs %d\n", sum, header -> witdth * header -> height);
        free(threadResults);
    }


    int sum = 0; 
    for (int i = 0; i < header -> M + 1; i++) {
        sum += results[i];
    }
    printf("%d vs %d\n", sum, header -> witdth * header -> height);
    printResults(results, header -> M + 1);
    free(results);
    free(threads);

    return 0;
}
