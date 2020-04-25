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
    shArrayId = getSharedArrayId();
    shCounterId = getSharedCounterId();
    orders    = getOrders(shArrayId);
    semaphoreId = getSemaphore();
    counter = getCounter(shCounterId);

    char buff [84];
    int i = 0;
    while (true) {
        P(semaphoreId);

        if (counter -> orders_packed > 0) {
            i = findNextPacked(i, orders);            
            
            orders[i].num *= 3;
            counter -> orders_packed -= 1;

            sprintf(buff, "Sent order of size: %d.. Orders to prepare: %d; orders to send %d", 
                    orders[i].num, counter -> orders_waiting, counter -> orders_packed);
            printLog(WORKER_TYPE_SENDER, i, buff);

            orders[i].num = 0;
            orders[i].packed = false;

            i = ((i + 1) % NO_MAX_ORDERS);
        } 

        V(semaphoreId);
    }

    return 0;
}

