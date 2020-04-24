#include "shared.h"

/*
 * This piece of code is responsible for starting program and terminating it.
 * */

int    childrenPids [NO_TOTAL_WORKERS];

void handleExit(int signal) {
    printf("Terminating upon receiving signal %d...\n", signal);
    for (int i = 0; i < NO_TOTAL_WORKERS; i++) {
        kill(childrenPids[i], SIGINT); 
    }
}

int main() {
    int shArrayId = createSharedArray();
    int shCounterId = createSharedCounter();
    
    Counter* counter = getCounter(shCounterId);
    counter -> orders_packed = 0;
    counter -> orders_waiting = 0;

    Order* orders = getOrders(shArrayId);
    for (int i = 0; i < NO_MAX_ORDERS; i++) {
        orders[i] = newOrder(i);
    }

    int pidI = 0;
    for (int i = 0; i < NO_WORKER_RECEIVER; i++) {
        int pid = fork();

        if (pid == 0) {
            // kid
            execl("./worker_receiver", "./worker_receiver", NULL);
        } else {
            childrenPids[pidI++] = pid; 
        }
    } 

    for (int i = 0; i < NO_WORKER_PACKER; i++) {
        int pid = fork();

        if (pid == 0) {
            // kid
            execl("./worker_packer", "./worker_packer", NULL);
        } else {
            childrenPids[pidI++] = pid; 
        }

    } 

    for (int i = 0; i < NO_WORKER_SENDER; i++) {
        int pid = fork();

        if (pid == 0) {
            // kid
            execl("./worker_sender", "./worker_sender", NULL);
        } else {
            childrenPids[pidI++] = pid; 
        }

    } 

    signal(SIGINT, handleExit); 
    for (int i = 0; i < NO_TOTAL_WORKERS; i++) {
        waitpid(childrenPids[i], NULL, 0);    
    }


    detachSharedArray(orders);
    detachSharedCounter(counter);
    deleteSharedCounter(shCounterId);
    deleteSharedArray(shArrayId); 

    return 0;
}
