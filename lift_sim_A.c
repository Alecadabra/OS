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
pthread_mutex_t logMutex;
    /* Mutex lock for accessing sim_out file */
pthread_mutex_t buffMutex;
    /* Mutex lock for accessing buffer */
pthread_cond_t buffFullCond;
    /* Condition variable for full buffer */
pthread_cond_t buffEmptyCond;
    /* Condition variable for empty buffer */

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
    FILE* sim_out;
        /* File ptr to sim_out to clear the file before making threads */

    /* Handle command line arguments */
    if(argc != 3)
    {
        printf("Error: Invalid number of arguments\n");
        pthread_exit(NULL);
    }
    m = atoi(argv[1]);
    t = atoi(argv[2]);

    /* Create buffer */
    buff = buffer_init(m);

    /* Create mutex and condition variables */
    pthread_mutex_init(&logMutex, NULL);
    pthread_mutex_init(&buffMutex, NULL);
    pthread_cond_init(&buffFullCond, NULL);
    pthread_cond_init(&buffEmptyCond, NULL);

    /* Clear out sim_out */
    sim_out = openFile("sim_out", "w");
    if(sim_out == NULL) pthread_exit(NULL);
    fprintf(sim_out, "");
    fclose(sim_out);

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
    
    buffer_destroy(buff);
    pthread_mutex_destroy(&logMutex);
    pthread_mutex_destroy(&buffMutex);
    pthread_cond_destroy(&buffEmptyCond);
    pthread_cond_destroy(&buffFullCond);

    pthread_exit(NULL);
}

void* lift(void* liftNumPtr)
{
    int liftNum = *((int*)liftNumPtr);
    int flr = 0, srcFlr, destFlr;
        /* Current floor, source floor, destination floor */
    FILE* sim_out;
        /* File ptr to sim_out to append logs to */

    printf("I am lift %d of pid %d and tid %ld!\n",
        liftNum, getpid(), pthread_self());
    
    sim_out = openFile("sim_out", "a");
    if(sim_out == NULL) pthread_exit(1);

    while(!checkIfFinished())
    {
        /* Obtain mutex lock on buffer */
        pthread_mutex_lock(&buffMutex);

        if(buffer_isComplete(buff) && buffer_isEmpty(buff))
        {
            pthread_mutex_unlock(&buffMutex);
            break;
        }

        if(buffer_isEmpty(buff))
        {
            /* Wait if the buffer is empty */
            pthread_cond_wait(&buffEmptyCond, &buffMutex);
        }

        /* Dequeue once from buffer */
        buffer_dequeue(buff, &srcFlr, &destFlr);
        printf("Lift %d is dequeueing %d %d\n", liftNum, srcFlr, destFlr);

        if(!buffer_isFull(buff))
        {
            /* Stop request() from waiting if the buffer is not full */
            pthread_cond_signal(&buffFullCond);
        }

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);
        
        printf("Lift %d moving from floor %d to floor %d\n",
            liftNum, flr, srcFlr);
        sleep(t);

        /* Change current floor to srcFloor */
        flr = srcFlr;

        printf("Lift %d moving from floor %d to floor %d\n",
            liftNum, flr, destFlr);
        sleep(t);

        /* Change current floor to destFloor */
        flr = destFlr;

        /* Obtain lock for reading to sim_out */
        pthread_mutex_lock(&logMutex);
    }
    
    printf("Lift %d is done\n", liftNum);

    pthread_exit(0);
}

void* request(void* nullPtr)
{
    FILE* file;
        /* sim_out file ptr */
    int srcFlr, destFlr;
        /* Source floor and destination floor read from file */

    printf("I am a request of pid %d and tid %ld!\n",
        getpid(), pthread_self());

    /* File opening */
    file = openFile("sim_input", "r");
    if(file == NULL)
    {
        buffer_setComplete(buff);
        pthread_exit(0);
    }

    while(!feof(file))
    {
        /* Read one line from sim_input */
        fscanf(file, "%d %d", &srcFlr, &destFlr);

        /* Obtain mutex lock on buffer */
        pthread_mutex_lock(&buffMutex);

        if(buffer_isFull(buff))
        {
            /* Wait if the buffer is full */
            pthread_cond_wait(&buffFullCond, &buffMutex);
        }

        /* Enqueue once into the buffer */
        printf("Requester is enqueueing %d %d onto the buffer\n",
            srcFlr, destFlr );
        buffer_enqueue(buff, srcFlr, destFlr);
        buffer_print(buff);

        if(!buffer_isEmpty(buff))
        {
            /* Wake up lifts if the buffer is no longer empty */
            pthread_cond_signal(&buffEmptyCond);
        }

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);
    }

    printf("Requester is done\n");
    buffer_setComplete(buff);
    fclose(file);

    pthread_exit(0);
}

int checkIfFinished()
{
    int finished = 0;

    pthread_mutex_lock(&buffMutex);
    if(buffer_isComplete(buff) && buffer_isEmpty(buff))
    {
        finished = 1;
    }
    pthread_mutex_unlock(&buffMutex);

    return finished;
}

FILE* openFile(char fileName[], char mode[])
{
    FILE* file = fopen(fileName, mode);

    if(file == NULL)
    {
        perror("Error, file could not be opened");
    }
    else if(ferror(file))
    {
        perror("Error in opening file");
        fclose(file);
        file = NULL;
    }

    return file;
}