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
        printLog(WORKER_TYPE_PACKER, "Hey there!");
        sleep(1);
        V(semaphoreId);    
    }

    return 0;
}

