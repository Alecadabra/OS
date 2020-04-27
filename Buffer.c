#include <stdlib.h>

#include "Buffer.h"

buffer* buffer_create(int inM)
{
    buffer* buff = (buffer*)malloc(sizeof(buffer));
    int i;

    buff->m = inM;
    buff->i = 0;

    buff->array = (int**)malloc(buff->m * sizeof(int*));

    for(i = 0; i < buff->m; i++)
    {
        buff->array[i] = (int*)malloc(2 * sizeof(int));
    }

    return buff;
}

int buffer_isEmpty(buffer* buff)
{
    return buff->count == 0;
}

int buffer_isFull(buffer* buff)
{
    return buff->count == buff->m;
}

int buffer_enqueue(buffer* buff, int srcNum, int destNum)
{
    if(buffer_isFull(buff)) return 0;

    buff->array[buff->end][0] = srcNum;
    buff->array[buff->end][1] = destNum;
    buff->count++;

    if(buff->end == (buff->m - 1))
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
    if(buffer_isEmpty(buff)) return 0;

    *srcNum = buff->array[buff->start][0];
    *destNum = buff->array[buff->start][1];
    buff->count--;

    if(buff->start == (buff->m - 1))
    {
        /* Queue needs to loop backwards */
        buff->start = 0;
    }
    else
    {
        /* Queue can decrement 
    }
    
}