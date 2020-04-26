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
    pthread_t requestThread;
        /* Thread for request */
    long i, j;
        /* For loop indexes */
    /* int m; */
        /* Buffer size, given in args */
    /* int t; */
        /* Time taken for lift to move, given in args */
    int thread_error;
        /* Return value of pthread_create(), nonzero if in error */
    int** buffer;
        /* The buffer */
    void* bufferPtr = 
    

    /* Handle command line arguments */
    if(argc != 3)
    {
        printf("Error: Invalid number of arguments\n");
        pthread_exit(NULL);
    }
    m = atoi(argv[1]);
    /* t = atoi(argv[2]); */

    /* Allocate memory for buffer */
    buffer = malloc(m * sizeof(int*));
    for(i = 0; i < m; i++) buffer[i] = malloc(2 * sizeof(int));


    /* Thread creation */
    thread_error = pthread_create(
        &requestThread,
        NULL,
        request,
        (void *)buffer);

    )
    for(j = 0; j < NUM_LIFTS; j++)
    {
        printf("Main is initialising thread of lift %ld\n", j);
        thread_error = pthread_create(
            &liftsThread[j], /* pthread_t ptr */
            NULL,        /* attr, NULL means use default attributes*/
            lift,        /* function ptr to start routine lift() in Lift.c */
            (void *)buffer);  /* argument to give to lift() */
        
        if(thread_error)
        {
            /* Executes if pthreads_create returns an error */
            printf("Error: pthread_create error number %d\n", thread_error);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}