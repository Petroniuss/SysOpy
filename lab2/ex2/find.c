#include "string_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <sys/times.h>

#define MODE_SET       ((unsigned char) '1')
#define MODE_NOT_SET   ((unsigned char) '0')
#define MODE_MORE      ((unsigned char) '+')
#define MODE_LESS      ((unsigned char) '-')
#define MODE_PRECISELY ((unsigned char) '=')

// LOGGING ----------------------------------------------------------

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(0);
}

void logInfo(FILE* logFile, char* msg) {
    printf("%s\n", msg);
    fprintf(logFile, "%s\n", msg);
}

double timeDifference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void formatTime(char* buffer, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    double real = timeDifference(start, end);
    double user = timeDifference(t_start -> tms_utime, t_end -> tms_utime);
    double system = timeDifference(t_start -> tms_stime, t_end -> tms_stime);

    sprintf(buffer, "Timing ---------------------------\n\tREAL_TIME: %fs\n\tUSER_TIME: %fs\n\tSYSTEM_TIME: %fs\n----------------------------------\n", real, user, system);
}

// -----------------------------------------------------------------

struct FindArgs {
    char* path;

    int mTime;
    unsigned char mMode;

    int aTime;
    unsigned char aMode;

    unsigned char maxDepthMode;
    int maxDepth;
} typedef FindArgs;

struct Mode {

} typedef Mode;

void showArgs(FindArgs* args) {
    printf("%s\n", args -> path);
    printf("%d\n", args -> mTime);
    printf("%c\n", args -> mMode);
    printf("%d\n", args -> aTime);
    printf("%c\n", args -> aMode);
    printf("%d\n", args -> maxDepth);
    printf("%c\n", args -> maxDepthMode);
}


// -----------------------------------------------------------------

int main(int argc, char* argv[]) {
    FILE* logFile = fopen("log.txt", "a");
    
    clock_t startTime;
    clock_t endTime;
    struct tms* tmsStart = calloc(1, sizeof(struct tms*));
    struct tms* tmsEnd   = calloc(1, sizeof(struct tms*));

    char msg[250];
    int i = 1;
    srand(time(0));

    FindArgs* args = (FindArgs*) malloc(sizeof(FindArgs));
    args -> path = argv[i];
    args -> aMode = MODE_NOT_SET;
    args -> mMode = MODE_NOT_SET;
    args -> maxDepthMode = MODE_NOT_SET; 

    while (++i < argc) {
        char* flag = argv[i];
        char* arg  = argv[++i];

        if (strcmp("-mtime", flag) == 0) {
            if (startsWith(arg, "-")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> mTime = value;
                args -> mMode = MODE_LESS;
                free(strNum);
            } else if (startsWith(arg, "+")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> mTime = value;
                args -> mMode = MODE_MORE;
                free(strNum);
            } else if (startsWithDigit(arg)) {
                int   value    = atoi(arg);

                args -> mTime = value;
                args -> mMode = MODE_PRECISELY;
            } else {
                error("Bad argument");
            }
        } else if (strcmp("-atime", flag) == 0) { 
            if (startsWith(arg, "-")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> aTime = value;
                args -> aMode = MODE_LESS;
                free(strNum);
            } else if (startsWith(arg, "+")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> aTime = value;
                args -> aMode = MODE_MORE;
                free(strNum);
            } else if (startsWithDigit(arg)) {
                int   value    = atoi(arg);

                args -> aTime = value;
                args -> aMode = MODE_PRECISELY;
            } else {
                error("Bad argument");
            }
        } else if (strcmp("-maxdepth", flag) == 0) { 
            args -> maxDepth = atoi(arg);
            args -> maxDepthMode = MODE_SET;
        } else {
            error("Uknown flag");
        }
    }

    showArgs(args);
    fclose(logFile);

    return 0;
}