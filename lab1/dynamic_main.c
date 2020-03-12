#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/times.h>
#include <ctype.h>
#include <time.h>

FILE* resultFilePtr = NULL;
void (*initializeTable)(int);
int (*createBlock)(int);
int (*operationsForBlock)(int);
void (*deleteOperation)(int,int);
void (*deleteBlock)(int);
void (*compareAll)(int, char*[]);
char* (*concat)(char* , char*);
char* (*concatWithSeparator)(const char*, const char* ,const char* );
char* (*numberToString)(int);

void error(char* msg) {
    printf("%s \n", msg);
    
    exit(-1);
}

double timeDifference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void writeResult(const char* msg, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("%s --------------\n", msg);
    printf("\tREAL_TIME: %fl\n", timeDifference(start, end));
    printf("\tUSER_TIME: %fl\n", timeDifference(t_start -> tms_utime, t_end -> tms_utime));
    printf("\tSYSTEM_TIME: %fl\n", timeDifference(t_start -> tms_stime, t_end -> tms_stime));

    fprintf(resultFilePtr, "%s --------------\n", msg);
    fprintf(resultFilePtr, "\tREAL_TIME: %fl\n", timeDifference(start, end));
    fprintf(resultFilePtr, "\tUSER_TIME: %fl\n", timeDifference(t_start -> tms_utime, t_end -> tms_utime));
    fprintf(resultFilePtr, "\tSYSTEM_TIME: %fl\n\n", timeDifference(t_start -> tms_stime, t_end -> tms_stime));
}

char* testCreateTable(char* sizeStr) {
    int size = atoi(sizeStr);
    char* msg = "create_table ";
    msg = (*concat)(msg, sizeStr);
    (*initializeTable)(size);

    return msg;
}

void testComparePairs(char* argv[], int from, int to) {
    char** files = calloc(to - from + 1, sizeof(char*));
    for(int i = from; i <= to; i++) {
        files[i - from] = argv[i];
    }

    (*compareAll)(to - from + 1, files);
    free(files);
}

char* testRemoveBlock(char* indexStr, char* timesStr) {
    int blockIndex = atoi(indexStr);
    int times = atoi(timesStr);


    char* msg = "Remove_block ";
    char* tmp = (*concat)(msg, indexStr);
    char* result = (*concatWithSeparator)(tmp, timesStr, " ");

    free(tmp);

    while(times-- > 0) {
        (*deleteBlock)(blockIndex);
        (*createBlock)(blockIndex);
    }

    return result;
}

char* testRemoveOperation(char* blockStr, char* operationStr) {
    int blockIndex = atoi(blockStr);
    int operationIndex = atoi(operationStr);
    
    char* msg = "Remove operation ";
    char* tmp = (*concat)(msg, blockStr);
    msg = (*concatWithSeparator)(tmp, operationStr, " ");
    free(tmp);

    (*deleteOperation)(blockIndex, operationIndex);

    return msg;
}

int main(int argc, char* argv[]) {
    resultFilePtr = fopen("raport2.txt", "a");
    void *handle = dlopen("./liblib.so", RTLD_LAZY);

    if(!handle) {
        printf("Failed to load\n");
    }


    initializeTable = dlsym(handle, "initalizeTable");
    createBlock     = dlsym(handle, "createBlock");
    operationsForBlock = dlsym(handle, "operationsForBlock");
    deleteOperation = dlsym(handle, "deleteOperation");
    deleteBlock = dlsym(handle, "deleteBlock");
    compareAll = dlsym(handle, "compareAll");
    concat = dlsym(handle, "concat");
    concatWithSeparator = dlsym(handle, "concatWithSeparator");
    numberToString = dlsym(handle, "numberToString");

    printf("y\n");

    int i = 1;
    clock_t startTime;
    clock_t endTime;
    struct tms* tmsStart = calloc(1, sizeof(struct tms*));
    struct tms* tmsEnd   = calloc(1, sizeof(struct tms*));

    while (i < argc) {
        const char* command = argv[i];
        char* msg = NULL;
        startTime = times(tmsStart);

        if (strcmp("create_table", command) == 0) {
            msg = testCreateTable(argv[++i]);
        } else if (strcmp("compare_pairs", command) == 0) {
            msg = (*concatWithSeparator)("compare_pairs", argv[i + 1], " ");
            int filesCount = atoi(argv[++i]);
            testComparePairs(argv, i + 1, i + filesCount);
            i += filesCount;           
        } else if (strcmp("remove_block", command) == 0) {
            msg = testRemoveBlock(argv[i + 1], argv[i + 2]);
            i += 2;
        } else if ("remove_operation") {
            msg = testRemoveOperation(argv[i + 1], argv[i + 2]);
            i += 2;
        } else {
            error((*concatWithSeparator)("No such command as", command, " "));
        }

        endTime = times(tmsEnd);
        writeResult(msg, startTime, endTime, tmsStart, tmsEnd);
        i++;
    }

    fclose(resultFilePtr);
    dlclose(handle);

    return 0;
}

