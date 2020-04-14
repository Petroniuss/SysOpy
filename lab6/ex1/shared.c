#include "shared.h"

void printError() {
  int errnum = errno;
  fprintf(stderr, "Value of errno: %d\n", errno);
  perror("Error printed by perror");
  fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
}
