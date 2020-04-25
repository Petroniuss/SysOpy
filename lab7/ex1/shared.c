#include "shared.h"

Order newOrder(int num) {
  Order* order = malloc(sizeof(Order));

  order->num = num;
  order->packed = false;

  return *order;
}

void printLog(char* type, int i, char* msg) {
    int pid = getpid();
    char timeBuff [TIME_BUFFER_LENGTH];
    currentTime(timeBuff);

    printf("[%s %d%s, %s%s %s] %s%s%s:           %s %s(Order index: %d)%s\n",
            ANSI_COLOR_RED,  pid, ANSI_COLOR_RESET,
            ANSI_COLOR_YELLOW, timeBuff, ANSI_COLOR_RESET,
            ANSI_COLOR_GREEN, type, ANSI_COLOR_RESET, msg,
            ANSI_COLOR_MAGENTA, i, ANSI_COLOR_RESET);
}

char* currentTime(char* buffer) {
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char buff [82];
  
    strftime(buff, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    sprintf(buffer, "%s:%03d", buff, milli);

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

void printOrders(Order* orders) {
    printf("[ ");
    for (int i = 0; i < NO_MAX_ORDERS; i++) {
        printf("%d ", orders[i].num);    
    }
    printf("]\n");
}

void printOrder(Order order) {
    printf("Order { num: %d, packed: %d }\n", order.num, order.packed);
}

int freeSpaces(Counter* counter) {
    return NO_MAX_ORDERS - (counter -> orders_packed + counter -> orders_waiting);
}

int  findNextEmpty(int startI, Order* orders) {
    int i = startI;
    while (true) {
        if (orders[i].num == 0) {
            return i;
        }

        i = (i + 1) % NO_MAX_ORDERS;
    }
}

int  findNextWaiting(int startI, Order* orders) {
    int i = startI;
    while (true) {
        if (!orders[i].packed && orders[i].num != 0) {
            return i;
        }

        i = (i + 1) % NO_MAX_ORDERS;
    }
}

int  findNextPacked(int startI, Order* orders) {
    int i = startI;
    while (true) {
        if (orders[i].packed) {
            return i;
        }

        i = (i + 1) % NO_MAX_ORDERS;
    }
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
    return shmget(SHM_KEY_COUNTER, sizeof(Counter), 0666 | IPC_CREAT);
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

