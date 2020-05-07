#ifndef LIFT_SIM_B_H

    #define LIFT_SIM_B_H

    typedef struct semaphores
    {
        sem_t buffMutex; /* Mutex lock for accessing buffer */
        sem_t buffFull; /* Condition variable for full buffer */
        sem_t buffEmpty; /* Condition variable for empty buffer */
        sem_t logMutex; /* Mutex lock for accessing sim_out file */
    };
    

    /* Function foward declarations */
    void* lift    (void*);
    void* request (void*);

#endif