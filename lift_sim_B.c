/* Simulates 3 consumer lifts serving floor requests created by a producer using
 * a bounded buffer. Implemented using processes with POSIX shared memory */

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

#define LIFTS             3                    /* Number of lifts             */
#define SEM_BUFF_NAME     "/lift_sem_buff"     /* Shared name of buffMutex    */
#define SEM_FULL_NAME     "/lift_sem_full"     /* Shared name of buffFull     */
#define SEM_EMPTY_NAME    "/lift_sem_empty"    /* Shared name of buffEmpty    */
#define SEM_LOG_NAME      "/lift_sem_log"      /* Shared name of logMutex     */
#define TOT_MOVES_NAME    "/lift_tot_moves"    /* Sharedname of totMov        */
#define TOT_REQUESTS_NAME "/lift_tot_requests" /* Shared name of totRequests  */

int main(int argc, char* argv[])
{
    pid_t   liftIDs[LIFTS]; /* Process IDs for lifts 1 to LIFTS               */
    pid_t   requestID;      /* Process ID for request                         */
    int     i;              /* For loop index                                 */
    int     m;              /* Buffer size, given in args                     */
    int     t;              /* Time taken to move lift, given in args         */
    pid_t   forkVal;        /* Return value of fork()                         */
    FILE*   sim_in;         /* sim_input file to count lines of               */
    FILE*   sim_out;        /* sim_out file ptr to write final stats to       */
    int     lineNo     = 0; /* Number of lines counted in sim_input           */
    buffer* buff;           /* The buffer shared memory obj ptr               */
    int*    fd_buffArr;     /* File descriptors for buffer shared memory      */
    int*    totMov;         /* Total moves done by lifts shared obj ptr       */
    int     fd_totMov;      /* File descriptor for totMov                     */
    int*    totReq;         /* Total requests served shared obj ptr           */
    int     fd_totReq;      /* File descriptor for totReq                     */
    sem_t*  buffMutex;      /* Binary semaphore for accessing buffer          */
    sem_t*  buffFull;       /* Counting semaphore for full, init to m         */
    sem_t*  buffEmpty;      /* Counting semaphore for empty, init to m        */
    sem_t*  logMutex;       /* Binary semaphore for accessing sim_out         */

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

    /* Clear out sim_out since we are appending to it */
    remove("sim_out");

    /* Initialise semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME,  O_CREAT | O_EXCL, 0666, 1);
    buffFull =  (sem_t*)sem_open(SEM_FULL_NAME,  O_CREAT | O_EXCL, 0666, m);
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, O_CREAT | O_EXCL, 0666, 0);
    logMutex =  (sem_t*)sem_open(SEM_LOG_NAME,   O_CREAT | O_EXCL, 0666, 1);
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    if(buffFull  == SEM_FAILED) perror("BuffFull failed" );
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    if(logMutex  == SEM_FAILED) perror("LogMutex failed" );

    /* Initialise buffer */ 
    fd_buffArr = (int*)malloc((m + 2) * sizeof(int)); /* FDs of entire buffer */
    buff       = buffer_init_process(m, fd_buffArr);  /* Stores FDs in array  */

    /* Set up shared memory for total moves an requests */
    fd_totMov = shm_open(TOT_MOVES_NAME,    O_CREAT | O_RDWR, 0666);
    fd_totReq = shm_open(TOT_REQUESTS_NAME, O_CREAT | O_RDWR, 0666);
    if(fd_totMov == -1) perror("Open failed on main moves counter"  );
    if(fd_totReq == -1) perror("Open failed on main request counter");
    ftruncate(fd_totMov, sizeof(int));
    ftruncate(fd_totReq, sizeof(int));
    totMov = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_totMov, 0);
    totReq = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_totReq, 0);
    if(totMov == MAP_FAILED) perror("Map failed on main move counter"   );
    if(totReq == MAP_FAILED) perror("Map failed on main request counter");
    *totMov = 0;
    *totReq = 0;

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

    /* Lift process creation and error checking */
    for(i = 0; i < LIFTS; i++)
    {
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
            lift(i + 1, t);
            return 0;
        }
        else
        {
            /* You are the parent */
            liftIDs[i] = forkVal;
        }
    }

    /* Wait until all processes terminate before cleaning up */
    waitpid(requestID, NULL, 0);
    for(i = 0; i < LIFTS; i++) waitpid(liftIDs[i], NULL, 0);

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
    fprintf(sim_out, "Total number of requests: %d\n",  *totReq  );
    fprintf(sim_out, "Total number of movements: %d\n", *totMov);
    fclose (sim_out);
    
    /* Clean up */
    if(munmap(totMov,    sizeof(int))   == -1) perror("Munmap error totMov"   );
    if(munmap(totReq,    sizeof(int))   == -1) perror("Munmap error totReq"   );
    if(munmap(buffMutex, sizeof(sem_t)) == -1) perror("Munmap error buffMutex");
    if(munmap(buffFull,  sizeof(sem_t)) == -1) perror("Munmap error buffFull" );
    if(munmap(buffEmpty, sizeof(sem_t)) == -1) perror("Munmap error buffEmpty");
    if(munmap(logMutex,  sizeof(sem_t)) == -1) perror("Munmap error logMutex" );

    if(close    (fd_totMov) == -1) perror("Close error totMov"    );
    if(close    (fd_totReq) == -1) perror("Close error totReq"    );
    if(sem_close(buffMutex) == -1) perror("Close error bufffMutex");
    if(sem_close(buffFull)  == -1) perror("Close error buffFull"  );
    if(sem_close(buffEmpty) == -1) perror("Close error buffEmpty" );
    if(sem_close(logMutex)  == -1) perror("Close error logMutex"  );

    if(sem_unlink(SEM_BUFF_NAME)     == -1) perror("Unlink error sem buff"    );
    if(sem_unlink(SEM_FULL_NAME)     == -1) perror("Unlink error sem full"    );
    if(sem_unlink(SEM_EMPTY_NAME)    == -1) perror("Unlink error sem empty"   );
    if(sem_unlink(SEM_LOG_NAME)      == -1) perror("Unlink error sem log"     );
    if(shm_unlink(TOT_MOVES_NAME)    == -1) perror("Unlink error tot moves"   );
    if(shm_unlink(TOT_REQUESTS_NAME) == -1) perror("Unlink error tot requests");

    buffer_destroy_process(buff, fd_buffArr);
    free                  (fd_buffArr);

    return 0;
}

/* Represents Lift-1 to Lift-N, consumer process that dequeues requests from the
 * buffer                                                                     */
void lift(int liftNum, int t)
{
    int     flr         = 1; /* Current floor                                 */
    int     srcFlr;          /* Source floor of current request               */
    int     destFlr;         /* Destination floor of current request          */
    int     requestNum  = 0; /* Number of requests served                     */
    int     move        = 0; /* Number of floors moved this request           */
    int     localTotMov = 0; /* Total number of floors moved                  */
    int     done        = 0; /* Boolean for the lift being finished           */
    int     fd_buff;         /* File descriptor for buffer shared obj         */
    buffer* buff;            /* Buffer shared obj                             */
    int     fd_totMov;       /* File descriptor for totMov shared obj         */
    int*    totMov;          /* totMov shared obj                             */
    sem_t*  buffMutex;       /* Binary semaphore for accessing buffer         */
    sem_t*  buffFull;        /* Counting semaphore for full, init to m        */
    sem_t*  buffEmpty;       /* Counting semaphore for empty, init to m       */
    sem_t*  logMutex;        /* Binary semaphore for accessing sim_out        */
    FILE*   sim_out;         /* File ptr to log outout file to append to      */

    /* Set up shared buffer and total moves counter */
    fd_buff   = shm_open(BUFF_NAME,      O_RDWR, 0666);
    fd_totMov = shm_open(TOT_MOVES_NAME, O_RDWR, 0666);
    if(fd_buff   == -1) perror("Open failed on lift buffer"       );
    if(fd_totMov == -1) perror("Open failed on lift moves counter");
    buff        = (buffer*)mmap(0, sizeof(buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_buff,   0);
    localTotMov = (int*)   mmap(0, sizeof(int),    PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_totMov, 0);
    if(buff        == MAP_FAILED) perror("Map failed on lift buffer"       );
    if(localTotMov == MAP_FAILED) perror("Map failed on lift moves counter");

    /* Open semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME,  0);
    buffFull  = (sem_t*)sem_open(SEM_FULL_NAME,  0);
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, 0);
    logMutex  = (sem_t*)sem_open(SEM_LOG_NAME,   0);
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    if(buffFull  == SEM_FAILED) perror("BuffFull failed" );
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    if(logMutex  == SEM_FAILED) perror("LogMutex failed" );

    while(!done)
    {
        /* Wait if the buffer is empty */
        sem_wait(buffEmpty);

        /* Obtain lock for buffer */
        sem_wait(buffMutex);

        /* Dequeue once from buffer */
        if(!buffer_dequeue(buff, &srcFlr, &destFlr))
        {
            /* Dequeue failed */
            sem_post(buffMutex);
            return;
        }

        /* Unlock buffer mutex */
        sem_post(buffMutex);

        /* Stop request() from waiting */
        sem_post(buffFull);

        /* Calculate values for sim_out */
        move = abs(flr - srcFlr) + abs(srcFlr - destFlr);
        localTotMov += move;
        requestNum++;

        /* Obtain lock for appending to sim_out */
        sem_wait(logMutex);

        /* Increment shared total moves, uses logMutex */
        (*localTotMov) += move;

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
        fprintf(sim_out, "\tTotal #movement: %d\n",            localTotMov    );
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
    munmap(buff,        sizeof(buffer));
    munmap(localTotMov, sizeof(int)   );

    sem_close(buffMutex);
    sem_close(buffFull );
    sem_close(buffEmpty);
    sem_close(logMutex );
    close    (fd_totMov);
    close    (fd_buff  );
}

/* Represents Lift-R, producer process that enqueues requests onto the buffer */
void request()
{
    FILE*   sim_in;   /* sim_input file ptr                                   */
    int     srcFlr;   /* Destination floor read from sim_in                   */
    int     destFlr;  /* Source floor read from sim_in                        */
    int     fd_buff;  /* File descriptor for buffer shared obj                */
    buffer* buff;      /* Buffer shared obj                                   */
    int     fd_totReq; /* File descriptor for totMov shared obj               */
    int*    totReq;    /* totReq shared obj                                   */
    sem_t*  buffMutex; /* Binary semaphore for accessing buffer               */
    sem_t*  buffFull;  /* Counting semaphore for full buffer, init to m       */
    sem_t*  buffEmpty; /* Counting semaphore for empty buffer, init to m      */
    sem_t*  logMutex;  /* Binary semaphore for accessing sim_out              */
    FILE*   sim_out;   /* File ptr to log outout file to append to            */
    int     i;         /* For loop index                                      */

    /* Set up shared buffer and  total request counter */
    fd_buff   = shm_open(BUFF_NAME,         O_RDWR, 0666);
    fd_totReq = shm_open(TOT_REQUESTS_NAME, O_RDWR, 0666);
    if(fd_buff   == -1) perror("Open failed on request buffer" );
    if(fd_totReq == -1) perror("Open failed on request counter");
    buff   = (buffer*)mmap(0, sizeof(buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_buff,   0);
    totReq = (int*)   mmap(0, sizeof(int),    PROT_READ | PROT_WRITE,
        MAP_SHARED, fd_totReq, 0);
    if(buff   == MAP_FAILED) perror("Map failed on requester buffer"         );
    if(totReq == MAP_FAILED) perror("Map failed on requester's total counter");

    /* Open semaphores */
    buffMutex = (sem_t*)sem_open(SEM_BUFF_NAME,  0);
    buffFull  = (sem_t*)sem_open(SEM_FULL_NAME,  0);
    buffEmpty = (sem_t*)sem_open(SEM_EMPTY_NAME, 0);
    logMutex  = (sem_t*)sem_open(SEM_LOG_NAME,   0);
    if(buffMutex == SEM_FAILED) perror("BuffMutex failed");
    if(buffFull  == SEM_FAILED) perror("BuffFull failed" );
    if(buffEmpty == SEM_FAILED) perror("BuffEmpty failed");
    if(logMutex  == SEM_FAILED) perror("LogMutex failed" );

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
            perror("Illegal floor values in sim_input");
            buffer_setComplete(buff);
            fclose(sim_in);
            return;
        }

        /* Wait if the buffer is full */
        sem_wait(buffFull);

        /* Obtain mutex lock on buffer */
        sem_wait(buffMutex);

        /* Enqueue once into the buffer */
        if(!buffer_enqueue(buff, srcFlr, destFlr))
        {
            /* Enqueue failed */
            sem_post(buffMutex);
            return;
        }

        /* Unlock buffer mutex */
        sem_post(buffMutex);

        /* Wake up a lift */
        sem_post(buffEmpty);

        /* Obtain mutex lock for sim_out */
        sem_wait(logMutex);
        
        /* Increment shared total num of requests, uses logMutex */
        (*totReq)++;

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
        fprintf(sim_out, "Request No: %d\n", *totReq);
        fprintf(sim_out, "--------------------------------------------\n\n");
        fclose(sim_out);

        /* Unlock sim_out mutex */
        sem_post(logMutex);
    }

    /* Set lifts to close once the buffer is empty */
    buffer_setComplete(buff);
    for(i = 0; i < LIFTS; i++) sem_post(buffEmpty);

    /* Clean up */
    fclose(sim_in);

    munmap(buff,   sizeof(buff));
    munmap(totReq, sizeof(int));

    sem_close(buffMutex);
    sem_close(buffFull );
    sem_close(buffEmpty);
    sem_close(logMutex );
    close    (fd_buff  );
    close    (fd_totReq);
}