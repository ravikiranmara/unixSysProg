
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <cstring>
#include <exception>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::bad_alloc;
using std::exception;
using std::strerror;

/* global functions */
const int status_success = 0;
const int CreateThreadException = -1;

const int BufferLength = 10000000;
const int NumProducer = 10;
const int NumConsumer = 10;
const int NumberOfWrites = 10000000;

const int SemNotShared = 0;
const int MutexLocked = 0;
const int MutexUnlocked = 1;

char *buffer = NULL;    /* buffer shared by producer/consumer */
int buffer_index = 0;         /* index for buffer */

sem_t *empty = NULL;    /* signal buffer is empty (in other words keeps track of number of items ) */
sem_t *full = NULL;     /* signal buffer is full (in other words keeps track of number of empty slots) */
sem_t *mutex = NULL;    /* mutex for buffer */


/*
If the semaphore currently has the value zero, then the call blocks until either it
becomes possible to perform the decrement (i.e., the semaphore value rises above zero)

consumer should wait till something is in the buffer. empty = not zero
producer need not wait, it can start iinserting values. full = not bufferlength
mutex gaurds the access to both. And either should be able to enter. mutex = 1

also, modern machines, assuming int to be 32 bits
*/

int init_semaphore()
{
    // the buffer is initially empty
    /* number of items in buffer, thus number of times consmer consume */
    empty = new sem_t;
    sem_init(empty, SemNotShared, buffer_index);

    /* similarly, number of free solts in bugger, thus the number of times producer can produe */
    full = new sem_t;
    sem_init(full, SemNotShared, BufferLength - buffer_index);

    /* mutex either locked or unlocked */
    mutex = new sem_t;
    sem_init(mutex, SemNotShared, MutexUnlocked);
}

/* initialize wait time

/* write/add X to buffer numberOfWrite tiems */
void *producer_function(void *arg)
{
    int *rval = new int;
    *rval = status_success;

    cout << "producer enter" << endl;
    for (long i=0; i<NumberOfWrites; i++)
    {
        /* if buffer is not full */
        sem_wait(full);

        /* if nobody is accessing buffer currently */
        sem_wait(mutex);

        /* if buffer is full, signal full */
        *(buffer + buffer_index)= 'X';
        buffer_index+=1;

        /* if release mutex */
        sem_post(mutex);

        /* signal buffer is not empty (signal or one item is added) */
        sem_post(empty);
    }

    cout << "Producer exit" << endl;
    /* exit from thread */
    //pthread_exit(rval);

    return rval;
}

/* read/remove x from buffer */
void* consumer_function(void *arg)
{
    cout << "Consumer Enter" << endl;

    while(1)
    {
        /* if buffer is not empty */
        sem_wait(empty);

        /* if nobody is accessing buffer currently */
        sem_wait(mutex);

        /* if buffer is empty, signal empty */
        buffer_index -= 1;

        /* if release mutex */
        sem_post(mutex);

        /* signal buffer is not full (signal one item was taken away from buffer)*/
        sem_post(full);
    }

    /* it will be a unclean exit for consumers */
}


int main(int argc, char* argv[])
{
    pthread_t *producers = NULL, *consumers = NULL; /* array for producer and consumer thread handles */
    int rval = status_success;
    void* producer_rval = NULL;

    try
    {
        /* arraylist for producer consumer */
        producers = new pthread_t[NumProducer];
        consumers = new pthread_t[NumConsumer];
        buffer = new char[BufferLength];

        /* init sem locks */
        init_semaphore();

        /* create producer threads  */
        cout << "Creating producer threads" << endl;
        for(int i=0; i<NumProducer; i++)
        {
            rval = pthread_create((producers + i), NULL, producer_function, (void*)NULL);
            if(0 != rval)
            {
                cerr << "Producer thread:" << strerror(rval) << endl;;
                throw bad_alloc();
            }
        }

        /* crate consumer threads */
        cout << "Creating consumer threads" << endl;
        for(int i=0; i<NumConsumer; i++)
        {
            rval = pthread_create((consumers + i), NULL, consumer_function, (void* )NULL);
            if(0 != rval)
            {
                cerr << "Consumer thread:" << strerror(rval) << endl;
                throw bad_alloc();
            }

        }

        /* wait for producers to finish */
        cout << "Wait for producers to finish" << endl;
        for(int i=0; i<NumProducer; i++)
        {
            cout << "Join " << i << endl;
            pthread_join(*(producers+i), NULL);
        }

        /* wait ofr the consumers to finish */
        cout << "Wait for consumers to read all the buffer" << endl;
        bool loop = true;
        while (loop)
        {
            sem_wait(mutex);

            /* if all input is read by consumer */
            if(0 == buffer_index)
            {
                loop = false;
            }

            sem_post(mutex);
        }

        rval = 0;
    }
    catch (bad_alloc& ex)
    {
        cerr << "failed to allocate memory " << ex.what() << endl;
        rval = -1;
    }
    catch(...)
    {
        cerr << "generic exception" << endl;
        rval = -1;
    }

    /* cleanup code */
    cout << "Cleanup code" << endl;
    if(NULL != producers)
    {
        delete []producers;
        producers = NULL;
    }

    if(NULL != producers)
    {
        delete []consumers;
        consumers = NULL;
    }

    cout << "all input is read, End of program" << endl;
    return rval;
}
