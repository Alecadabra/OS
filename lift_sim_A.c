#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Buffer.h"
#include "lift_sim_A.h"

#define NUM_LIFTS 3

buffer* buff;
    /* The buffer */
int t;
    /* Time taken for lift to move, given in args */
pthread_mutex_t buffMutex;
    /* Mutex variable for buff */

int main(int argc, char* argv[])
{
    pthread_t liftThreads[NUM_LIFTS];
        /* Threads for lifts 0, 1 & 2 */
    pthread_t requestThread;
        /* Thread for request */
    int liftNums[NUM_LIFTS];
        /* Numbers 1 to NUM_LIFTS */
    int i;
        /* For loop index */
    int m;
        /* Buffer size, given in args */
    int threadError;
        /* Return value of pthread_create(), nonzero if in error */

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

    pthread_mutex_init(&buffMutex, NULL);

    /* Thread creation */
    printf("Main is initialising thread of request\n");
    threadError = pthread_create(
        &requestThread, /* pthread_t ptr to request thread */
        NULL,           /* attr, NULL means use default attributes*/
        request,        /* function ptr to start routine request() */
        NULL            /* argument to give to lift() - ptr to the buffer */
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

        liftNums[i] = i + 1;
            /* If we just passed &i to the lift, i might increment before the
            lift set's its lift number to *i, so we must allocate a separate
            int for each lift */

        /* Create thread */
        threadError = pthread_create(
            &liftThreads[i],    /* pthread_t ptr */
            NULL,               /* attr, NULL means use default attributes*/
            lift,               /* function ptr to start routine lift() */
            (void*)&liftNums[i] /* argument to give to lift() - lift number */
        );
        
        /* Check for errors */
        if(threadError)
        {
            printf("Error: pthread_create error number %d\n", threadError);
            pthread_exit(NULL);
        }
    }

    pthread_join(requestThread, NULL);
    for(i = 0; i < NUM_LIFTS; i++) pthread_join(liftThreads[i], NULL);
    
    buffer_free(buff);
    pthread_mutex_destroy(&buffMutex);

    pthread_exit(NULL);
}

void* lift(void* liftNumPtr)
{
    int liftNum = *((int*)liftNumPtr);

    printf("I am lift %d of pid %d and tid %ld!\n",
        liftNum, getpid(), pthread_self());

    while(!buffer_isComplete(buff))
    {
        /* Wait until buffer not empty */
        /* Wait until buffer lock is obtained */
        /* Dequeue one entry from buffer */
        /* Unlock buffer */
        /* Wait t seconds */
        /* Change current floor to srcFloor */
        /* Wait t seconds */
        /* Change current floor to destFloor */        
    }
    
    printf("Lift %d is done\n", liftNum);

    pthread_exit(0);
}

void* request(void* nullPtr)
{
    FILE* file;

    printf("I am a request of pid %d and tid %ld!\n",
        getpid(), pthread_self());

    /* File opening */
    file = fopen("sim_input", "r");

    if(file == NULL)
    {
        perror("Error, file could not be opened");
        buffer_setComplete(buff);
        pthread_exit(0);
    }
    else if(ferror(file))
    {
        perror("Error in opening file");
        fclose(file);
        buffer_setComplete(buff);
        pthread_exit(0);
    }

    while(!feof(file))
    {
        /* Wait until buffer is not full */
        /* Wait until sim_input lock is obtained */
        /* Read one line from sim_input */
        /* Unlock sim_input */
        /* Wait until buffer lock is obtained */
        /* Enqueue once into the buffer */
        /* Unlock buffer */
        break;
    }

    printf("Requester is done\n");
    buffer_setComplete(buff);
    fclose(file);

    pthread_exit(0);
}