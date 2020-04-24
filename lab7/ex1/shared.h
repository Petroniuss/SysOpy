#define _XOPEN_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

/*
   Let's define common iterface so that this will be the only
   thing that changes!
*/

#define NO_MAX_ORDERS 10
#define SHM_KEY_ORDERS_ARRAY (ftok(getenv("HOME"), 12))
#define SHM_KEY_COUNTER (ftok(getenv("HOME"), 124))
#define SEMAPHORE_KEY (ftok(getenv("HOME"), 64))

#define NO_WORKER_PACKER 2
#define NO_WORKER_RECEIVER 1
#define NO_WORKER_SENDER 1
#define NO_TOTAL_WORKERS (NO_WORKER_SENDER + NO_WORKER_PACKER + NO_WORKER_RECEIVER)

/*
   One semaphore:
        - binary for accessing resoure (array)

   Shared memory:
        - array aka orders (done..)
        - counters (done..)

*/

struct Order {
    int  num;
    bool packed;
} typedef Order;

struct Counter {
    int  orders_waiting;
    int  orders_packed;
} typedef Counter;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short* array;
} arg;

// Task specific utils...
Order newOrder(int num);
void printOrder(Order order);
int  freeSpaces(Counter* counter);

// General utils...
char*  currentTime(char* buffer);
bool stringEq(char* str1, char* str2);
char* randomString(int length);
void printError();

//--------------------------- SYSTEM V ----------------------------- 

// Shared array...
int getSharedArrayId();
int createSharedArray();
void detachSharedArray(Order* orders);
void deleteSharedArray(int id); 
Order* getOrders(int id);

// Shared counter ..
int getSharedCounterId();
int createSharedCounter();
void detachSharedCounter(Counter* counter);
void deleteSharedCounter(int id); 
Counter* getCounter(int id);

// Seamphore..
int createSemaphore(); 
void deleteSemaphore(int semaphoreId);
int getSemaphore();

void P(int semaphoreId);
void V(int semaphoreId);



















