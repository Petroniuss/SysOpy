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


   One semaphore:
        - binary for accessing resource (array) (done..)

   Shared memory:
        - array aka orders (done..)
        - counters (done..)

*/

#define NO_MAX_ORDERS 10
#define SHM_KEY_ORDERS_ARRAY (ftok(getenv("HOME"), 12))
#define SHM_KEY_COUNTER (ftok(getenv("HOME"), 124))
#define SEMAPHORE_KEY (ftok(getenv("HOME"), 64))

#define WORKER_TYPE_SENDER "SENDER"
#define WORKER_TYPE_PACKER "PACKER"
#define WORKER_TYPE_RECEIVER "RECEIVER"

#define NO_WORKER_PACKER 2
#define NO_WORKER_RECEIVER 5
#define NO_WORKER_SENDER 1
#define NO_TOTAL_WORKERS (NO_WORKER_SENDER + NO_WORKER_PACKER + NO_WORKER_RECEIVER)

#define TIME_BUFFER_LENGTH 84
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


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
void printLog(char* type, char* msg);
int  freeSpaces(Counter* counter);
int  findNextEmpty(int startI, Order* orders);
int  findNextUnpacked(int startI, Order* orders);
int  findNextPacked(int startI, Order* orders);

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




