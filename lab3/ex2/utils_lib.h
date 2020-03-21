#include <stdio.h>

// String utils 
char* concat(const char* s1, const char* s2);

char* concatWithSeparator(const char* s1, const char* s2, const char* separator);

char* numberToString(int i);

int startsWithDigit(const char* str);

int isEmpty(const char* str);

char* copyString(const char* str);

int startsWith(const char* str, const char* prefix);

char* randomString(int length);

char* substring(char* str, int start, int end);

char* suffix(char* str, int from); 

// Time utils
double realTime(struct timespec start);

double cpuTime(struct timespec start);

struct timespec nowRealTime();

struct timespec nowCpuTime();

// File uitls
int finsert(FILE* file, const char *buffer);

int countLines(FILE* filePtr);

void movePtrToLine(FILE* ptr, int line);

void movePtrToNextLine(FILE* ptr);
