#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "Request.h"

void request(void* threadIdPtr)
{
    printf("I am a request of pid %d\n", getpid());

    pthread_exit(0);
}