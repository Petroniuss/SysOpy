#include "utils.h"

// Global variables <=> shared memory between threads.
int m;
char* filename;
PGM* header;
int** img;
int** threadResults;

// Arguments passed to threads.
struct SignThreadArg {
    int from;
    int until;
    int k;
} typedef SignThreadArg;

struct ThreadArg {
    int k;
} typedef ThreadArg;


long int getTimeMicroSec(struct timespec start, struct timespec end) {
    long int elapsedTime;

    elapsedTime = (end.tv_sec - start.tv_sec) * 1e6;
    elapsedTime += (end.tv_nsec - start.tv_nsec) * 1e-3;

    return elapsedTime;
}

// First option
void* threadSign(void* signThreadArg) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    
    SignThreadArg* arg = (SignThreadArg*) signThreadArg;
    threadResults[arg -> k]  = calloc(header -> M + 1, sizeof(int)); 

    int value;
    for (int j = 0; j < header -> height; j++) {
        for(int i = 0; i < header -> width; i++) {
            value = img[j][i];
            if (value >= arg -> from && value < arg -> until) {
                threadResults[arg -> k][value] += 1;
            }
        }
    }

    free(arg);
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return (void*) getTimeMicroSec(start, end);
}

// Second option
void* threadBlock(void* threadArg) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    ThreadArg* arg = (ThreadArg*) threadArg;
    threadResults[arg -> k]  = calloc(header -> M + 1, sizeof(int)); 

    int readK = (header -> width) / m;
    int ignore = readK * arg -> k;
    // Just in case we get odd number.
    if (arg -> k == m - 1) {
        readK += readK % m; 
    }

    int value;
    int startX = ignore;
    for (int j = 0; j < header -> height; j++) {
        for(int i = 0; i < readK; i++) {
            value = img[j][startX + i];
            threadResults[arg -> k][value] += 1;
        }
    }
    
    free(arg);
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return (void*) getTimeMicroSec(start, end);
}

// Third option
void* threadInterleaved(void* threadArg) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    ThreadArg* arg = (ThreadArg*) threadArg;
    threadResults[arg -> k]  = calloc(header -> M + 1, sizeof(int)); 

    int value;
    int leapSize = m;
    int max = header -> width * header -> height;

    int i = arg -> k;
    int x, y;
    while (i < max) {
        x = i % header -> width;
        y = i / header -> width;

        value = img[y][x];
        threadResults[arg -> k][value] += 1;

        i += leapSize;
    }

    free(arg);
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    struct timespec endCpu;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endCpu);

    return (void*) getTimeMicroSec(start, end);
}

int main(int charc, char* argv []) {
    if (charc < 5) {
        printf("Not enaugh arguments!\n");
        return -1;
    }

    m = atoi(argv[1]);
    char* flag = argv[2];
    filename = argv[3];
    char* outputFilename = argv[4];

    header = malloc(sizeof(PGM));
    img    = readImg(filename, header);
    pthread_t* threads = malloc(sizeof(pthread_t) * m);
    threadResults = malloc(sizeof(int*) * m);

    // Measure time without IO!
    struct timespec startReal;
    clock_gettime(CLOCK_REALTIME, &startReal);
    struct timespec startCpu;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startCpu);

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
           args -> k = i;

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

    printf("-------------------------------------------------------\n");
    printf("Running %s option, with %d threads.\n\n", flag, m);

    // Wait for threads termination and gather results in a generic way.
    int* results = calloc(header -> M + 1, sizeof(int)); 
    void* threadReturnValue;
    for (int i = 0; i < m; i++) {
        pthread_join(threads[i], &threadReturnValue);
        long int threadTime = (long int) threadReturnValue;
        printf("Thread: %lx, elapsed time: %ld mircoseconds.\n", threads[i], threadTime);
        for (int p = 0; p < header -> M + 1; p++) {
            results[p] += threadResults[i][p];
        }

        free(threadResults[i]);
    }

    // Report measured time! 
    struct timespec endReal;
    clock_gettime(CLOCK_REALTIME, &endReal);
    struct timespec endCpu;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endCpu);

    long int elapsedCpuTime = getTimeMicroSec(startCpu, endCpu);
    long int elapsedRealTime = getTimeMicroSec(startReal, endReal);

    printf("Total cpu time: %ld mircoseconds.\n", elapsedCpuTime);
    printf("Total elapsed (real) time: %ld mircoseconds.\n", elapsedRealTime);
    printf("-------------------------------------------------------\n\n");
    
    // Save histogram to file.
    saveResults(results, header -> M + 1, outputFilename);
    freeImg(img, header);
    free(header);
    free(threadResults);
    free(results);
    free(threads);

    return 0;
}
