#include "shared.h"

int stringEq(char* str1, char* str2) { return strcmp(str1, str2) == 0; }

char* randomString(int length) {
  char* str = calloc(length + 1, sizeof(char));

  for (int i = 0; i < length; i++) {
    char randomletter = 'a' + (rand() % 26);
    str[i] = randomletter;
  }

  str[length] = '\0';

  return str;
}

char* concat(const char* s1, const char* s2) {
  int   len = strlen(s1) + strlen(s2) + 1;
  char* result = (char*)malloc(len);

  strcpy(result, s1);
  strcat(result, s2);

  return result;
}

char* getCurrentWorkingDirectory() {
  char* buffer = malloc(sizeof(char) * PATH_MAX);
  getcwd(buffer, sizeof(buffer));

  return buffer;
}

void printError() {
  if (errno != 0) {
    fprintf(stderr, "Value of errno: %d\n", errno);
    perror("Error printed by perror");
  }
}