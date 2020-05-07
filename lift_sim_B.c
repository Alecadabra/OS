#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <semaphore.h>
#include <errno.h>

#include "Buffer.h"
#include "lift_sim_B.h"

#define LIFTS 3 /* Number of lifts */
#define BUFFER_NAME "/lift_sim_buffer"



int main(int argc, char* argv[])
{
    sem_t buffMutex; /* Mutex lock for accessing buffer */
    sem_t buffFull; /* Condition variable for full buffer */
    sem_t buffEmpty; /* Condition variable for empty buffer */
    sem_t logMutex; /* Mutex lock for accessing sim_out file */


    sem_t buffMutex;
    buffer* buff; /* The buffer */
    FILE* sim_out;  /* Shared file ptr to sim_out for logging */
    int t; /* Time taken to move lift, given in args */
    int globalTotMoves; /* Total number of moves done by lifts */
    int globalTotRequests; /* Total number of requests handled */
    pid_t liftIDs[LIFTS]; /* Process IDs for lifts 1 to LIFTS */
    pid_t requestID; /* Process ID for request */
    int i; /* For loop index */
    int m; /* Buffer size, given in args */
    int forkVal; /* Return value of fork() */
    FILE* sim_in; /* sim_input file to count lines of */
    int lineNo = 0; /* Number of lines counted in sim_input */

    /* Handle command line arguments */
    if(argc != 3)
    {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        return 1;
    }
    m = atoi(argv[1]);
    t = atoi(argv[2]);
    if(m < 1 || t < 0)
    {
        fprintf(stderr, "Error, invalid given values of m = %d and t = %d\n",
            m, t);
        return 1;
    }

    /* Count lines in sim_input */
    sim_in = fopen("sim_input", "r");
    if(sim_in == NULL)
    {
        perror("Error, sim_input file could not be opened");
        return 1;
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening sim_input");
        fclose(sim_in);
        sim_in = NULL;
        return 1;
    }
    while(!feof(sim_in)) if(fgetc(sim_in) == '\n') lineNo++;
    fclose(sim_in);
    if(lineNo < 50 || lineNo > 100)
    {
        fprintf(stderr,
            "Error: number of lines in sim_input must be between 50 and 100\n");
        return 1;
    }

    /* Create buffer */
    buff = buffer_init(m);

    /* Initialise semaphores */
    sem_init(&buffMutex, 1, 1); /* 0 when buffer is being used, > 0 when free */
    sem_init(&buffFull,  1, m); /* 0 when full, > 0 when empty slots */
    sem_init(&buffEmpty, 1, 0); /* 0 when empty, > 0 when full slots */
    sem_init(&logMutex,  1, 1); /* 0 when file is being used, > 0 when free */

    /* Open sim_out logging file and check for errors */
    sim_out = fopen("sim_out", "w");
    if(sim_out == NULL)
    {
        perror("Error, sim_out file could not be opened");
        return 1;
    }
    else if(ferror(sim_out))
    {
        perror("Error in opening sim_out");
        fclose(sim_out);
        sim_out = NULL;
        return 1;
    }

    /* Request process creation and error checking */
    forkVal = fork();
    if(forkVal == -1)
    {
        /* Error */
        fprintf(stderr, "Error: fork error number %d\n", errno);
        return 1;
    }
    if(forkVal == 0)
    {
        /* You are the child */
        request();
        return 0;
    }
    else
    {
        /* You are the parent */
        requestID = forkVal;
    }

    /* Lift process creation and error checking */
    for(i = 0; i < LIFTS; i++)
    {
        forkVal = fork();
        if(forkVal == -1)
        {
            /* Error */
            fprintf(stderr, "Error: fork error number %d\n", errno);
            return 1;
        }
        if(forkVal == 0)
        {
            /* You are the child */
            lift(i + 1);
            return 0;
        }
        else
        {
            /* You are the parent */
            liftIDs[i] = forkVal;
        }
    }

    /* Wait until all processes terminate before cleaning up */
    waitpid(requestID, NULL, NULL);
    for(i = 0; i < LIFTS; i++) waitpid(liftIDs[i], NULL, NULL);

    /* Print final stats to sim_out */
    fprintf(sim_out, "Total number of requests: %d\n", globalTotRequests);
    fprintf(sim_out, "Total number of movements: %d\n" globalTotMoves);
    
    /* Close file and free heap memory */
    fclose(sim_out);
    buffer_destroy(buff);
    sem_destroy(&buffMutex);
    sem_destroy(&buffFull);
    sem_destroy(&buffEmpty);
    sem_destroy(&logMutex);

    return 1;
}

/* Represents Lift-1 to Lift-N, consumer thread that dequeues requests from the
 * buffer                                                                     */
void lift(int liftNum)
{
    int flr = 1; /* Current floor */
    int srcFlr; /* Source floor of current request */
    int destFlr; /* Destination floor of current request */
    int requestNum = 0; /* Number of requests served */
    int move = 0; /* Number of floors moved this request */
    int totMoves = 0; /* Total number of floors moved */
    int done = 0; /* Boolean for the lift being finished */

    while(!done)
    {
        /* Obtain lock for buffer */
        sem_wait(&buffMutex);

        /* Wait if the buffer is empty */
        if(buffer_isEmpty(buff)) pthread_cond_wait(&buffEmpty, &buffMutex);

        /* Dequeue once from buffer */
        buffer_dequeue(buff, &srcFlr, &destFlr);

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);

        /* Stop request() from waiting */
        pthread_cond_signal(&buffFull);

        /* Calculate values for sim_out */
        move = abs(flr - srcFlr) + abs(srcFlr - destFlr);
        totMoves += move;
        requestNum++;

        /* Obtain lock for appending to sim_out */
        pthread_mutex_lock(&logMutex);

        /* Increment global total moves, uses logMutex */
        globalTotMoves += move;

        /* Write log to sim_out */
        fprintf(sim_out, "Lift-%d Operation\n",                liftNum        );
        fprintf(sim_out, "Previous position: Floor %d\n",      flr            );
        fprintf(sim_out, "Request: Floor %d to Floor %d\n",    srcFlr, destFlr);
        fprintf(sim_out, "Deatil operations:\n"                               );
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n",   flr,    srcFlr );
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n",   srcFlr, destFlr);
        fprintf(sim_out, "\t#movement for this request: %d\n", move           );
        fprintf(sim_out, "\t#request: %d\n",                   requestNum     );
        fprintf(sim_out, "\tTotal #movement: %d\n",            totMoves       );
        fprintf(sim_out, "Current position: Floor %d\n\n",     destFlr        );

        /* Release lock on sim_out */
        pthread_mutex_unlock(&logMutex);

        /* Move from current floor to srcFloor */
        sleep(t);
        flr = srcFlr;

        /* Move from current floor to destFloor */
        sleep(t);
        flr = destFlr;

        /* Check if we need to loop again */
        pthread_mutex_lock(&buffMutex);
        if(buffer_isEmpty(buff) && buffer_isComplete(buff)) done = 1;
        pthread_mutex_unlock(&buffMutex);
    }

    return(0);
}

/* Represents Lift-R, producer thread that enqueues requests onto the buffer  */
void* request(void* nullPtr)
{
    FILE* sim_in;  /* sim_input file ptr                                      */
    int   srcFlr;  /* Destination floor read from sim_in                      */
    int   destFlr; /* Source floor read from sim_in                           */

    /* Open sim_input */
    sim_in = fopen("sim_input", "r");
    if(sim_in == NULL)
    {
        perror("Error, sim_input file could not be opened");
        buffer_setComplete(buff);
        return(0);
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening sim_input");
        buffer_setComplete(buff);
        fclose(sim_in);
        sim_in = NULL;
        return(0);
    }

    while(!feof(sim_in))
    {
        /* Read one line from sim_input */
        fscanf(sim_in, "%d %d", &srcFlr, &destFlr);

        /* Make sure read values are legal */
        if(srcFlr < 1 || srcFlr > 20 || destFlr < 1 || destFlr > 20)
        {
            fprintf(stderr, "Illegal floor values of %d %d in sim_input\n",
                srcFlr, destFlr);
            buffer_setComplete(buff);
            fclose(sim_in);
            return(0);
        }

        /* Obtain mutex lock on buffer */
        pthread_mutex_lock(&buffMutex);

        /* Wait if the buffer is full */
        if(buffer_isFull(buff)) pthread_cond_wait(&buffFull, &buffMutex);

        /* Enqueue once into the buffer */
        buffer_enqueue(buff, srcFlr, destFlr);

        /* Unlock buffer mutex */
        pthread_mutex_unlock(&buffMutex);

        /* Wake up lifts */
        pthread_cond_signal(&buffEmpty);

        /* Obtain mutex lock for sim_out */
        pthread_mutex_lock(&logMutex);
        
        /* Increment global total num of requests, uses logMutex */
        globalTotRequests++;

        /* Print to sim_out */
        fprintf(sim_out, "--------------------------------------------\n");
        fprintf(sim_out, "New Lift Request From Floor %d to Floor %d\n",
            srcFlr, destFlr);
        fprintf(sim_out, "Request No: %d\n", globalTotRequests);
        fprintf(sim_out, "--------------------------------------------\n\n");

        /* Unlock sim_out mutex */
        pthread_mutex_unlock(&logMutex);
    }

    /* Set lifts to close once the buffer is empty */
    buffer_setComplete(buff);

    fclose(sim_in);

    return(0);
}