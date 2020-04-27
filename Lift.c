#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "Lift.h"

void* lift(void* liftInputVoidPtr)
{
    liftInput* input = (liftInput*)liftInputVoidPtr;
    /* int*** bufferPtr = input->bufferPtr; */
    int liftNum      = input->liftNum;
    /* int m            = input->m; */
    /* int t            = input->t; */

    printf("I am lift %d of pid %d and tid %ld!\n",
        liftNum, getpid(), pthread_self());

    pthread_exit(0);
}