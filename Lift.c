#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "Lift.h"

void* lift(void* bufferPtr)
{
    /* int floor; */
        /* Current floor */

    printf("I am a lift of pid %d and tid %ld!\n", getpid(), pthread_self());



    pthread_exit(0);
}