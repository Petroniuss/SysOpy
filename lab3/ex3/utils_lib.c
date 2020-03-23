#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/resource.h>

struct Matrix {
    FILE* filePtr;
    int rows;
    int cols;
} typedef Matrix;

char* concat(const char* s1, const char* s2) {
    int   len    = strlen(s1) + strlen(s2) + 1;
    char* result = (char*) malloc(len);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

char* concatWithSeparator(const char* s1, const char* s2, const char* separator) {
    char* temp   = concat(s1, separator);
    char* result = concat(temp, s2);
    
    free(temp);
    return result;
}

char* numberToString(int i) {
    int size = 1;
    int tmp  = abs(i);
    while(tmp / 10 > 0) {
        tmp /= 10;
        size++;
    }

    char* buffer = (char*) malloc((size + 2) * sizeof(char));
    sprintf(buffer, "%d", i);

    return buffer;
}

char* copyString(const char* str) {
    int   len    = strlen(str);
    char* result = (char*) malloc(len);

    strcpy(result, str);

    return result;
}

int startsWithDigit(const char* str) {
    return isdigit(str[0]);
}

int isEmpty(const char* str) {
    return str[0] == '\0';
}

int startsWith(const char* str, const char* prefix) {
    int i = 0;
    while (str[i] != '\0' && prefix[i] != '\0') {
        if (str[i] != prefix[i]) {
            return 0;
        }
        i++;
    }

    if(prefix[i] == '\0') {
        return 1; 
    } else {
        return 0;
    }
}

char* randomString(int length) {
    char* str = calloc(length + 1, sizeof(char));

    for (int i = 0; i < length; i++) {
        char randomletter = 'a' + (rand() % 26);
        str[i] = randomletter;
    }

    str[length] = '\0';

    return str;
}

char* substring(char* str, int start, int end) {
    if (start >= end)
        return "";
    
    char* substr = (char*) malloc(sizeof(char) * (end - start + 1));

    int i = start - 1;
    while(++i < end) {
        substr[i - start] = str[i];
    }

    substr[end - start] = '\0';

    return substr;
}

char* suffix(char* str, int from) {
    int len = strlen(str);

    return substring(str, from, len);
}

// TIME UTILS

double diffSeconds(struct timespec start, struct timespec end) {
    long double elapsedTimeSeconds = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1e9);
    return elapsedTimeSeconds;
}


struct timespec nowCpuTime() {
    struct timespec now;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);

    return now;
}

double cpuTime(struct timespec start) {
    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    return diffSeconds(start, end);   
}

// System-wide realtime clock.
struct timespec nowRealTime() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    return now;
}

// Real time in seconds
double realTime(struct timespec start) {
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return diffSeconds(start, end);
}

// FILE UTILS ----------------------------------
// assumes files are open.
void copyFile(FILE* dest, FILE* src) {
    fseek(dest, 0, SEEK_SET);
    fseek(src, 0, SEEK_SET);

    char c;
    while((c = fgetc(src)) != EOF) {
        fputc(c, dest);
    }
}

void insert(FILE* file, char* buffer) {
    char* temp = malloc(sizeof(char) * 50);
    sprintf(temp, "test/temp-%d", rand());

    long int insertPosition = ftell(file);
    fseek(file, 0, SEEK_SET);

    FILE* tmp = fopen(temp, "w+");
    long i = 0;
    while(i != insertPosition) {
        fputc(fgetc(file), tmp);
        i++;
    }
    fputs(buffer, tmp);

    char c;
    while ((c = fgetc(file)) != EOF)
      fputc(c, tmp); 

    // fputc(EOF, -1);

    copyFile(file, tmp);
    
    fclose(tmp);
    remove(temp);
}

// This ugly thing allows for inserting into files.. Note that it's not very efficient.
int finsert(FILE* file, const char *buffer) {
    long int insert_pos = ftell(file);
    if (insert_pos < 0) return insert_pos;

    // Grow from the bottom
    int seek_ret = fseek(file, 0, SEEK_END);
    if (seek_ret) return seek_ret;
    long int total_left_to_move = ftell(file);
    if (total_left_to_move < 0) return total_left_to_move;

    char move_buffer[1024];
    long int ammount_to_grow = strlen(buffer);
    if (ammount_to_grow >= sizeof(move_buffer)) return -1;

    total_left_to_move -= insert_pos;

    for(;;) {
        int ammount_to_move = sizeof(move_buffer);
        if (total_left_to_move < ammount_to_move) ammount_to_move = total_left_to_move;

        long int read_pos = insert_pos + total_left_to_move - ammount_to_move;

        seek_ret = fseek(file, read_pos, SEEK_SET);
        if (seek_ret) return seek_ret;
        fread(move_buffer, ammount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        seek_ret = fseek(file, read_pos + ammount_to_grow, SEEK_SET);
        if (seek_ret) return seek_ret;
        fwrite(move_buffer, ammount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        total_left_to_move -= ammount_to_move;

        if (!total_left_to_move) break;

    }

    seek_ret = fseek(file, insert_pos, SEEK_SET);
    if (seek_ret) return seek_ret;
    fwrite(buffer, ammount_to_grow, 1, file);
    if (ferror(file)) return ferror(file);

    return 0;
}


// Move file pointer to start of specified line.
void movePtrToLine(FILE* ptr, int line) {
    fseek(ptr, 0, SEEK_SET);

    int count = 0;
    while (count < line) {
        for (char c = getc(ptr); c != '\n'; c = getc(ptr)) {}
        count++;
    }
}


// Counts lines of given file.
int countLines(FILE* filePtr) {
    int count = 0;
    movePtrToLine(filePtr, 0);

    for (char c = getc(filePtr); c != EOF; c = getc(filePtr)) {
        if (c == '\n')  
            count++;
    }

    return count;
}

void movePtrToNextLine(FILE* ptr) {
    for (char c = getc(ptr); c != '\n'; c = getc(ptr)) {}
}

// [lower, upper]
int randNum(int lower, int upper) {
    int num =  (rand() % (upper - lower + 1)) + lower;
    
    return num;
}


// System - usage
// User cpu time - time spent on the processor running your program's code | user space
// System cpu time - s the time spent running code in the operating system kernel on behalf of your program | kernel space
void logSystemUsage(int who, int pid) {
    struct rusage usage;
    getrusage(who, &usage);
    
    struct timeval userCpuTime, systemCpuTime;
    userCpuTime   = usage.ru_utime;
    systemCpuTime = usage.ru_stime;

    // Linux doesn't support reading memory usage through getrusage anymore :/. 

    printf("Process with pid: %d\n\tUser CPU time: %lu.%06lu s\n\tSystem CPU time: %lu.%06lu s\n\tFilesystem performed ouput %d times\n",
             pid, userCpuTime.tv_sec, userCpuTime.tv_usec, systemCpuTime.tv_sec, systemCpuTime.tv_usec, usage.ru_oublock);

}  