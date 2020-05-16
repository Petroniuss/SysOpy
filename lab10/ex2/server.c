#include "utils.h"

// ping every second
int pingEvery = 1;
int netSockfd, unixSockfd;
char* path;
Player** players;

// server address
struct sockaddr_in address;
struct sockaddr_un addr;

// client addresses
struct sockaddr_in cliNetAddr  [MAX_CLIENTS];
struct sockaddr_un cliUnixAddr [MAX_CLIENTS];
int occupied [MAX_CLIENTS]; // 0 free, 1 net, 2 unix

pthread_mutex_t mutex;
char buffer [MESSAGE_BUFFER_LEN];

// sends message stored in buffer to ith client
void sendBuff(int i) {
    socklen_t len;
    if (occupied[i] == 1) {
        len = sizeof(cliNetAddr[i]);
        sendto(netSockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr*) &cliNetAddr[i], len);
    } else {
        len = sizeof(cliUnixAddr[i]);
        sendto(unixSockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr*) &cliUnixAddr[i], len);
    }
}


// This way we differntiate clinets!
// check for net addr equality
bool netAddrEq(struct sockaddr_in a, struct sockaddr_in b) {
    return a.sin_port == b.sin_port;
}

// check for unix addr equality
bool unixAddrEq(struct sockaddr_un a, struct sockaddr_un b) {
    return stringEq(a.sun_path, b.sun_path); 
}


int findNetAddr(struct sockaddr_in addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (occupied[i] && netAddrEq(addr, cliNetAddr[i]))
            return i;
    }

    return -1;
}

int findUnixAddr(struct sockaddr_un addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (occupied[i] && unixAddrEq(addr, cliUnixAddr[i]))
            return i;
    }

    return -1;
}

int findEmptySpot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!occupied[i])
            return i;
    }

    error("Maximum capacity reached!");
    return -1;
}

void handleDisconnect(int i) {
    printf("[Server] Client %s(%d) disconnected\n", players[i] -> name, i);
        
    if (players[i] -> inGame) {
        int opponent = players[i] -> other;
        // check if opponent hasn't already left
        if (occupied[opponent] > 0) {
            notificationMessage(buffer, MESSAGE_OP_QUIT);
            sendBuff(opponent);
        }
    }

    // mark for reuse 
    occupied[i] = 0;
    free(players[i] -> name);

    players[i] -> name = NULL;
    players[i] -> inGame = false;
    players[i] -> other = -1;
    players[i] -> wasPinged = false;
    players[i] -> pingedBack = false;
}

void* pingThread(void* arg) {
    while (1) {
        printf("[Ping Thread] - sending pings...\n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (occupied[i] > 0 ) {
                Player* player = players[i];
                // check if client responded to ping
                if (player -> wasPinged && !player -> pingedBack) {
                    printf("[Ping Thread] Client %s(%d) timed out\n", players[i] -> name, i);
                    handleDisconnect(i);
                } else {
                    notificationMessage(buffer, MESSAGE_PING);
                    sendBuff(i);
                    player -> wasPinged = true;
                    player -> pingedBack = false;
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(pingEvery);
    }

    return NULL;
}

void sighandler() {
    exit(EXIT_SUCCESS);
}

void cleanup() {
    // send shutdown message and close connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (occupied[i] > 0) {
            notificationMessage(buffer, MESSAGE_SERVER_SHUTDOWN);
            sendBuff(i);
        }
        free(players[i]);
    }

    unlink(path);
    free(players);
    close(netSockfd);
    close(unixSockfd);

    printf("\n[Server] shutdwon -> closed all connections\n");
}

int findOpponent(int j) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (occupied[i] > 0 && i != j && players[i] -> name && !players[i] -> inGame)
            return i;
    }

    return -1;
}

bool uniqueName(char* name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (players[i] -> name && stringEq(players[i] -> name, name))
            return false;
    }

    return true;
}


int main(int charc, char* argv[]) {
    if (charc < 3) 
        error("Not enaugh arguments!");

    // init datastructure
    players        = malloc(MAX_CLIENTS * sizeof(Player*));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        players[i] = malloc(sizeof(Player));
        players[i] -> name = NULL;
        players[i] -> inGame = false;
        players[i] -> wasPinged = false;
        players[i] -> pingedBack = false;
        occupied[i] = 0;

        memset(&cliNetAddr, 0, sizeof(cliNetAddr)); 
        memset(&cliUnixAddr, 0, sizeof(cliUnixAddr)); 
    }

    atexit(cleanup);
    signal(SIGINT, sighandler);

    // set of socket descriptors
    fd_set readfds;
    int port = atoi(argv[1]);
    path = argv[2];

    if (port < 1024) 
        error("Port number must be greater or equal 1024");

    // address for unix socket
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    // address for net socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // bind socket
    netSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // This is to handle situation when client does not close his socket after server shut down 
    int opt = 1;  
    setsockopt(netSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *)&opt, sizeof(opt));

    if (netSockfd == -1) 
        error("Failed to create socket");

    if ((bind(netSockfd, (struct sockaddr*) &address, sizeof(address))) != 0)
        error("Binding socket failed");

    printf("[Server] INET socket started on ipv4: %s (localhost), port: %d\n",
            "127.0.0.1", ntohs(address.sin_port));

    // bind unix socket
    unixSockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (unixSockfd == -1) 
        error("Failed to create unix socket");

    if ((bind(unixSockfd, (struct sockaddr*) &addr, sizeof(addr))) != 0)
        error("Binding unix socket failed");

    printf("[Server] UNIX socket, receiving on path: %s \n", path);
    puts("[Server] Waiting for connections");

    // start pinging thread
    pthread_t thr;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&thr, NULL, pingThread, NULL);

    // Actual server logic down here
    int max_sd;
    char header [64];

    struct sockaddr_in cliIn;
    struct sockaddr_un cliUn;
    socklen_t addrLen;

    while (1) {
        // Initialize set 
        max_sd = (netSockfd > unixSockfd) ? netSockfd : unixSockfd;

        FD_ZERO(&readfds);
        FD_SET(netSockfd, &readfds);
        FD_SET(unixSockfd, &readfds);

        // Wait for any fd in set to become ready && lock sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); 
        pthread_mutex_lock(&mutex);
		if (activity < 0 && errno != EINTR) 		 
            error("Select error occured");

        // Check if there's connection on net socket
        if (FD_ISSET(netSockfd, &readfds)) {
            recvfrom(netSockfd, buffer, sizeof(buffer), MSG_WAITALL, 
                         (struct sockaddr*) &cliIn, &addrLen);

            int i = findNetAddr(cliIn);
            getHeader(buffer, header);
            // add new socket/player, we know this is name message
            if (i == -1 && stringEq(header, MESSAGE_NAME)) {
                i = findEmptySpot();
                cliNetAddr[i].sin_addr.s_addr = cliIn.sin_addr.s_addr;
                cliNetAddr[i].sin_family = cliIn.sin_family;
                cliNetAddr[i].sin_port = cliIn.sin_port;
                strcpy((char*) cliNetAddr[i].sin_zero, (char*) cliIn.sin_zero);

                char* name = malloc(sizeof(MAX_CLIENT_NAME));
                parseNameMessage(buffer, name);

                players[i] -> inGame = false;
                players[i] -> pingedBack = false;
                players[i] -> wasPinged = false;
                players[i] -> other = -1; 
                occupied[i] = 1; 
                // handle non unique name
                if (!uniqueName(name)) {
                    printf("[Server] Not unique name! %s\n", name);
                    notificationMessage(buffer, MESSAGE_NOT_UNIQUE);
                    sendBuff(i);
                    handleDisconnect(i);
                } else {
                    players[i] -> name = name;

                    int opponent = findOpponent(i);
                    if (opponent == -1) {
                        // send wait message
                        notificationMessage(buffer, MESSAGE_WAIT);
                        sendBuff(i);
                        printf("[Server] Sent wait message to player %s(%d)\n", name, i);
                    } else {
                        // start game 
                        char mark = crossCircle();
                        char oppositeMark = opposite(mark);

                        players[i] -> other = opponent;                 
                        players[opponent] -> other = i;

                        players[i] -> inGame = true;                 
                        players[opponent] -> inGame = true;

                        printf("[Server] Starting game between %s(%d) and %s(%d)\n",
                            players[i] -> name, i, players[opponent] -> name, opponent);

                        // send players play message 
                        playMessage(buffer, players[opponent] -> name, mark);
                        sendBuff(i);

                        playMessage(buffer, players[i] -> name, oppositeMark);
                        sendBuff(opponent);
                    }
                } 
            // handle other messages
            } else if (stringEq(header, MESSAGE_MOVE)) {
                // forward message
                int opponent = players[i] -> other;
                sendBuff(opponent);
            } else if(stringEq(header, MESSAGE_PING_BACK)) {
                players[i] -> pingedBack = true;
            } else {
                printf("[Server] Unknown message %s \n", buffer);
            }
        }

        // Else check if there's connection on unix socket
        if (FD_ISSET(unixSockfd, &readfds)) {
            recvfrom(unixSockfd, buffer, sizeof(buffer), 0, 
                         (struct sockaddr*) &cliUn, &addrLen);

            int i = findUnixAddr(cliUn);
            getHeader(buffer, header);
            // add new socket/player, we know this is name message
            if (i == -1 && stringEq(header, MESSAGE_NAME)) {
                i = findEmptySpot();
                cliUnixAddr[i].sun_family = cliUn.sun_family;
                strcpy(cliUnixAddr[i].sun_path, cliUn.sun_path);

                char* name = malloc(sizeof(MAX_CLIENT_NAME));
                parseNameMessage(buffer, name);

                players[i] -> inGame = false;
                players[i] -> pingedBack = false;
                players[i] -> wasPinged = false;
                players[i] -> other = -1; 
                occupied[i] = 2; 
                // handle non unique name
                if (!uniqueName(name)) {
                    notificationMessage(buffer, MESSAGE_NOT_UNIQUE);
                    sendBuff(i);
                    handleDisconnect(i);
                } else {
                    players[i] -> name = name;

                    int opponent = findOpponent(i);
                    if (opponent == -1) {
                        // send wait message
                        notificationMessage(buffer, MESSAGE_WAIT);
                        sendBuff(i);
                        printf("[Server] Sent wait message to player %s(%d)\n", name, i);
                    } else {
                        // start game 
                        char mark = crossCircle();
                        char oppositeMark = opposite(mark);

                        players[i] -> other = opponent;                 
                        players[opponent] -> other = i;

                        players[i] -> inGame = true;                 
                        players[opponent] -> inGame = true;

                        printf("[Server] Starting game between %s(%d) and %s(%d)\n",
                            players[i] -> name, i, players[opponent] -> name, opponent);

                        // send players play message 
                        playMessage(buffer, players[opponent] -> name, mark);
                        sendBuff(i);

                        playMessage(buffer, players[i] -> name, oppositeMark);
                        sendBuff(opponent);
                    }
                } 
            // handle other messages
            } else if (stringEq(header, MESSAGE_MOVE)) {
                // forward message
                int opponent = players[i] -> other;
                sendBuff(opponent);
            } else if(stringEq(header, MESSAGE_PING_BACK)) {
                players[i] -> pingedBack = true;
            } else if (stringEq(header, MESSAGE_QUIT)) {
                handleDisconnect(i);
            } else {
                printf("[Server] Unknown message %s \n", buffer);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    return EXIT_SUCCESS;
}