#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

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
    int tmp  = i;
    while(tmp / 10 > 0) {
        tmp /= 10;
        size++;
    }

    char* buffer = (char*) malloc((size + 1) * sizeof(char));

    for(int j = size - 1; j >= 0; j--) {
        char psi  = ((i % 10) + '0');
        buffer[j] = psi;
        i /= 10;
    }
    buffer[size] = '\0';

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

// FILE UTILS ----------------------------------

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

// MATRIX--specific ---------------------

int countElemsInFirstRow(FILE* filePtr) {
    int spaces = 0;
    movePtrToLine(filePtr, 0);

    for (char c = getc(filePtr); c != '\n'; c = getc(filePtr)) {
        if (c == ' ')  
            spaces++;
    }

    return spaces + 1;
}

void readRow(Matrix* matrix, int* nums, int row) {
    movePtrToLine(matrix -> filePtr, row);
    for (int i = 0; i < matrix -> cols; i++) {
        fscanf(matrix -> filePtr, "%d", &(nums[i]));
    }
}

void readNextRow(Matrix* matrix, int* nums) {
    for (int i = 0; i < matrix -> cols; i++) {
        fscanf(matrix -> filePtr, "%d", &(nums[i]));
    }
}

void readColumn(Matrix* matrix, int* nums, int col) {
    movePtrToLine(matrix -> filePtr, 0);

    for (int i = 0; i < matrix -> rows; i++) {
        int j = 0;
        int num;

        while (j <= col) {
            fscanf(matrix -> filePtr, "%d", &num);
            j++;
        }

        nums[i] = num;
        movePtrToNextLine(matrix -> filePtr);
    }
}
