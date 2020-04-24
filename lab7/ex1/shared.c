#include "shared.h"

Order newOrder(int num) {
  Order* order = malloc(sizeof(Order));

  order->num = num;
  order->packed = false;

  return *order;
}

char* currentTime(char* buffer) {
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char buff [82];
  
    strftime(buff, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    sprintf(buffer, "%s:%03d", buffer, milli);

    return buffer;
}

bool stringEq(char* str1, char* str2) {
    return strcmp(str1, str2);
}

char* randomString(int length) {
    char* str = calloc(length + 1, sizeof(char));
    
    for (int i = 0; i < length; i++) {
        char randomLetter = 'a' + (rand() % 26);
        str[i] = randomLetter;
    }

    str[length] = '\0';

    return str;
}

void printError() {
    if (errno != 0) {
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
    }
}

void printOrder(Order order) {
    printf("Order { num: %d, packed: %d }\n", order.num, order.packed);
}

int freeSpaces(Counter* counter) {
    return NO_MAX_ORDERS - (counter -> orders_packed + counter -> orders_waiting);
}

//--------------------------- SYSTEM V ----------------------------- 

// Shared  array..
int createSharedArray() {
    int id = shmget(SHM_KEY_ORDERS_ARRAY, sizeof(Order) * NO_MAX_ORDERS, 0666 | IPC_CREAT);
    if (id == -1) {
        printf("Failed to create array.\n");
    }

    return id;
}

void detachSharedArray(Order* orders) {
    shmdt(orders);
}

void deleteSharedArray(int id) {
    shmctl(id, IPC_RMID, NULL);
}

int getSharedArrayId() {
    int id = shmget(SHM_KEY_ORDERS_ARRAY, 0, 0666);

    if (id == -1) {
        printf("Failed to fetch array descriptor...\n");
    }

    return id;
}

Order* getOrders(int id) {
    Order* orders = shmat(id, NULL, 0);
    if (orders == (void *) -1) {
        printf("Failed t fetch orders..\n");
    }

    return orders;
}

// Shared counter..
int getSharedCounterId() {
    return shmget(SHM_KEY_COUNTER, 0 , 0666);
}

int createSharedCounter() {
    return shmget(SHM_KEY_ORDERS_ARRAY, sizeof(Counter), 0666 | IPC_CREAT);
}

void detachSharedCounter(Counter* counter) {
    shmdt(counter);
}

void deleteSharedCounter(int id) {
    shmctl(id, IPC_RMID, NULL);
} 

Counter* getCounter(int id) {
    return shmat(id, NULL, 0);
}

// Semaphore..
int createSemaphore() {
    int id = semget(SEMAPHORE_KEY, 1, 0666 | IPC_CREAT);

    union semun arg;
    arg.val = 1;
    semctl(id, 0, SETVAL, arg);

    return id;
} 

void deleteSemaphore(int semaphoreId) {
    semctl(semaphoreId, 0, IPC_RMID);
}

int getSemaphore() {
    return semget(SEMAPHORE_KEY, 0, 0666);
}

void P(int semaphoreId) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;

    struct sembuf bufs[1];
    bufs[0] = buf;
    
    semop(semaphoreId, bufs, 1);
}

void V(int semaphoreId) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = 0;

    struct sembuf bufs[1];
    bufs[0] = buf;
    
    semop(semaphoreId, bufs, 1);
};
































