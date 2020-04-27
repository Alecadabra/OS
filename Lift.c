#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "Lift.h"
/*#include "Buffer.h"*/

void* lift(void* liftInputVoidPtr)
{
    liftInput* input = (liftInput*)liftInputVoidPtr;
    /*buffer* buff     = input->buffer;*/
    int liftNum      = input->liftNum;
    /*int t            = input->t;*/
    free(input);
    free(liftInputVoidPtr);

    printf("I am lift %d of pid %d and tid %ld!\n",
        liftNum, getpid(), pthread_self());

    pthread_exit(0);
}