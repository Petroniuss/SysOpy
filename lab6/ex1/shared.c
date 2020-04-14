#include "shared.h"

const int ID_NUMS[SERVER_MAX_CLIENTS_CAPACITY] = {
    231,  432,  3489, 483,  4829, 83214, 4392, 832,   238,  374, 824,
    8343, 121,  43,   243,  5145, 5235,  5342, 52346, 5324, 34,  32,
    42,   4345, 534,  5234, 543,  123,   321,  456,   654,  41};

int iPointer = 0;

void printError() {
  if (errno != 0) {
    fprintf(stderr, "Value of errno: %d\n", errno);
    perror("Error printed by perror");
  }
}

int stringEq(char* str1, char* str2) { return strcmp(str1, str2) == 0; }

int isQueueEmpty(int queueId) {
  struct msqid_ds buf;
  msgctl(queueId, IPC_STAT, &buf);

  return buf.msg_qnum == 0;
}
