#include "utils.h"

#define MODE_NET "net"
#define MODE_LOCAL "local"

int sockfd = 0;
char buffer[MESSAGE_BUFFER_LEN]; 
char otherPlayersName[MAX_CLIENT_NAME];
char mark;

char* randPath = NULL;

struct sockaddr_un addr;

void cleanup() {
    puts("[Client] exits...");
    printf("-------------------------------------------------------------\n");
    if (randPath != NULL)
        unlink(randPath);


    close(sockfd);
}

void sighandler() {
    exit(EXIT_SUCCESS);
}

void netSocket(char* ipv4, int port) {
    struct sockaddr_in serv_addr; 

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
        error("Failed to create socket");
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, ipv4, &serv_addr.sin_addr) <= 0)  
        error("Invalid/not supported adress");
   
    // Bind to address
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Connection failed");
}

// 
void unixSocket(char* path) {
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);

    // autobind
    randPath = randomString(12);
    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, randPath, 108);
    bind(sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr));

    int opt = 1;  
    setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, (char *)&opt, sizeof(opt));
    if (sockfd < 0)
        error("Failed to create socket");
    
    if (connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) 
        error("Failed to establish connection");
} 

int main(int charc, char* argv []) {
    srand(time(NULL));
    if (charc < 3)
        error("Not enaugh arguments");

    char* name = argv[1];
    char* mode = argv[2];

    atexit(cleanup);
    signal(SIGINT, sighandler);
    printf("\n");

    // connect using either socket
    if (stringEq(mode, MODE_NET)) {
        if (charc < 5)
            error("Not enaugh arguments");

        char* ipv4 = argv[3];
        int   port = atoi(argv[4]);
        netSocket(ipv4, port);
    } else if (stringEq(mode, MODE_LOCAL)) {
        if (charc < 4)
            error("Not enaugh arguments");
        
        char* path = argv[3];
        unixSocket(path);
    } else 
        error("Unknown option");

    // once connected handle game logic
    nameMessage(buffer, name);
    if (send(sockfd, buffer, strlen(buffer), MSG_CONFIRM) <= 0)
        error("Failed to establish connection");

    fd_set s_rd;
    int stdinfd = fileno(stdin);
    char header [64];
    Board* board;
    while (1) {
        FD_ZERO(&s_rd);
        FD_SET(stdinfd, &s_rd);
        FD_SET(sockfd, &s_rd);
        int max = (stdinfd > sockfd) > stdinfd ? stdinfd : sockfd;
        select(max + 1, &s_rd, NULL, NULL, NULL);

        // check for server updates
        if (FD_ISSET(sockfd, &s_rd)) {
            int bytesRead = recv(sockfd, buffer, MESSAGE_BUFFER_LEN, MSG_WAITALL);
            buffer[bytesRead] = '\0';
            getHeader(buffer, header);

            // here happens most of the game logic
            if (stringEq(header, MESSAGE_MOVE)) {
                int mv;
                parseMoveMessage(buffer, &mv);
                move(board, mv);

                int gameov = gameover(board);
                if (gameov == 1) {
                    sprintf(buffer, "%s won!", otherPlayersName);
                    show(board, buffer);
                    exit(EXIT_SUCCESS);
                } else if (gameov == -1) {
                    show(board, "Tie!");
                    exit(EXIT_SUCCESS);
                } else {
                    show(board, "Your turn, type digit [0-8]");
                }

            } else if (stringEq(header, MESSAGE_WAIT)) {
                puts("[Client] waiting for another player to join game");
            } else if (stringEq(header, MESSAGE_NOT_UNIQUE)) {
                puts("[Client] Your name is not unique!");
                exit(EXIT_SUCCESS);
            } else if (stringEq(header, MESSAGE_PLAY)) {
                parsePlayMessage(buffer, otherPlayersName, &mark);
                printf("----------------------- Game --------------------------------\n ");
                printf("\t %s (%c) \t vs \t %s (%c) \n",
                       name, mark, otherPlayersName, opposite(mark));
                printf("-------------------------------------------------------------\n ");

                board = newBoard();
                if (mark == MV_CHR_CIRCLE) {
                    puts("Your turn, type digit [0-8]");
                } else {
                    printf("%s's turn\n", otherPlayersName);
                }
                printBoard(board);
            } else if (stringEq(header, MESSAGE_PING)) {
                // ping back
                notificationMessage(buffer, MESSAGE_PING_BACK);
                send(sockfd, buffer, strlen(buffer), 0);
            } else if (stringEq(header, MESSAGE_SERVER_SHUTDOWN) || bytesRead == 0) {
                printf("Server shut down or connection timed out.. \n");
                exit(EXIT_SUCCESS);
            } else if (stringEq(header, MESSAGE_OP_QUIT)) {
                printf("Opponent quit game.. \n");
                exit(EXIT_SUCCESS);
            }
        }

        // check for input from stdin
        int mv;
        if (FD_ISSET(stdinfd, &s_rd)) {
            read(stdinfd, buffer, MESSAGE_BUFFER_LEN);
            // validate user input
            if (board == NULL) {
                clearline();
            } else if (mark != board -> nextMove) {
                clearline();
                show(board, "Wait for your turn!");
            } else {
                clearline();
                int x = sscanf(buffer, "%d", &mv);
                if (x <= 0) {
                    show(board, "Enter valid int between [0-8], try again..");
                } else {
                    if (validateMove(board, buffer, mv) == 0) {
                        move(board, mv);
                        moveMessage(buffer, mv);
                        send(sockfd, buffer, strlen(buffer), 0);

                        sprintf(buffer, "%s's turn", otherPlayersName);
                        show(board, buffer);

                        int gameov = gameover(board);
                        if (gameov == 1) {
                            show(board, "You won!");
                            exit(EXIT_SUCCESS);
                        } else if (gameov == -1) {
                            show(board, "Tie!");
                            exit(EXIT_SUCCESS);
                        }
                    } else {
                        show(board, "Enter valid int between [0-8], try again..");
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}