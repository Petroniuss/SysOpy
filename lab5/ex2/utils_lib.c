#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINE_SIZE 1000

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

// Trims last n characters.
void trimEnd(char* str, int n) {
    int len = strlen(str);
    str[len - n] = '\0';
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


void** subarray(void** array, int from, int to) {
    void** sub = malloc(sizeof(void*) * (to - from));

    for (int i = from; i < to; i++) 
        sub[i - from] = array[i];
    

    return sub;
}

void** subarrayWithNull(void** array, int from, int to) {
    void** sub = malloc(sizeof(void*) * (to - from + 1));

    for (int i = from; i < to; i++) 
        sub[i - from] = array[i];
    
    sub[to - from] = NULL;

    return sub;
}


// Counts lines of given file.
int countLines(char* filename) {
    FILE* filePtr = fopen(filename, "r");

    int count = 0;
    for (char c = getc(filePtr); c != EOF; c = getc(filePtr)) {
        if (c == '\n')  
            count++;
    }

    fclose(filePtr);

    return count;
}

//  Reads `linesN`` from given file.
char** readLines(int linesN, char* filename) {
    FILE* filePtr = fopen(filename, "r");

    char** strV = malloc(sizeof(char*) * linesN);
    char buffer [MAX_LINE_SIZE];

    for (int i = 0; i < linesN; i++) {
        fgets(buffer, MAX_LINE_SIZE, filePtr);
        strV[i] = copyString(buffer);
        
        // Trim \n 
        trimEnd(strV[i], 1);
    }

    fclose(filePtr);

    return strV;
}


char** breakLineBySpaces(char* line, int* wordC) {
    char* cpy   = copyString(line);
    char* token = strtok(cpy, " ");
    int n = 0;
    while (token != NULL) {
        n     += 1;
        token = strtok (NULL, " ");
    }
    free(cpy);

    cpy = copyString(line);
    char** words = malloc(sizeof(char*) * n);

    *wordC = n;
    words[0] = strtok(cpy, " ");
    for (int i = 1; i < n; i++) {
        words[i] = strtok(NULL, " "); 
    }

    return words;
}