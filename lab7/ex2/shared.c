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
    int id = shm_open(SHM_KEY_ORDERS_ARRAY, O_RDWR | O_CREAT | O_EXCL, 0666); 
    if (id == -1) {
        printf("Failed to create array.\n");
        printError();
    }

    return id;
}

void detachSharedArray(Order* orders) {
    munmap(orders, sizeof(Order) * NO_MAX_ORDERS);
}

void deleteSharedArray() {
    shm_unlink(SHM_KEY_ORDERS_ARRAY);
}

int getSharedArrayId() {
    int id = shm_open(SHM_KEY_ORDERS_ARRAY, O_RDWR, 0666); 

    if (id == -1) {
        printf("Failed to fetch array descriptor...\n");
    }

    return id;
}

Order* getOrders(int id) {
    ftruncate(id, sizeof(Order) * NO_MAX_ORDERS);
    Order* orders = mmap(NULL, sizeof(Order) * NO_MAX_ORDERS,
            PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);
    if (orders == (void *) -1) {
        printf("Failed t fetch orders..\n");
        printError();
    }

    return orders;
}

// Shared counter..
int getSharedCounterId() {
    return shm_open(SHM_KEY_COUNTER, O_RDWR, 0666); 
}

int createSharedCounter() {
    return shm_open(SHM_KEY_COUNTER, O_RDWR | O_CREAT | O_EXCL, 0666); 
}

void detachSharedCounter(Counter* counter) {
    munmap(counter, sizeof(Counter));
}

void deleteSharedCounter() {
    shm_unlink(SHM_KEY_COUNTER);
} 

Counter* getCounter(int id) {
    ftruncate(id, sizeof(Counter));
    Counter* counter = mmap(NULL, sizeof(Counter),
            PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);
    if (counter == (void *) -1) {
        printf("Failed t fetch orders..\n");
        printError();
    }

    return counter;
}

// Semaphore..
sem_t* createSemaphore() {
    return sem_open(SEMAPHORE_KEY, O_CREAT | O_EXCL, 0666, 1);
} 

void closeSemaphore(sem_t* sem) {
    sem_close(sem);
}

void deleteSemaphore() {
    sem_unlink(SEMAPHORE_KEY);
}

sem_t* getSemaphore() {
    return sem_open(SEMAPHORE_KEY, 0);
}

void P(sem_t* sem) {
    sem_wait(sem);
}

void V(sem_t* sem) {
    sem_post(sem);
};

