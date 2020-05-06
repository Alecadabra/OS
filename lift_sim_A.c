#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Buffer.h"
#include "lift_sim_A.h"

#define NUM_LIFTS 3

buffer*         buff;          /* The buffer */
FILE*           sim_out;       /* Shared file ptr to sim_out for logging */
int             t;             /* Time taken for lift to move, given in args */
pthread_mutex_t logMutex;      /* Mutex lock for accessing sim_out file */
pthread_mutex_t buffMutex;     /* Mutex lock for accessing buffer */
pthread_cond_t  buffFullCond;  /* Condition variable for full buffer */
pthread_cond_t  buffEmptyCond; /* Condition variable for empty buffer */

int main(int argc, char* argv[])
{
    pthread_t liftThreads[NUM_LIFTS]; /* Threads for lifts 0, 1 & 2 */
    pthread_t requestThread; /* Thread for request */
    int liftNums[NUM_LIFTS]; /* Numbers 1 to NUM_LIFTS */
    int i; /* For loop index */
    int m; /* Buffer size, given in args */
    int threadError; /* Return value of pthread_create() */

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

    /* Clear open sim_out */
    sim_out = fopen("sim_out", "w");
    if(sim_out == NULL)
    {
        perror("Error, file could not be opened");
        pthread_exit(NULL);
    }
    else if(ferror(sim_out))
    {
        perror("Error in opening file");
        fclose(sim_out);
        sim_out = NULL;
        pthread_exit(NULL);
    }

    /* Thread creation */
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
    fclose(sim_out);
    pthread_mutex_destroy(&logMutex);
    pthread_mutex_destroy(&buffMutex);
    pthread_cond_destroy(&buffEmptyCond);
    pthread_cond_destroy(&buffFullCond);

    pthread_exit(NULL);
}

void* lift(void* liftNumPtr)
{
    /* This lift's number, 1 to NUM_LIFTS */
    int liftNum = *((int*)liftNumPtr);
    /* Current floor, source floor, destination floor */
    int flr = 0, srcFlr, destFlr;
    /* File ptr to sim_out to append logs to */
    int requestNum = 0, move = 0, totMove = 0;

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

        if(!buffer_isFull(buff))
        {
            /* Stop request() from waiting if the buffer is not full */
            pthread_cond_signal(&buffFullCond);
        }

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);

        /* Calculate values for sim_out */
        move = abs(flr - srcFlr) + abs(srcFlr - destFlr);
        totMove += move;
        requestNum++;

        /* Obtain lock for appending to sim_out */
        pthread_mutex_lock(&logMutex);

        /* Write log to sim_out */
        fprintf(sim_out, "Lift-%d Operation\n", liftNum);
        fprintf(sim_out, "Previous position: Floor %d\n", flr);
        fprintf(sim_out, "Request: Floor %d to Floor %d\n", srcFlr, destFlr);
        fprintf(sim_out, "Deatil operations:\n");
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n", flr, srcFlr);
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n", srcFlr, destFlr);
        fprintf(sim_out, "\t#movement for this request: %d\n", move);
        fprintf(sim_out, "\t#request: %d\n", requestNum);
        fprintf(sim_out, "\tTotal #movement: %d\n", totMove);
        fprintf(sim_out, "Current position: Floor %d\n\n", destFlr);

        /* Release lock on sim_out */
        pthread_mutex_unlock(&logMutex);
        
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
    }
    
    printf("Lift %d is done\n", liftNum);

    pthread_exit(0);
}

void* request(void* nullPtr)
{
    FILE* sim_in; /* sim_input file ptr */
    int srcFlr, destFlr; /* Source floor and destination floor read from file */
    int requestNum = 0; /* Number of requests enqueued */

    printf("I am a request of pid %d and tid %ld!\n",
        getpid(), pthread_self());

    /* File opening */
    sim_in = fopen("sim_input", "r");
    if(sim_in == NULL)
    {
        perror("Error, file could not be opened");
        buffer_setComplete(buff);
        pthread_exit(0);
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening file");
        buffer_setComplete(buff);
        fclose(sim_in);
        sim_in = NULL;
        pthread_exit(0);
    }

    while(!feof(sim_in))
    {
        /* Read one line from sim_input */
        fscanf(sim_in, "%d %d", &srcFlr, &destFlr);

        /* Obtain mutex lock on buffer */
        pthread_mutex_lock(&buffMutex);

        if(buffer_isFull(buff))
        {
            /* Wait if the buffer is full */
            pthread_cond_wait(&buffFullCond, &buffMutex);
        }

        /* Enqueue once into the buffer */
        buffer_enqueue(buff, srcFlr, destFlr);
        buffer_print(buff);

        if(!buffer_isEmpty(buff))
        {
            /* Wake up lifts if the buffer is no longer empty */
            pthread_cond_signal(&buffEmptyCond);
        }

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);

        /* Obtain mutex lock for sim_out */
        pthread_mutex_lock(&logMutex);

        fprintf(sim_out, "--------------------------------------------\n");
        fprintf(sim_out, "New Lift Request From Floor %d to Floor %d\n",
            srcFlr, destFlr);
        fprintf(sim_out, "Request No: %d\n", requestNum);
        fprintf(sim_out, "--------------------------------------------\n\n");

        /* Unlock sim_out mutex */
        pthread_mutex_unlock(&logMutex);
    }

    buffer_setComplete(buff);
    fclose(sim_in);

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