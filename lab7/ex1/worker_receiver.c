#include "shared.h"

Order* orders;
int shArrayId;

void cleanup() {
    detachSharedArray(orders); 
}

int main() {
    atexit(cleanup);
    shArrayId = getSharedArrayId();
    orders    = getOrders(shArrayId);

    return 0;
}

