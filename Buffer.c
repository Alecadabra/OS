/* Control and store FIFO array buffer initialised with either heap or shared
 * memory allocation */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "Buffer.h"

/* Create a new buffer using malloc allocation */
buffer* buffer_init(int m)
{
    int     i;    /* For loop index                                           */
    buffer* buff; /* The new buffer                                           */
    
    buff           = (buffer*)malloc(sizeof(buffer));
    buff->array    = (int**)  malloc(sizeof(int*) * m);
    buff->size     = m;
    buff->count    = 0;
    buff->head     = 0;
    buff->tail     = 0;
    buff->complete = 0;

    for(i = 0; i < buff->size; i++)
        buff->array[i] = (int*)malloc(2 * sizeof(int));

    return buff;
}

/* Create a new buffer using shared memory allocation */
buffer* buffer_init_process(int m, int* fd)
{
    int i;
    buffer* buff;
    char entry_str[32];

    /* Allocate the buffer */
    fd[0] = shm_open(BUFF_NAME, O_CREAT | O_RDWR, 0666);
    if(fd[0] == -1) perror("Open failed on buffer");
    if(ftruncate(fd[0], sizeof(buffer)) == -1) perror("Buff truncate");
    buff = (buffer*)mmap(0, sizeof(buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd[0], 0);
    if(buff == MAP_FAILED) perror("Map failed on buffer");
    
    /* Allocate the array */
    fd[1] = shm_open(ARRAY_NAME, O_CREAT | O_RDWR, 0666);
    if(fd[1] == -1) perror("Open failed on buffer array");
    ftruncate(fd[1], sizeof(int*) * m);
    buff->array = (int**)mmap(0, sizeof(int*) * m, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd[1], 0);
    if(buff->array == MAP_FAILED) perror("Map failed on buffer array");
    for(i = 0; i < m; i++)
    {
        sprintf(entry_str, "%s_entry_%d", ARRAY_NAME, i);
        fd[i+2] = shm_open(entry_str, O_CREAT | O_RDWR, 0666);
        if(fd[i+2] == -1) perror("Open failed on buffer array i");
        ftruncate(fd[i+2], sizeof(int) * 2);
        buff->array[i] = (int*)mmap(0, sizeof(int) * 2, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd[i+2], 0);
        if(buff->array[i] == MAP_FAILED) perror("Map failed on buffer array i");
    }

    /* Set other variables */
    buff->size     = m;
    buff->count    = 0;
    buff->head     = 0;
    buff->tail     = 0;
    buff->complete = 0;

    return buff;
}

/* Check if a buffer is empty */
int buffer_isEmpty(buffer* buff)
{
    return buff->count == 0;
}

/* Check if a buffer is full */
int buffer_isFull(buffer* buff)
{
    return buff->count == buff->size;
}

/* Check if a buffer has been marked as complete */
int buffer_isComplete(buffer* buff)
{
    return buff->complete;
}

/* Enqueue a src and dest num onto the end of the buffer */
int buffer_enqueue(buffer* buff, int srcNum, int destNum)
{
    /* First make sure buffer is not full */
    if(buffer_isFull(buff)) return 0;

    /* Push values */
    buff->array[buff->tail][0] = srcNum;
    buff->array[buff->tail][1] = destNum;

    buff->count++;

    /* Either increment tail index or loop around */
    if(buff->tail == (buff->size - 1)) buff->tail = 0; /* Tail needs to loop  */
    else                               buff->tail++;   /* Tail can increment  */
    
    return 1;
}

/* Dequeue a src and dest num from the front of the buffer */
int buffer_dequeue(buffer* buff, int* srcNum, int* destNum)
{
    /* First make sure buffer is not empty */
    if(buffer_isEmpty(buff)) return 0;

    /* Pull values */
    *srcNum  = buff->array[buff->head][0];
    *destNum = buff->array[buff->head][1];

    buff->count--;

    /* Either increment head index or loop around */
    if(buff->head == (buff->size - 1)) buff->head = 0; /* Head needs to loop  */
    else                               buff->head++;   /* Head can increment  */

    return 1;
}

/* Set a buffer as complete */
void buffer_setComplete(buffer* buff)
{
    buff->complete = 1;
}

/* Free all asscociated memory of a buffer */
void buffer_destroy(buffer* buff)
{
    int i;

    for(i = 0; i < buff->size; i++) free(buff->array[i]);
    free(buff->array);
    free(buff);
    buff = NULL;
}

/* Clean up all asscociated memory of a buffer */
void buffer_destroy_process(buffer* buff, int* fd)
{
    int i;
    char entry_str[32]; /* Name of each 2nd dimension array in buff->array */

    for(i = 0; i < buff->size; i++)
    {
        sprintf(entry_str, "%s_entry_%d", ARRAY_NAME, i);

        if(munmap(buff->array[i], sizeof(int*)) == -1)
            perror("Munmap error on buffer array element");

        if(close(fd[i+2]) == -1) perror("Close error on buffer array element");

        if(shm_unlink(entry_str) == -1) perror("Unlink error on array elem");
    }
    if(munmap(buff->array, sizeof(int**)) == -1)
        perror("Munmap error on buffer array");
    if(close(fd[1]) == -1)
        perror("Close error on buffer array");
    if(shm_unlink(ARRAY_NAME) == -1)
        perror("Unlink error on buffer array");

    if(munmap(buff, sizeof(buffer)) == -1)
        perror("Munmap error on buffer");
    if(close(fd[0]) == -1)
        perror("Close error on buffer");
    if(shm_unlink(BUFF_NAME) == -1)
        perror("Unlink error on buffer");
}