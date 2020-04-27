#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lift_sim_A.h"

#define NUM_LIFTS 3

int main(int argc, char* argv[])
{
    pthread_t liftThreads[NUM_LIFTS];
        /* Threads for lifts 0, 1 & 2 */
    liftInput* liftInputs[NUM_LIFTS];
        /* Lift input structs for parameter of lift() */
    pthread_t requestThread;
        /* Thread for request */
    int i;
        /* For loop indexes */
    int m;
        /* Buffer size, given in args */
    int t;
        /* Time taken for lift to move, given in args */
    int threadError;
        /* Return value of pthread_create(), nonzero if in error */
    buffer* buff;
        /* The buffer */

    /* Handle command line arguments */
    if(argc != 3)
    {
        printf("Error: Invalid number of arguments\n");
        pthread_exit(NULL);
    }
    m = atoi(argv[1]);
    t = atoi(argv[2]);

    /* Create buffer */
    buff = buffer_create(m);

    /* Thread creation */
    printf("Main is initialising thread of request\n");
    threadError = pthread_create(
        &requestThread, /* pthread_t ptr to request thread */
        NULL,           /* attr, NULL means use default attributes*/
        request,        /* function ptr to start routine request() */
        (void*)buff     /* argument to give to lift() - ptr to the buffer */
    );

    /* Check for errors */
    if(threadError)
    {
        printf("Error: pthread_create error number %d\n", threadError);
        pthread_exit(NULL);
    }

    for(i = 0; i < NUM_LIFTS; i++)
    {
        printf("Main is initialising thread of lift %d\n", i + 1);

        /* Populate lift input struct */
        liftInputs[i]          = (liftInput*)malloc(sizeof(liftInput));
        liftInputs[i]->buffer  = buff;
        liftInputs[i]->liftNum = i + 1;
        liftInputs[i]->t       = t;

        /* Create thread */
        threadError = pthread_create(
            &liftThreads[i],     /* pthread_t ptr */
            NULL,                /* attr, NULL means use default attributes*/
            lift,                /* function ptr to start routine lift() */
            (void*)liftInputs[i] /* argument to give to lift() - lift input */
        );
        
        /* Check for errors */
        if(threadError)
        {
            printf("Error: pthread_create error number %d\n", threadError);
            pthread_exit(NULL);
        }
    }

    /* buffer_free(buff); */

    pthread_exit(NULL);
}

void* lift(void* liftInputVoidPtr)
{
    liftInput* input = (liftInput*)liftInputVoidPtr;
    /*buffer* buff     = input->buffer;*/
    int liftNum      = input->liftNum;
    /*int t            = input->t;*/
    free(input);

    printf("I am lift %d of pid %d and tid %ld!\n",
        liftNum, getpid(), pthread_self());

    pthread_exit(0);
}

void* request(void* bufferVoidPtr)
{
    /* buffer* buff = (buffer*)bufferVoidPtr; */

    printf("I am a request of pid %d and tid %ld!\n",
        getpid(), pthread_self());

    pthread_exit(0);
}