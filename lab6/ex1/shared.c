#include "shared.h"

void printError() {
  int errnum = errno;
  fprintf(stderr, "Value of errno: %d\n", errno);
  perror("Error printed by perror");
  fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
}

int stringEq(char* str1, char* str2) { return strcmp(str1, str2) == 0; }

int isQueueEmpty(int queueId) {
  struct msqid_ds buf;
  msgctl(queueId, IPC_STAT, &buf);

  return buf.msg_qnum == 0;
}