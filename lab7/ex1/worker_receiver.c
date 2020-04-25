#include "shared.h"

Order* orders;
Counter* counter;
int shArrayId;
int shCounterId;
int semaphoreId;

void cleanup() {
    detachSharedArray(orders); 
    detachSharedCounter(counter);
}

int main() {
    atexit(cleanup);
    srand(time(NULL));
    shArrayId = getSharedArrayId();
    shCounterId = getSharedCounterId();
    orders    = getOrders(shArrayId);
    semaphoreId = getSemaphore();
    counter = getCounter(shCounterId);
    printError();

    char buff [84];
    int i = 0;
    while (true) {
        P(semaphoreId);

        if (freeSpaces(counter) > 0) {
            i = findNextEmpty(i, orders);            
            
            orders[i].num = (rand() % 1000) + 1;
            orders[i].packed = false;
            counter -> orders_waiting += 1;

            sprintf(buff, "Added number: %d.. Orders to prepare: %d; orders to send %d", 
                    orders[i].num, counter -> orders_waiting, counter -> orders_packed);
            printLog(WORKER_TYPE_RECEIVER, i, buff);

            i = ((i + 1) % NO_MAX_ORDERS);
        } 

        V(semaphoreId);
    }

    return 0;
}

