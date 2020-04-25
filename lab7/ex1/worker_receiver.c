#include "shared.h"

Order* orders;
int shArrayId;
int semaphoreId;

void cleanup() {
    detachSharedArray(orders); 
}

int main() {
    atexit(cleanup);
    shArrayId = getSharedArrayId();
    orders    = getOrders(shArrayId);
    semaphoreId = getSemaphore();

    while (true) {
        P(semaphoreId);
        printf("Receiver Pid: %d acquired semaphore... sleeping for 2sec...\n", getpid());
        sleep(2);
        printf("Receiever Pid: %d releasing semaphore...\n", getpid());
        V(semaphoreId);    
    }

    return 0;
}

