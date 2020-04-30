#include "utils.h"

PGM* readHeaderPGM(const char* filename) {
    PGM*  header = malloc(sizeof(PGM));
    FILE* ptr = fopen(filename, "r");

    char magicNum[16];
    // Read magic number and ignore following comment.
    fscanf(ptr, "%s\n#%*[^\n]\n", magicNum);
    fscanf(ptr, "%d %d %d", &header->witdth, &header->height, &header->M);

    fclose(ptr);
    return header;
}

FILE* init(const char* filename) {
    FILE* ptr = fopen(filename, "r");

    fscanf(ptr, "%*s\n#%*[^\n]\n");
    fscanf(ptr, "%*d %*d %*d");

    return ptr;
}

int readNext(FILE* ptr) {
    int v;
    fscanf(ptr, "%d", &v);

    return v;
}

int readKth(FILE* ptr, int k) {
    int v;
    while (k-- > 0) {
        fscanf(ptr, "%d", &v);
    }

    return v;
}

void ignoreK(FILE* ptr, int k) {
    int x;
    while (k-- > 0) {
        fscanf(ptr, "%d", &x);
    }
}

// Stupid c does not even have normal boolean type..
bool stringEq(const char* x, const char* y){
    int res = strcmp(x, y);

    if (res == 0) {
        return true;
    } else {
        return false;
    }    
}

void printResults(int* data, int m) {
    printf("--------------------------------Matrix histogram--------------------------------\n");
    int sq = (int) sqrt((double) m);
    for (int i = 0; i < m; i++) {
        printf("%04d ", data[i]);
        if (i % sq == (sq - 1)) {
            printf("\n");
        }
    }       
    printf("---------------------------------------------------------------------------------\n");
}
