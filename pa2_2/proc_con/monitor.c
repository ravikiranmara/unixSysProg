#ifndef __OSP_2B_MONITOR_C__
#define __OSP_2B_MONITOR_C__

/* monitor code */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

/* my constants */
const int BufferLength = 1;
const int monitor_mutexLocked = 0;
const int monitor_mutexUnlocked = 1;
const int monitor_semNotShared = 0;

int buffer_index = -1;
int monitor_getNumConsumersBlocked();
int monitor_getNumProducersBlocked();

/* condition struct definition */
struct cv
{
    bool blocked;
    int blockedThreadCount;
};

/* encompasses conditional variable, and the
   mutex associated */
struct cond
{
    struct cv cv;
    sem_t mutex;
};

/* my shared buffer */
char *buffer = NULL;

/* shared mutex variables */
struct cond cond_empty; /* consumer */
struct cond cond_full;  /* producer */

/* monitor_mutex : regulate access to monitor. shared between producer and consumer.
    only one can acquire it, so only one thread will be in monitor
    at a givent time */
sem_t monitor_mutex;

/* regulate access to conditional variable. this is used in producer
   and consumer respectively  */
sem_t producer_mutex;
sem_t consumer_mutex;


/***** monitor functions ******/

/* initialize monitor
   Initial condition, buffer is empty. set consumer to block and
   producer to continue */
void monitor_init()
{
    cout<<"MONITOR_INIT:enter" << endl;

    /* allocate buffer */
    buffer = new char[BufferLength];

    /* initialize monitor mutex to be unlocked*/
    sem_init(&monitor_mutex, monitor_semNotShared, monitor_mutexUnlocked);

    /* producer and consumer should have access to condition variable */
    sem_init(&producer_mutex, monitor_semNotShared, monitor_mutexUnlocked);
    sem_init(&consumer_mutex, monitor_semNotShared, monitor_mutexUnlocked);

    /* initial condition for consumer */
    cond_empty.cv.blocked = true;
    cond_full.cv.blocked = false;

    /* mutex for condition values set to locked */
    sem_init(&(cond_empty.mutex), monitor_semNotShared, monitor_mutexLocked);
    sem_init(&(cond_full.mutex), monitor_semNotShared, monitor_mutexLocked);

    /* initialize blocked thread count */
    cond_empty.cv.blockedThreadCount = 0;
    cond_full.cv.blockedThreadCount = 0;

    cout << "MONITOR_INIT:exit" << endl;

    return ;
}


/* condition variable functions */
int monitor_getNumProducersBlocked()
{
    int num = 0;

    /* take lock as we might be out of monitor */
    sem_wait(&producer_mutex);
    num = cond_full.cv.blockedThreadCount;
    sem_post(&producer_mutex);

    //cout << "NumProducers : " << num << endl;
    return num;
}



/* condition variable functions */
int monitor_getNumConsumersBlocked()
{
    int num = 0;

    /* take lock as we might be out of monitor */
    sem_wait(&consumer_mutex);
    num = cond_empty.cv.blockedThreadCount;
    sem_post(&consumer_mutex);

    //cout << "NumConsumers : " << num << endl;
    return num;;
}



/* insert a character into the shared buffer */
void monitor_insert(char ch)
{
    pthread_t self = pthread_self();

    cout <<"monitor_insert: enter (" << self << "):Enter" << endl;
    while(true)
    {
        /* acquire mutex for monitor. only one thread can be in the monitor */
        sem_wait(&monitor_mutex);

        /* check if the condition is satisfied */
        sem_wait(&producer_mutex);
        if(true == cond_full.cv.blocked)
        {
            /* if not block yourself */
            cond_full.cv.blockedThreadCount++;

            /* release locks */
            sem_post(&producer_mutex);
            sem_post(&monitor_mutex);

            /* block yourself  */
            cout << "monitor_insert(" << self << ") : producer release monitor lock and block:" << cond_full.cv.blockedThreadCount << endl;
            sem_wait(&cond_full.mutex);

            /* if somebody released you from the lock
            they have already set the condition variable false
            join the queue for monitor*/
        }
        else
        {
            break;
        }
    }

    /* release the mutex gaurding cond_full */
    sem_post(&producer_mutex);

    /* if you are here, then you the the condition cv is favourable to thread,
        and we have the montior mutex, so we are the only thead in the monitor */

    cout <<"monitor_insert: (" << self << "):Insert - " << buffer_index+1 << endl;
    /*** our actual insert ***/
    buffer[buffer_index] = ch;
    buffer_index++;

    /* we inserted into the queue, thus it is no longer empty. set condition variable false */
    sem_wait(&consumer_mutex);
    //cout << "monitor_insert(" << self << ") : signal new item" << endl;
    cond_empty.cv.blocked = false;
    sem_post(&consumer_mutex);


    /* if somebody is blocked in the thread because of us, signal them */
    if(0 < monitor_getNumConsumersBlocked())
    {
        sem_wait(&consumer_mutex);
        sem_post(&(cond_empty.mutex));
        cond_empty.cv.blockedThreadCount--;
        cout << "monitor_insert(" << self << ") : free someone" << cond_empty.cv.blockedThreadCount << endl;
        sem_post(&consumer_mutex);
    }

    /* check if the queue is full, and block future producers */
    if(buffer_index == BufferLength-1)
    {
        /* no need to take the lock as we are already gauranteed that we are the only
        thread in the monitor. We take the lock anyways, to guard against future
        scenario where we might allow multiple threads to run simultaneously*/
        sem_wait(&producer_mutex);
        cout << "monitor_insert(" << self << ") : signal full" << endl;
        cond_full.cv.blocked = true;
        sem_post(&producer_mutex);
    }

    /* release mutex */
    sem_post(&monitor_mutex);

    cout <<"monitor_insert: enter (" << self << "):Exit" << endl;
    return;
}


/* remove a character from shared buffer */
char monitor_remove()
{
    char ch = 'a';
    pthread_t self = pthread_self();

    cout <<"monitor_remove: enter (" << self << "):Enter" << endl;
    while(true)
    {
        /* acquire mutex for monitor. only one thread can be in the monitor */
        sem_wait(&monitor_mutex);

        /* check if the condition is satisfied */
        sem_wait(&consumer_mutex);
        if(true == cond_empty.cv.blocked)
        {
            /* if not block yourself */
            cond_empty.cv.blockedThreadCount++;

            /* release locks */
            sem_post(&consumer_mutex);
            sem_post(&monitor_mutex);

            /* block yourself  */
            cout << "monitor_remove(" << self << ") : consumer release monitor lock and block:" << cond_empty.cv.blockedThreadCount << endl;
            sem_wait(&cond_empty.mutex);

            /* if somebody released you from the lock
            they are already updating condition variable
            and numOfBlockedThreads */

        }
        else
        {
            break;
        }
    }

    /* release the mutex gaurding cond_full */
    sem_post(&consumer_mutex);

    /* if you are here, then you the the condition cv is favourable to thread,
        and we have the montior mutex, so we are the only thead in the monitor */

    cout <<"monitor_remove: (" << self << "):remove - " << buffer_index << endl;
    /*** our actual remove ***/
    ch = buffer[buffer_index];
    buffer_index--;

    /* if somebody is blocked in the thread because of us, signal them */
    if(0 < monitor_getNumProducersBlocked())
    {
        sem_wait(&consumer_mutex);
        sem_post(&(cond_full.mutex));
        cout << "monitor_remove(" << self << ") : free someone : " << cond_empty.cv.blockedThreadCount << endl;
        sem_post(&consumer_mutex);
    }

    /* we just inerted, the queue is no longer full */
    sem_wait(&producer_mutex);
    cout << "monitor_remove(" << self << ") : signal removed item" << endl;
    cond_full.cv.blocked = false;
    sem_post(&producer_mutex);


    /* check if the queue is empty */
    if(buffer_index == -1)
    {
        /* no need to take the lock as we are already gauranteed that we are the only
        thread in the monitor. We take the lock anyways, to guard against future
        scenario where we might allow multiple threads to run simultaneously*/
        sem_wait(&consumer_mutex);
        cout << "monitor_remove(" << self << ") : signal empty" << endl;
        cond_empty.cv.blocked = true;
        sem_post(&consumer_mutex);
    }

    /* release mutex */
    sem_post(&monitor_mutex);

    cout <<"monitor_remove: enter (" << self << "):Exit" << endl;
    return ch;
}


#endif
