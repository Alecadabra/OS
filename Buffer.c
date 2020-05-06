#include <stdlib.h>
#include <stdio.h>

#include "Buffer.h"

/* Create a new buffer */
buffer* buffer_init(int m)
{
    int     i;    /* For loop index                                           */
    buffer* buff; /* The new buffer                                           */
    
    buff           = (buffer*)malloc(sizeof(buffer));
    buff->array    = (int**)  malloc(sizeof(int*) * buff->size);
    buff->size     = m;
    buff->count    = 0;
    buff->head     = 0;
    buff->tail     = 0;
    buff->complete = 0;

    for(i = 0; i < buff->size; i++)
        buff->array[i] = (int*)malloc(2 * sizeof(int));

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
}