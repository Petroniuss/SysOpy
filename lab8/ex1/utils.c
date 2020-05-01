#include "utils.h"

int** readImg(const char* filename, PGM* header) {
    FILE* ptr = fopen(filename, "r");
    char magicNum[16];
    // Read magic number and ignore following comment.
    fscanf(ptr, "%s\n#%*[^\n]\n", magicNum);
    fscanf(ptr, "%d %d %d", &header->width, &header->height, &header->M);

    int** img = malloc(sizeof(int*) * header-> height);
    for (int j = 0; j < header -> height; j++) {
        img[j] = malloc(sizeof(int) * header -> width);
        for(int i = 0; i < header -> width; i++) {
            img[j][i] = readNext(ptr);
        }
    }

    fclose(ptr);
    return img;
}

void freeImg(int** img, PGM* header) {
    for (int j = 0; j < header -> height; j++) {
        free(img[j]);
    }

    free(img);
}

int readNext(FILE* ptr) {
    int value;
    fscanf(ptr, "%d", &value);

    return value;
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

void saveResults(int* results, int m, const char* filename) {
    FILE* ptr = fopen(filename, "w");
    for (int i = 0; i < m; i++) {
        fprintf(ptr, "%d %d\n", i, results[i]);
    }
}


