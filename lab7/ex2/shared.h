#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/*
   Let's define common iterface so that this will be the only
   thing that changes!


   One semaphore:
        - binary for accessing resource (array) (done..)

   Shared memory:
        - array aka orders (done..)
        - counters (done..)

*/

#define NO_MAX_ORDERS 100
#define SHM_KEY_ORDERS_ARRAY "/orders-shm"
#define SHM_KEY_COUNTER "/counter-shm"
#define SEMAPHORE_KEY "/key-semaphore"

#define WORKER_TYPE_SENDER "SENDER"
#define WORKER_TYPE_PACKER "PACKER"
#define WORKER_TYPE_RECEIVER "RECEIVER"

#define NO_WORKER_PACKER 5
#define NO_WORKER_RECEIVER 10
#define NO_WORKER_SENDER 8
#define NO_TOTAL_WORKERS                                                       \
  (NO_WORKER_SENDER + NO_WORKER_PACKER + NO_WORKER_RECEIVER)

#define TIME_BUFFER_LENGTH 84
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

struct Order {
  int  num;
  bool packed;
} typedef Order;

struct Counter {
  int orders_waiting;
  int orders_packed;
} typedef Counter;

// Task specific utils...
Order newOrder(int num);
void  printOrder(Order order);
void  printOrders(Order* orders);
void  printLog(char* type, int i, char* msg);
int   freeSpaces(Counter* counter);
int   findNextEmpty(int startI, Order* orders);
int   findNextWaiting(int startI, Order* orders);
int   findNextPacked(int startI, Order* orders);

// General utils...
char* currentTime(char* buffer);
bool  stringEq(char* str1, char* str2);
char* randomString(int length);
void  printError();

//--------------------------- POSIX -----------------------------

// Shared array...
int    getSharedArrayId();
int    createSharedArray();
void   detachSharedArray(Order* orders);
void   deleteSharedArray();
Order* getOrders(int id);

// Shared counter ..
int      getSharedCounterId();
int      createSharedCounter();
void     detachSharedCounter(Counter* counter);
void     deleteSharedCounter();
Counter* getCounter(int id);

// Seamphore..
sem_t* createSemaphore();
void   deleteSemaphore();
void   closeSemaphore(sem_t* sem);
sem_t* getSemaphore();

void P(sem_t* sem);
void V(sem_t* sem);
