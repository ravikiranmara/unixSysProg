#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <exception>
#include <cstring>

#include "monitor.c"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::bad_alloc;
using std::strerror;

/* global functions */
const int status_success = 0;
const int CreateThreadException = -1;

const int NumProducer = 2;
const int NumConsumer = 2;
const int NumberOfWrites = 100;


/* get random character */
char getRandomCharacter()
{
    srand(time(NULL));
    int r = rand() % 26;
    return r + 'a';
}

/* producer insert into buffer number of write number of times */
void *producer_function(void * arg)
{
    int *rval = NULL;
    char ch;

    cout << "Producer : Allocate memory for retrun value" << endl;;
    if(NULL == (rval = (int*) malloc(sizeof(int))))
    {
        perror("Unable to allocate variable for return value:");
        exit(errno);
    }

    *rval = status_success;

    cout<<"Producer:foreach producer input to buffer" << endl;
    for(int i=0; i<NumberOfWrites; i++)
    {
        ch = getRandomCharacter();
        monitor_insert(ch);
    }

    cout << "Producer:End of producer" << endl;
    return (void*)rval;
}


/* consumer removes an item from the buffer */
void *consumer_function(void *arg)
{
    int *rval = NULL;
    char ch;

    cout << "Consumer : allocate memory for return value" << endl;
    if(NULL == (rval = (int*)malloc(sizeof(int))))
    {
        perror("Unable to allocate memory to return value:");
        exit(errno);
    }

    int i=0;
    while(true)
    {
        ch = monitor_remove();
    }

    /* we will let the end of process kill the consumer threads.
       The other approaches are need some work, this is the simplest
       way to get this done */

    return (void*)rval;
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

        /* init sem locks */
        monitor_init();

        /* create producer threads  */
        cout << "Main:Creating producer threads" << endl;
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
        cout << "Main:Creating consumer threads" << endl;
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
        cout << "Main:Wait for producers to finish" << endl;
        for(int i=0; i<NumProducer; i++)
        {
            cout << "Join " << i << endl;
            pthread_join(*(producers+i), NULL);
        }

        /* wait ofr the consumers to finish */
        cout << "Main:Wait for consumers to read all the buffer" << endl;
        bool loop = true;
        int num;
        while (loop)
        {
            num = monitor_getNumConsumersBlocked();
            if(NumConsumer == num)
                loop = false;

            cout << "Main:num consumers blocked : " << num << endl;
            sleep(5);
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
    cout << "Main:Cleanup code" << endl;
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

    cout << "Main:all input is read, End of program" << endl;
    return rval;
}
