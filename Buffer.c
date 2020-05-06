#include <stdlib.h>
#include <stdio.h>

#include "Buffer.h"

buffer* buffer_init(int m)
{
    buffer* buff = (buffer*)malloc(sizeof(buffer));
    int i;

    buff->size     = m;
    buff->count    = 0;
    buff->start    = 0;
    buff->end      = 0;
    buff->complete = 0;
    buff->array    = (int**)malloc(buff->size * sizeof(int*));
    for(i = 0; i < buff->size; i++)
        buff->array[i] = (int*)malloc(2 * sizeof(int));

    return buff;
}

int buffer_isEmpty(buffer* buff)
{
    return buff->count == 0;
}

int buffer_isFull(buffer* buff)
{
    return buff->count == buff->size;
}

int buffer_isComplete(buffer* buff)
{
    return buff->complete;
}

int buffer_enqueue(buffer* buff, int srcNum, int destNum)
{
    /* First make sure buffer is not full */
    if(buffer_isFull(buff)) return 0;

    buff->array[buff->end][0] = srcNum;
    buff->array[buff->end][1] = destNum;
    buff->count++;

    if(buff->end == (buff->size - 1))
    {
        /* Queue needs to loop around */
        buff->end = 0;
    }
    else
    {
        /* End index can can increment normally */
        buff->end++;
    }
    
    return 1;
}

int buffer_dequeue(buffer* buff, int* srcNum, int* destNum)
{
    /* First make sure buffer is not empty */
    if(buffer_isEmpty(buff)) return 0;

    *srcNum = buff->array[buff->start][0];
    *destNum = buff->array[buff->start][1];
    buff->count--;

    if(buff->start == (buff->size - 1))
    {
        /* Queue needs to loop backwards */
        buff->start = 0;
    }
    else
    {
        /* Start index can increment normally */
        buff->start++;
    }
    
    return 1;
}

void buffer_setComplete(buffer* buff)
{
    buff->complete = 1;
}

void buffer_destroy(buffer* buff)
{
    int i;

    for(i = 0; i < buff->size; i++) free(buff->array[i]);
    free(buff->array);
    free(buff);
}

void buffer_print(buffer* buff)
{
    int i;

    printf("[");
    for(i = 0; i < buff->size; i++)
    {
        printf("(%d, %d) ", buff->array[i][0], buff->array[i][1]);
    }
    printf("]\n");
}