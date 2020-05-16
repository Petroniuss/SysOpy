#include "utils.h"

int* clientsSockets;
int pingEvery = 3;
Player** players;

int netSockfd, unixSockfd;
char* path;
struct sockaddr_in address;
struct sockaddr_un addr;

pthread_mutex_t mutex;
char buffer [MESSAGE_BUFFER_LEN];

void handleDisconnect(int i) {
    int sd = clientsSockets[i];
    int addrlen = sizeof(address);
    getpeername(sd , (struct sockaddr*) &address ,(socklen_t*) &addrlen);
    printf("[Server] Client %s(%d) disconnected\n", players[i] -> name, i);
        
    if (players[i] -> inGame) {
        int opponent = players[i] -> other;
        // check if opponent hasn't already left
        if (clientsSockets[opponent] != 0) {
            notificationMessage(buffer, MESSAGE_OP_QUIT);
            send(clientsSockets[opponent], buffer, strlen(buffer), 0);
        }
    }

    // Close the socket and mark for reuse 
    close(sd); 
    free(players[i] -> name);

    clientsSockets[i] = 0; 
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
            if (clientsSockets[i] > 0 ) {
                Player* player = players[i];
                // check if client responded to ping
                if (player -> wasPinged && !player -> pingedBack) {
                    printf("[Ping Thread] Client %s(%d) timed out\n", players[i] -> name, i);
                    handleDisconnect(i);
                } else {
                    notificationMessage(buffer, MESSAGE_PING);
                    send(clientsSockets[i], buffer, strlen(buffer), 0);
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
    for (int i = 0; i < MAX_CLIENTS; i++) {
        free(players[i]);
        int fd = clientsSockets[i];
        notificationMessage(buffer, MESSAGE_SERVER_SHUTDOWN);
        send(fd, buffer, strlen(buffer), 0);
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }

    unlink(path);

    free(players);
    free(clientsSockets);

    close(netSockfd);
    close(unixSockfd);

    puts("");
    printf("[Server] shutdwon -> closed all connections\n");
}

int findOpponent(int j) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientsSockets[i] > 0 && i != j && players[i] -> name && !players[i] -> inGame)
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

    clientsSockets = calloc(MAX_CLIENTS, sizeof(int));
    players        = malloc(MAX_CLIENTS * sizeof(Player*));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        players[i] = malloc(sizeof(Player));
        players[i] -> name = NULL;
        players[i] -> inGame = false;
        players[i] -> wasPinged = false;
        players[i] -> pingedBack = false;
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

    // bind & listen net socket
    netSockfd = socket(AF_INET, SOCK_STREAM, 0);
    // This is to handle situation when client does not close his socket after server shut down 
    int opt = 1;  
    setsockopt(netSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *)&opt, sizeof(opt));

    if (netSockfd == -1) 
        error("Failed to create socket");

    if ((bind(netSockfd, (struct sockaddr*) &address, sizeof(address))) != 0)
        error("Binding socket failed");

    if ((listen(netSockfd, MAX_WAITING_CONNECTIONS)) != 0)   
        error("Listen failed");
    
    printf("[Server] INET socket started on ipv4: %s (localhost), port: %d\n",
            "127.0.0.1", ntohs(address.sin_port));

    // bind & listen unix socket
    unixSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unixSockfd == -1) 
        error("Failed to create unix socket");

    if ((bind(unixSockfd, (struct sockaddr*) &addr, sizeof(addr))) != 0)
        error("Binding unix socket failed");

    if ((listen(unixSockfd, MAX_WAITING_CONNECTIONS)) != 0)   
        error("Listen unix socket failed");

    printf("[Server] UNIX socket, listening on path: %s \n", path);

    int addrlen = sizeof(address);
    puts("[Server] Waiting for connections");

    // start pinging thread
    pthread_t thr;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&thr, NULL, pingThread, NULL);

    // Actual server logic down here
    int sd, max_sd, newSocket;
    char header [64];
    while (1) {
        // Initialize set 
        max_sd = (netSockfd > unixSockfd) ? netSockfd : unixSockfd;

        FD_ZERO(&readfds);
        FD_SET(netSockfd, &readfds);
        FD_SET(unixSockfd, &readfds);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clientsSockets[i];

            if (sd > 0)
                FD_SET(sd, &readfds);
            
            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for any fd in set to become ready && lock sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); 
        pthread_mutex_lock(&mutex);
		if (activity < 0 && errno != EINTR) 		 
            error("Select error occured");

        // Check if there's connection on net socket
        if (FD_ISSET(netSockfd, &readfds)) {
            newSocket = accept(netSockfd, (struct sockaddr*) &address, (socklen_t*)&addrlen);
            if (newSocket == -1) 
                error("Accept connection error");

			printf("[Server] New connection, socket fd is %d, ip is : %s, port : %d \n",
                   newSocket , inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port)); 
		
            // add new socket/player
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientsSockets[i] == 0) {
                    clientsSockets[i] = newSocket;
                    break;
                }
            }
        }

        // Else check if there's connection on unix socket
        if (FD_ISSET(unixSockfd, &readfds)) {
            newSocket = accept(unixSockfd, NULL, NULL);
            if (newSocket == -1) 
                error("Accept connection error");

			printf("[Server] New connection on unix socket, socket fd is %d\n", newSocket);
		
            // add new socket/player
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientsSockets[i] == 0) {
                    clientsSockets[i] = newSocket;
                    break;
                }
            }
        }

        // else check client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clientsSockets[i];

            // handle various client messages
            if (FD_ISSET(sd, &readfds)) {
                // here we should read until we reach ;marker
                int bytesRead = readWholeMessage(buffer, sd);
                getHeader(buffer, header);

                // handle disconnect
                if (bytesRead <= 0 || stringEq(header, MESSAGE_QUIT)) {
                    handleDisconnect(i);
                } else if (stringEq(header, MESSAGE_NAME)) {
                    // player sends his name and waits for game to start
                    char* name = malloc(sizeof(MAX_CLIENT_NAME));
                    parseNameMessage(buffer, name);

                    // handle non unique name
                    if (!uniqueName(name)) {
                        notificationMessage(buffer, MESSAGE_NOT_UNIQUE);
                        send(sd, buffer, strlen(buffer), 0);
                        handleDisconnect(i);
                        continue;
                    } 

                    players[i] -> name = name;

                    int opponent = findOpponent(i);
                    if (opponent == -1) {
                        // send wait message
                        notificationMessage(buffer, MESSAGE_WAIT);
                        send(newSocket, buffer, strlen(buffer), 0);
                        printf("[Server] Sent wait message to player %s(%d)\n", name, i);
                    } else {
                        // start game 
                        char mark = crossCircle();
                        char oppositeMark = opposite(mark);

                        int playerA = clientsSockets[i];
                        int playerB = clientsSockets[opponent];

                        players[i] -> other = opponent;                 
                        players[opponent] -> other = i;

                        players[i] -> inGame = true;                 
                        players[opponent] -> inGame = true;

                        printf("[Server] Starting game between %s(%d) and %s(%d)\n",
                            players[i] -> name, i, players[opponent] -> name, opponent);

                        // send players play message 
                        playMessage(buffer, players[opponent] -> name, mark);
                        send(playerA, buffer, strlen(buffer), 0);

                        playMessage(buffer, players[i] -> name, oppositeMark);
                        send(playerB, buffer, strlen(buffer), 0);
                    }

                } else if (stringEq(header, MESSAGE_MOVE)) {
                    // forward message
                    int opponent = players[i] -> other;
                    int opfd = clientsSockets[opponent];

                    send(opfd, buffer, strlen(buffer), 0);
                } else if(stringEq(header, MESSAGE_PING_BACK)) {
                    players[i] -> pingedBack = true;
                } else {
                    printf("[Server] Unknown message %s \n", buffer);
                }
            }
        }
        
        // unlock mutex so that other thread can send pings
        pthread_mutex_unlock(&mutex);
    }


    return EXIT_SUCCESS;
}