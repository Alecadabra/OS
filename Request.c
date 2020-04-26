#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "Request.h"

void* request(void* bufferPtr)
{
    printf("I am a request of pid %d and tid %ld!\n", getpid(), pthread_self());

    pthread_exit(0);
}