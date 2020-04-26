#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "Lift.h"
#include "Request.h"

#define NUM_LIFTS 3

int main(int argc, char* argv[])
{
    pthread_t liftThreads[NUM_LIFTS];
        /* Threads for lifts 0, 1 & 2 */
    liftInput* liftInputs[NUM_LIFTS];
        /* Lift input structs for parameter of lift() */
    pthread_t requestThread;
        /* Thread for request */
    long i, j;
        /* For loop indexes */
    int m;
        /* Buffer size, given in args */
    /* int t; */
        /* Time taken for lift to move, given in args */
    int thread_error;
        /* Return value of pthread_create(), nonzero if in error */
    int** buffer;
        /* The buffer */

    /* Handle command line arguments */
    if(argc != 3)
    {
        printf("Error: Invalid number of arguments\n");
        pthread_exit(NULL);
    }
    m = atoi(argv[1]);
    /* t = atoi(argv[2]); */

    /* Allocate memory for buffer */
    buffer = (int**)malloc(m * sizeof(int*));
    for(i = 0; i < m; i++) buffer[i] = (int*)malloc(2 * sizeof(int));

    /* Thread creation */
    printf("Main is initialising thread of request\n");
    thread_error = pthread_create(
        &requestThread, /* pthread_t ptr to request thread */
        NULL,           /* attr, NULL means use default attributes*/
        request,        /* function ptr to start routine request() */
        &buffer         /* argument to give to lift() - ptr to buffer */
    );

    /* Check for errors */
    if(thread_error)
    {
        printf("Error: pthread_create error number %d\n", thread_error);
        pthread_exit(NULL);
    }

    for(j = 0; j < NUM_LIFTS; j++)
    {
        printf("Main is initialising thread of lift %ld\n", j);

        /* Populate lift input struct */
        liftInputs[i]            = (liftInput*)malloc(sizeof(liftInput));
        liftInputs[i]->bufferPtr = &buffer;
        liftInputs[i]->liftNum   = i + 1;
        liftInputs[i]->m         = m;
        liftInputs[i]->t         = t;

        /* Create thread */
        thread_error = pthread_create(
            &liftThreads[j],     /* pthread_t ptr */
            NULL,                /* attr, NULL means use default attributes*/
            lift,                /* function ptr to start routine lift() */
            (void*)liftInputs[i] /* argument to give to lift() - lift input */
        );
        
        /* Check for errors */
        if(thread_error)
        {
            printf("Error: pthread_create error number %d\n", thread_error);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}