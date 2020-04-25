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
    char timeBuff [TIME_BUFFER_LENGTH];

    P(semaphoreId);
    printf("Pid: %d acquired semaphore... sleeping for 2sec...\n", getpid());
    sleep(1);
    currentTime(timeBuff);
    printf("Time:%s, Pid: %d releasing semaphore...\n", timeBuff, getpid());
    V(semaphoreId);    

    return 0;
}

