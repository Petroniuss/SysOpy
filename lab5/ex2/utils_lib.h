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

void** subarray(void** array, int from, int to);

void** subarrayWithNull(void** array, int from, int to);

int countLines(char* filename);

char** readLines(int linesN, char* filename);

char** breakLineBySpaces(char* line, int* wordC);