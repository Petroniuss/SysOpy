#include "utils.h"

void error(const char* msg) {
    printf("Error: %s\n", msg);
    if (errno != 0) {
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
    }

    exit(EXIT_FAILURE);
}

void show(Board* board, const char* comment) {
    clear();
    puts(comment);
    printBoard(board);
}

void clear() {
    for (int i = 0; i < 4; i++) {
        clearline();
    }
}

void clearline() {
    printf("\033[A\33[2K\r");
}

Board* newBoard() {
    Board* board = malloc(sizeof(Board));
    for (int i = 0; i < 9; i++) {
        board -> mv[i / 3][i % 3] = MV_BLANK;
    }
    board -> nextMove = MV_CHR_CIRCLE;

    return board;
}

//  msg format: ("move" - str) (position - int)
void move(Board* board, int position) {
    board -> mv[position / 3][position % 3] = chhToMove(board -> nextMove);
    board -> nextMove = opposite(board -> nextMove);
}

// x x o
// - - o 
// x x o
void printBoard(Board* board) {
    printf("\t\t");
    for (int i = 0; i < 9; i++) {
        int x  = board -> mv[i / 3][i % 3];
        char mv = mvToChar(x);
        printf("%c", mv);
        if (i % 3 == 2)
            printf("\n");
        else
            printf(" ");

        if (i % 3 == 2 && i != 8)
            printf("\t\t");
    }
}

char mvToChar(int mv) {
    if (mv == MV_BLANK) 
        return MV_CHR_BLANK;
    else if (mv == MV_CIRCLE)
        return MV_CHR_CIRCLE;
    else if (mv == MV_CROSS)
        return MV_CHR_CROSS; 
    else 
        error("Bad mv..");

    return 'x';
}

int chhToMove(char v) {
    if (v == MV_CHR_BLANK) 
        return MV_BLANK; 
    else if (v == MV_CHR_CIRCLE)
        return MV_CIRCLE;
    else if (v == MV_CHR_CROSS)
        return MV_CROSS;
    else
        error("Sth bad happend");
    
    return -1;
}

char opposite(char mv) {
    if (mv == MV_CHR_CIRCLE)
        return MV_CHR_CROSS;
    else 
        return MV_CHR_CIRCLE;
}

// -1 for invalid, 0 for valid
int validateMove(Board* board, char* errorBuff, int mv) {
    if (mv < 0 || mv >= 9)
        sprintf(errorBuff, "%s", "Move must be between 0 and 9");
    else if (board -> mv[mv / 3][mv % 3] != MV_BLANK)
        sprintf(errorBuff, "%s", "This position is already taken");
    else 
        return 0;
    
    return -1;
}

// should be performed after move
bool gameover(Board* board) {
    int x = chhToMove(opposite(board -> nextMove));
    int matched = 0;

    // Check rows
    for (int j = 0; j < 3; j++) {
        matched = 0;
        for (int i = 0; i < 3; i++) {
            int y = board -> mv [j][i];
            if (x != y)
                break;
            else 
                matched += 1;
        }
        if (matched == 3)
            return true;
    }

    // Check columns
    for (int j = 0; j < 3; j++) {
        matched = 0;
        for (int i = 0; i < 3; i++) {
            int y = board -> mv [i][j];
            if (x != y)
                break;
            else 
                matched += 1;
        }
        if (matched == 3)
            return true;
    }

    // Check diagonals
    matched = 0;
    for (int i = 0; i < 3; i++) {
        int y = board -> mv [2 - i][i];
        if (x != y)
            break;
        else 
            matched += 1;
    }
    if (matched == 3)
        return true;

    matched = 0;
    for (int i = 0; i < 3; i++) {
        int y = board -> mv [i][i];
        if (x != y)
            break;
        else 
            matched += 1;
    }
    if (matched == 3)
        return true;

    return false;
}

void notificationMessage(char* buffer, char* notification) {
    sprintf(buffer, "%s ;", notification);
}

void nameMessage(char* buffer, char* name) {
    sprintf(buffer, "%s %s ;", MESSAGE_NAME, name);
}

void moveMessage(char* buffer, int move) {
    sprintf(buffer, "%s %d ;", MESSAGE_MOVE, move);
}

void playMessage(char* buffer, char* name, char mark) {
    sprintf(buffer, "%s %s %c ;", MESSAGE_PLAY, name, mark);
}

void parseMoveMessage(char* msg, int* ptr) {
    sscanf(msg, "%*s %d", ptr); 
}

void parsePlayMessage(char* msg, char* name, char* mark) {
    sscanf(msg, "%*s %s %c", name, mark); 
}

void parseNameMessage(char* buffer, char* nameBuff) {
    sscanf(buffer, "%*s %s", nameBuff); 
}

void getHeader(char* msg, char* headerBuffer) {
    sscanf(msg, "%s", headerBuffer); 
}

// Reads until ';', returns 0 if message couldnt be read which means host disconnected
int readWholeMessage(char* buffer, int sockfd) {
    char bu [2];
    int bytesRead = read(sockfd, bu, 1);
    
    int i = 0;
    while (bu[0] != ';' && bytesRead != 0) {
        buffer[i++] = bu[0];
        bytesRead = read(sockfd, bu, 1);
    }

    if (bytesRead != 0)
        buffer[i++] = bu[0];
    
    buffer[i++] = '\0';

    if (bytesRead == 0)
        return 0;
    else return 1;
}

char crossCircle() {
    if(rand() % 2 == 0)
        return MV_CHR_CIRCLE;
    else 
        return MV_CHR_CROSS;
}

bool stringEq(char* a, char* b) {
    if (strcmp(a, b) == 0)
        return true;
    else
        return false;
}
