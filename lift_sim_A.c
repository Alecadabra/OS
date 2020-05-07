#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Buffer.h"
#include "lift_sim_A.h"

#define LIFTS 3 /* Number of lifts */

buffer*         buff;              /* The buffer                              */
pthread_mutex_t buffMutex;         /* Mutex lock for accessing buffer         */
pthread_cond_t  buffFull;          /* Condition variable for full buffer      */
pthread_cond_t  buffEmpty;         /* Condition variable for empty buffer     */
FILE*           sim_out;           /* Shared file ptr to sim_out for logging  */
pthread_mutex_t logMutex;          /* Mutex lock for accessing sim_out file   */
int             t;                 /* Time taken to move lift, given in args  */
int             globalTotMoves;    /* Total number of moves done by lifts     */
int             globalTotRequests; /* Total number of requests handled        */

int main(int argc, char* argv[])
{
    pthread_t liftThreads[LIFTS]; /* Threads for lifts 1 to LIFTS             */
    pthread_t requestThread;      /* Thread for request                       */
    int       liftNums[LIFTS];    /* Numbers 1 to LIFTS for lift() parameter  */
    int       i;                  /* For loop index                           */
    int       m;                  /* Buffer size, given in args               */
    int       threadError;        /* Return value of pthread_create()         */
    FILE*     sim_in;             /* sim_input file to count lines of         */
    int       lineNo = 0;         /* Number of lines counted in sim_input     */

    /* Handle command line arguments */
    if(argc != 3)
    {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        pthread_exit(NULL);
    }
    m = atoi(argv[1]);
    t = atoi(argv[2]);
    if(m < 1 || t < 0)
    {
        fprintf(stderr, "Error, invalid given values of m = %d and t = %d\n",
            m, t);
        pthread_exit(NULL);
    }

    /* Count lines in sim_input */
    sim_in = fopen("sim_input", "r");
    if(sim_in == NULL)
    {
        perror("Error, sim_input file could not be opened");
        pthread_exit(NULL);
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening sim_input");
        fclose(sim_in);
        sim_in = NULL;
        pthread_exit(NULL);
    }
    while(!feof(sim_in)) if(fgetc(sim_in) == '\n') lineNo++;
    fclose(sim_in);
    if(lineNo < 50 || lineNo > 100)
    {
        fprintf(stderr,
            "Error: number of lines in sim_input must be between 50 and 100\n");
        pthread_exit(NULL);
    }

    /* Create buffer */
    buff = buffer_init(m);

    /* Create mutex and condition variables */
    pthread_mutex_init(&logMutex,  NULL);
    pthread_mutex_init(&buffMutex, NULL);
    pthread_cond_init (&buffFull,  NULL);
    pthread_cond_init (&buffEmpty, NULL);

    /* Open sim_out logging file and check for errors */
    sim_out = fopen("sim_out", "w");
    if(sim_out == NULL)
    {
        perror("Error, sim_out file could not be opened");
        pthread_exit(NULL);
    }
    else if(ferror(sim_out))
    {
        perror("Error in opening sim_out");
        fclose(sim_out);
        sim_out = NULL;
        pthread_exit(NULL);
    }

    /* Request thread creation and error checking */
    threadError = pthread_create(
        &requestThread, /* pthread_t ptr to request thread                    */
        NULL,           /* attr, NULL means use default attributes            */
        request,        /* function ptr to start routine request()            */
        NULL            /* argument to give to lift(), NULL for none          */
    );
    if(threadError)
    {
        fprintf(stderr, "Error: pthread_create error number %d\n",
            threadError);
        pthread_exit(NULL);
    }

    /* Lift thread creation and error checking */
    for(i = 0; i < LIFTS; i++)
    {
        liftNums[i] = i + 1;
            /* If we just passed &i to the lift, i might increment before the
            lift set's its lift number to *i, so we must allocate a separate
            int for each lift */

        threadError = pthread_create(
            &liftThreads[i],    /* pthread_t ptr to lift thread               */
            NULL,               /* attr, NULL means use default attributes    */
            lift,               /* function ptr to start routine lift()       */
            (void*)&liftNums[i] /* argument to give to lift() - lift number   */
        );
        if(threadError)
        {
            fprintf(stderr, "Error: pthread_create error number %d\n",
                threadError);
            pthread_exit(NULL);
        }
    }

    /* Join all threads to wait until they terminate before cleaning up */
    pthread_join(requestThread, NULL);
    for(i = 0; i < LIFTS; i++) pthread_join(liftThreads[i], NULL);

    /* Print final stats to sim_out */
    fprintf(sim_out, "Total number of requests: %d\n", globalTotRequests);
    fprintf(sim_out, "Total number of movements: %d\n" globalTotMoves);
    
    /* Close file and free heap memory */
    fclose               (sim_out);
    buffer_destroy       (buff);
    pthread_mutex_destroy(&logMutex);
    pthread_mutex_destroy(&buffMutex);
    pthread_cond_destroy (&buffEmpty);
    pthread_cond_destroy (&buffFull);

    pthread_exit(NULL);
}

/* Represents Lift-1 to Lift-N, consumer thread that dequeues requests from the
 * buffer                                                                     */
void* lift(void* liftNumPtr)
{
    int liftNum;          /* This lift's number, 1 to LIFTS                   */
    int flr          = 1; /* Current floor                                    */
    int srcFlr;           /* Source floor of current request                  */
    int destFlr;          /* Destination floor of current request             */
    int requestNum   = 0; /* Number of requests served                        */
    int move         = 0; /* Number of floors moved this request              */
    int totMoves     = 0; /* Total number of floors moved                     */
    int done         = 0; /* Boolean for the lift being finished              */

    /* Assign lift number from void* parameter */
    liftNum = *((int*)liftNumPtr);

    while(!done)
    {
        /* Obtain lock for buffer */
        pthread_mutex_lock(&buffMutex);

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

        /* Increment global total moves */
        globalTotMoves += move;

        /* Obtain lock for appending to sim_out */
        pthread_mutex_lock(&logMutex);

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

    pthread_exit(0);
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
        pthread_exit(0);
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening sim_input");
        buffer_setComplete(buff);
        fclose(sim_in);
        sim_in = NULL;
        pthread_exit(0);
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
            pthread_exit(0);
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

        /* Increment global total num of requests */
        globalTotRequests++;

        /* Obtain mutex lock for sim_out */
        pthread_mutex_lock(&logMutex);

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

    pthread_mutex_lock(&buffMutex);
    buffer_setComplete(buff);
    pthread_mutex_unlock(&buffMutex);

    fclose(sim_in);

    pthread_exit(0);
}