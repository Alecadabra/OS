#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

#include "Buffer.h"
#include "lift_sim_B.h"

#define LIFTS 3 /* Number of lifts */
#define SEM_BUFF_NAME "/lift_sem_buff"
#define SEM_FULL_NAME "/lift_sem_full"
#define SEM_EMPTY_NAME "/lift_sem_empty"
#define SEM_LOG_NAME "/lift_sem_log"
#define TOT_MOVES_NAME "/lift_tot_moves"
#define TOT_REQUESTS_NAME "/lift_tot_requests"

int main(int argc, char* argv[])
{
    pid_t liftIDs[LIFTS]; /* Process IDs for lifts 1 to LIFTS */
    pid_t requestID; /* Process ID for request */
    int i; /* For loop index */
    int m; /* Buffer size, given in args */
    int t; /* Time taken to move lift, given in args */
    pid_t forkVal; /* Return value of fork() */
    FILE* sim_in; /* sim_input file to count lines of */
    FILE* sim_out; /* sim_out file ptr to write final stats to */
    int lineNo = 0; /* Number of lines counted in sim_input */
    int fd; /* Shared memory file descriptor used for various variables */
    buffer* buff; /* The buffer */
    int* sharedTotMoves;
    int* sharedTotRequests;
    sem_t* buffMutex; /* Mutex lock for accessing buffer */
    sem_t* buffFull; /* Condition variable for full buffer */
    sem_t* buffEmpty; /* Condition variable for empty buffer */
    sem_t* logMutex; /* Mutex lock for accessing sim_out file */

    /* Handle command line arguments */
    if(argc != 3)
    {
        perror("Error: Invalid number of arguments");
        return 1;
    }
    m = atoi(argv[1]);
    t = atoi(argv[2]);
    if(m < 1 || t < 0)
    {
        perror("Error, invalid command line arguments");
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
        perror("Error, sim_input must be between 50 and 100 lines");
        return 1;
    }

    if(errno != 0) perror("Main error 1");

    /* Clear out sim_out since we are appending to it */
    remove("sim_out");
    if(errno != 0) perror("Main error 2");

    /* Initialise semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME, O_CREAT, 0644, 1); /* 0 when buffer is being used, > 0 when free */
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    buffFull = (sem_t*)sem_open(SEM_FULL_NAME, O_CREAT, 0644, m); /* 0 when full, > 0 when empty slots */
    if(buffFull == SEM_FAILED) perror("BuffFull failed");
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, O_CREAT, 0644, 0); /* 0 when empty, > 0 when full slots */
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    logMutex = (sem_t*)sem_open(SEM_LOG_NAME, O_CREAT, 0644, 1); /* 0 when file is being used, > 0 when free */
    if(logMutex == SEM_FAILED) perror("LogMutex failed");
    /* Alternative method
    fd = shm_open(SEM_BUFF_NAME, O_CREAT, O_RDWR);
    ftruncate(fd, sizeof(sem_t));
    buffMutex = (sem_t*)mmap(0, sizeof(sem_t), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);
    sem_init(buffMutex, 1, 1); */
    if(errno != 0) perror("Main error 3");

    /* Initialise buffer */ 
    buff = buffer_init_process(m);
    if(errno != 0) perror("Main error 4");

    /* Set up shared memory for total moves an requests */
    fd = shm_open(TOT_MOVES_NAME, O_CREAT | O_RDWR, 0666);
    if(fd == -1) perror("Open failed on main moves counter");
    ftruncate(fd, sizeof(int));
    sharedTotMoves = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);
    if(sharedTotMoves == MAP_FAILED) perror(
        "Map failed on main move counter");
    *sharedTotMoves = 0;
    fd = shm_open(TOT_REQUESTS_NAME, O_CREAT | O_RDWR, 0666);
    if(fd == -1) perror("Open failed on main request counter");
    ftruncate(fd, sizeof(int));
    sharedTotRequests = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);
    if(sharedTotRequests == MAP_FAILED) perror(
        "Map failed on main request counter");
    *sharedTotRequests = 0;
    if(errno != 0) perror("Main error 5");

    /* Request process creation and error checking */
    forkVal = fork();
    if(forkVal == -1)
    {
        /* Error */
        perror("Error, fork failed");
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
    if(errno != 0) perror("Main error 6");

    /* Lift process creation and error checking */
    for(i = 0; i < LIFTS; i++)
    {
        if(errno != 0) perror("Main error 6.1");
        forkVal = fork();
        if(forkVal == -1)
        {
            /* Error */
            if(errno != 0) perror("Main error 6.2");
            perror("Error, fork failed");
            return 1;
        }
        if(forkVal == 0)
        {
            if(errno != 0) perror("Main error 6.3");
            /* You are the child */
            lift(i + 1, t);
            return 0;
        }
        else
        {
            if(errno != 0) perror("Main error 6.4");
            /* You are the parent */
            liftIDs[i] = forkVal;
        }
        if(errno != 0) perror("Main error 6.5");
    }
    if(errno != 0) perror("Main error 7");

    /* Wait until all processes terminate before cleaning up */
    waitpid(requestID, NULL, 0);
    for(i = 0; i < LIFTS; i++) waitpid(liftIDs[i], NULL, 0);
    if(errno != 0) perror("Main error 8");

    /* Print final stats to sim_out */
    sim_out = fopen("sim_out", "a");
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
    fprintf(sim_out, "Total number of requests: %d\n", *sharedTotRequests);
    fprintf(sim_out, "Total number of movements: %d\n", *sharedTotMoves);
    fclose(sim_out);
    if(errno != 0) perror("Main error 9");
    
    /* Clean up */
    sem_destroy(buffMutex);
    sem_destroy(buffFull);
    sem_destroy(buffEmpty);
    sem_destroy(logMutex);
    shm_unlink(SEM_BUFF_NAME);
    shm_unlink(SEM_FULL_NAME);
    shm_unlink(SEM_EMPTY_NAME);
    shm_unlink(SEM_LOG_NAME);
    shm_unlink(TOT_MOVES_NAME);
    shm_unlink(TOT_REQUESTS_NAME);
    shm_unlink(BUFF_NAME);
    buffer_destroy_process(buff);
    if(errno != 0) perror("Main error 10");

    return 0;
}

/* Represents Lift-1 to Lift-N, consumer thread that dequeues requests from the
 * buffer                                                                     */
void lift(int liftNum, int t)
{
    int flr = 1; /* Current floor */
    int srcFlr = 0; /* Source floor of current request */
    int destFlr = 0; /* Destination floor of current request */
    int requestNum = 0; /* Number of requests served */
    int move = 0; /* Number of floors moved this request */
    int totMoves = 0; /* Total number of floors moved */
    int done = 0; /* Boolean for the lift being finished */
    int fd; /* File descriptor used to open buffer and tot moves */
    buffer* buff;
    int* sharedTotMoves;
    sem_t* buffMutex; /* Mutex lock for accessing buffer */
    sem_t* buffFull; /* Condition variable for full buffer */
    sem_t* buffEmpty; /* Condition variable for empty buffer */
    sem_t* logMutex; /* Mutex lock for accessing sim_out file */
    FILE* sim_out;

    printf("Check it out! I'm a lift\n");
    if(errno != 0) perror("Lift error");

    /* Set up buffer */
    fd = shm_open(BUFF_NAME, O_RDWR, 0666);
    if(fd == -1) perror("Open failed on lift buffer");
    buff = (buffer*)mmap(0, sizeof(buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    /* Set up shared total moves counter */
    fd = shm_open(TOT_MOVES_NAME, O_RDWR, 0666);
    if(fd == -1) perror("Open failed on lift moves counter");
    sharedTotMoves = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    /* Open semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME, 0);
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    buffFull = (sem_t*)sem_open(SEM_FULL_NAME, 0);
    if(buffFull == SEM_FAILED) perror("BuffFull failed");
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, 0);
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    logMutex = (sem_t*)sem_open(SEM_LOG_NAME, 0);
    if(logMutex == SEM_FAILED) perror("LogMutex failed");

    while(!done)
    {
        /* Obtain lock for buffer */
        sem_wait(buffMutex);

        /* Wait if the buffer is empty */
        sem_wait(buffEmpty);

        /* Dequeue once from buffer */
        buffer_dequeue(buff, &srcFlr, &destFlr);

        /* Unlock buffer mutex */
        sem_post(buffMutex);

        /* Stop request() from waiting */
        sem_post(buffFull);

        /* Calculate values for sim_out */
        move = abs(flr - srcFlr) + abs(srcFlr - destFlr);
        totMoves += move;
        requestNum++;

        /* Obtain lock for appending to sim_out */
        sem_wait(logMutex);

        /* Increment shared total moves, uses logMutex */
        (*sharedTotMoves) += move;

        /* Write log to sim_out */
        sim_out = fopen("sim_out", "a");
        if(sim_out == NULL)
        {
            perror("Error, sim_out file could not be opened");
            return;
        }
        else if(ferror(sim_out))
        {
            perror("Error in opening sim_out");
            fclose(sim_out);
            sim_out = NULL;
            return;
        }
        fprintf(sim_out, "Lift-%d Operation\n",                liftNum        );
        fprintf(sim_out, "Previous position: Floor %d\n",      flr            );
        fprintf(sim_out, "Request: Floor %d to Floor %d\n",    srcFlr, destFlr);
        fprintf(sim_out, "Detail operations:\n"                               );
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n",   flr,    srcFlr );
        fprintf(sim_out, "\tGo from Floor %d to Floor %d\n",   srcFlr, destFlr);
        fprintf(sim_out, "\t#movement for this request: %d\n", move           );
        fprintf(sim_out, "\t#request: %d\n",                   requestNum     );
        fprintf(sim_out, "\tTotal #movement: %d\n",            totMoves       );
        fprintf(sim_out, "Current position: Floor %d\n\n",     destFlr        );
        fclose(sim_out);

        /* Release lock on sim_out */
        sem_post(logMutex);

        /* Move from current floor to srcFloor */
        sleep(t);
        flr = srcFlr;

        /* Move from current floor to destFloor */
        sleep(t);
        flr = destFlr;

        /* Check if we need to loop again */
        sem_wait(buffMutex);
        if(buffer_isEmpty(buff) && buffer_isComplete(buff)) done = 1;
        sem_post(buffMutex);
    }

    /* Clean up */
    shm_unlink(BUFF_NAME);
    shm_unlink(TOT_MOVES_NAME);

    if(errno != 0) perror("Lift error");
}

/* Represents Lift-R, producer thread that enqueues requests onto the buffer  */
void request()
{
    FILE* sim_in;  /* sim_input file ptr                                      */
    int   srcFlr;  /* Destination floor read from sim_in                      */
    int   destFlr; /* Source floor read from sim_in                           */
    buffer* buff;
    int* sharedTotRequests;
    sem_t* buffMutex; /* Mutex lock for accessing buffer */
    sem_t* buffFull; /* Condition variable for full buffer */
    sem_t* buffEmpty; /* Condition variable for empty buffer */
    sem_t* logMutex; /* Mutex lock for accessing sim_out file */
    FILE* sim_out;
    int fd;
    
    printf("Check it out! I'm a request!\n");
    if(errno != 0) perror("Request error");

    /* Set up buffer */
    fd = shm_open(BUFF_NAME, O_RDWR, 0666);
    if(fd == -1) perror("Open failed on request buffer");
    buff = (buffer*)mmap(0, sizeof(buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    /* Set up shared total request counter */
    fd = shm_open(TOT_REQUESTS_NAME, O_RDWR, 0666);
    if(fd == -1) perror("Open failed on request counter");
    sharedTotRequests = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    /* Open semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME, 0);
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    buffFull = (sem_t*)sem_open(SEM_FULL_NAME, 0);
    if(buffFull == SEM_FAILED) perror("BuffFull failed");
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, 0);
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    logMutex = (sem_t*)sem_open(SEM_LOG_NAME, 0);
    if(logMutex == SEM_FAILED) perror("LogMutex failed");

    /* Open sim_input */
    sim_in = fopen("sim_input", "r");
    if(sim_in == NULL)
    {
        perror("Error, sim_input file could not be opened");
        buffer_setComplete(buff);
        return;
    }
    else if(ferror(sim_in))
    {
        perror("Error in opening sim_input");
        buffer_setComplete(buff);
        fclose(sim_in);
        sim_in = NULL;
        return;
    }

    while(!feof(sim_in))
    {
        /* Read one line from sim_input */
        fscanf(sim_in, "%d %d", &srcFlr, &destFlr);

        /* Make sure read values are legal */
        if(srcFlr < 1 || srcFlr > 20 || destFlr < 1 || destFlr > 20)
        {
            perror("Illegal floor values of %d %d in sim_input\n",
                srcFlr, destFlr);
            buffer_setComplete(buff);
            fclose(sim_in);
            return;
        }

        /* Obtain mutex lock on buffer */
        sem_wait(buffMutex);

        /* Wait if the buffer is full */
        sem_wait(buffFull);

        /* Enqueue once into the buffer */
        buffer_enqueue(buff, srcFlr, destFlr);

        /* Unlock buffer mutex */
        sem_post(buffMutex);

        /* Wake up lifts */
        sem_post(buffEmpty);

        /* Obtain mutex lock for sim_out */
        sem_wait(logMutex);
        
        /* Increment shared total num of requests, uses logMutex */
        (*sharedTotRequests)++;

        /* Print to sim_out */
        sim_out = fopen("sim_out", "a");
        if(sim_out == NULL)
        {
            perror("Error, sim_out file could not be opened");
            return;
        }
        else if(ferror(sim_out))
        {
            perror("Error in opening sim_out");
            fclose(sim_out);
            sim_out = NULL;
            return;
        }
        fprintf(sim_out, "--------------------------------------------\n");
        fprintf(sim_out, "New Lift Request From Floor %d to Floor %d\n",
            srcFlr, destFlr);
        fprintf(sim_out, "Request No: %d\n", *sharedTotRequests);
        fprintf(sim_out, "--------------------------------------------\n\n");
        fclose(sim_out);

        /* Unlock sim_out mutex */
        sem_post(logMutex);
    }

    /* Set lifts to close once the buffer is empty */
    buffer_setComplete(buff);

    /* Clean up */
    fclose(sim_in);
    shm_unlink(BUFF_NAME);
    shm_unlink(TOT_REQUESTS_NAME);

    if(errno != 0) perror("Request error");
}