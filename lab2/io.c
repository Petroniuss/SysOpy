#include "string_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void generate(char* filename, int recordsNum, int recordSize) {
    int fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU);

    while (recordsNum-- > 0) {
        char* rand = randomString(recordSize);
        char* buffer = concat(rand, "\n");

        write(fd, buffer, recordSize + 1);

        free(rand);
        free(buffer);
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    char msg[250] = "";
    int i = 1;
    while (i < argc) {
        const char* command = argv[i];
        
        if (strcmp("generate", command) == 0) {
            char* filename   = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            sprintf(msg, "Command: generate %s %s %s\n", argv[i - 2], argv[i - 1], argv[i]);
            generate(filename, recordsNum, recordSize);
            printf("%s", msg);
            printf("Generated %s\n", filename);
        } else {
            exit(0);
        }
    }

    return 0;
}


