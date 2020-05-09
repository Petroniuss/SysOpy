#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

// Hypyer-parameters
int maxDelay = 3;
int maxHairCutTime = 3;
// -------------------

int K;
int customersServedCounter = 0;
int        freeSeatsNo;
bool       isBarberReady = true;
bool       isCustomerReady = false;
bool       isBarberBusy = false;
pthread_t  currentCustomer;
int currentCustomerId;

// mutex
pthread_mutex_t waitRoomMutex;
pthread_mutex_t barberReadyMutex;
pthread_mutex_t customerReadyMutex;

// mutex cond
pthread_cond_t waitRoomCond;
pthread_cond_t barberReadyCond;
pthread_cond_t customerReadyCond;

void exitError(const char* msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void* customerThread(void* arg) {
    pthread_t thread = pthread_self();
    int* idPointer = (int*) arg;
    int  id = *idPointer;

    while (1) {
        pthread_mutex_lock(&waitRoomMutex);
        if (freeSeatsNo > 0 || !isBarberBusy) { 
            if (!isBarberBusy) {
               printf("Customer: waking barber! - %lx (%d)\n", thread, id);
               isBarberBusy = true;
            } else {
                printf("Customer: waiting room! Free places left %d - %lx (%d)\n", freeSeatsNo - 1, thread, id);
            }
            freeSeatsNo -= 1;
            // Notify barber
            pthread_cond_broadcast(&waitRoomCond);
            pthread_mutex_unlock(&waitRoomMutex);
           
            // Wait for barber to fetch customer
            pthread_mutex_lock(&barberReadyMutex);
            while (isBarberReady) {
                pthread_cond_wait(&barberReadyCond, &barberReadyMutex);
            }
            
            isBarberReady = true;
            pthread_cond_broadcast(&barberReadyCond); 
            pthread_mutex_unlock(&barberReadyMutex);

            pthread_mutex_lock(&customerReadyMutex);
            currentCustomer = thread;
            currentCustomerId = id;
            isCustomerReady = true;
            pthread_cond_broadcast(&customerReadyCond);
            pthread_mutex_unlock(&customerReadyMutex);

            // Wait until barber finishes cutting this customers's hair
            pthread_mutex_lock(&customerReadyMutex);
            while (isCustomerReady) {
                pthread_cond_wait(&customerReadyCond, &customerReadyMutex);
            }
            isCustomerReady = true;
            // Leave after 
            printf("Customer: barber finished cutting my hair, leaving.. - %lx (%d)\n", thread, id);
            pthread_cond_broadcast(&customerReadyCond);
            pthread_mutex_unlock(&customerReadyMutex);

            return NULL;
        } else {
            printf("Customer: no free places, waiting room full, leaving.. - %lx (%d)\n", thread, id);
            pthread_mutex_unlock(&waitRoomMutex);
            sleep(1 + rand() % maxDelay);
        }
    }
    
    return NULL;
}

void* barberThread(void* arg) {
    while (1) {
        // sleep until customer comes in
        pthread_mutex_lock(&waitRoomMutex);
        while (freeSeatsNo == K) {
            // If there is no one waiting then go to sleep.
            printf("Barber: I am going to sleep..\n");
            isBarberBusy = false;
            pthread_cond_wait(&waitRoomCond, &waitRoomMutex);
        }

        isBarberBusy = true;
        freeSeatsNo += 1;

        // notify customers that barber is ready
        pthread_mutex_lock(&barberReadyMutex);
        isBarberReady = false;
        pthread_cond_broadcast(&barberReadyCond); 
        pthread_mutex_unlock(&barberReadyMutex);

        // fetch customer from wating room
        pthread_mutex_lock(&customerReadyMutex);
        while (!isCustomerReady) {
            pthread_cond_wait(&customerReadyCond, &customerReadyMutex);
        }

        // Finally cut hair of this poor customer
        printf("Barber: %d customers waiting, cutting hair of %lx (%d)\n", K - freeSeatsNo, currentCustomer, currentCustomerId);
        pthread_mutex_unlock(&waitRoomMutex);
        sleep((rand() % maxHairCutTime) + 1);
        customersServedCounter += 1;

        // Once finished notify customer that he's done ;)
        isCustomerReady = false;
        printf("Barber: finished cutting hair of %lx (%d) waiting for him to leave\n", currentCustomer, currentCustomerId);
        pthread_cond_broadcast(&customerReadyCond); 
        pthread_mutex_unlock(&customerReadyMutex);

        // Wait for customer to leave 
        pthread_mutex_lock(&customerReadyMutex);
        while (!isCustomerReady) {
            pthread_cond_wait(&customerReadyCond, &customerReadyMutex);
        }
        isCustomerReady = false;
        pthread_mutex_unlock(&customerReadyMutex);
    }

    return NULL;
}


int main(int charc, char* argv[]) {
    if (charc < 3)
        exitError("Not enuagh arguments!");

    K = atoi(argv[1]);
    int N = atoi(argv[2]);

    srand(time(NULL));
    pthread_t* threads = malloc(sizeof(pthread_t) * (N + 1));
    printf("-------------------------------------------------------------------------\n");
    printf("SleepingBarberProblem: \n\t- %d chairs in waiting room\n\t- %d customers  \n", K, N);
    printf("-------------------------------------------------------------------------\n");

    // Create barber thread
    int i = 0;
    pthread_create(&threads[i++], NULL, barberThread, NULL);

    // Spawn threads with random delay
    int* waitFor = malloc(sizeof(int) * N);
    int maxWait = 0;
    for(i = 0; i < N; i++) { 
        waitFor[i] = rand() % maxDelay;
        if (maxWait < waitFor[i])
            maxWait = waitFor[i];
    }

    freeSeatsNo = K;

    pthread_mutex_init(&waitRoomMutex, NULL);
    pthread_mutex_init(&barberReadyMutex, NULL);
    pthread_mutex_init(&customerReadyMutex, NULL);

    pthread_cond_init(&barberReadyCond, NULL);
    pthread_cond_init(&waitRoomCond, NULL);
    pthread_cond_init(&customerReadyCond, NULL);

    // Create customers threads
    int execTime = 0;
    while (execTime <= maxWait) {
        for(i = 0; i < N; i++) { 
            if (execTime == waitFor[i]) {
                int* id = malloc(sizeof(int));
                *id = i;
                pthread_create(&threads[i + 1], NULL, customerThread, (void*) id);
            }                
        }
        sleep(1);
        execTime += 1;
    }

    // Wait for barber to serve all customers
    i = N + 1;
    void* threadReturnValue;
    while (i-- > 1) { 
        pthread_join(threads[i], &threadReturnValue);
    }
    printf("-------------------------------------------------------------------------------\n");
    printf("SleepingBarberProblem: all customers have had their hair cut -> problem solved.\n");
    printf("-------------------------------------------------------------------------------\n");

    // Clean up ;)
    pthread_mutex_destroy(&barberReadyMutex);
    pthread_mutex_destroy(&customerReadyMutex);

    pthread_cond_destroy(&barberReadyCond);
    pthread_cond_destroy(&customerReadyCond);

    free(waitFor);
    free(threads);

    return 0;
}


