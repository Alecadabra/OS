#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>

#include "Buffer.h"

int main()
{
    buffer* buff = buffer_init_process(2);
    int a, b;

    buffer_enqueue(buff, 1, 2);
    buffer_enqueue(buff, 3, 4);
    buffer_enqueue(buff, 5, 6);
    buffer_enqueue(buff, 7, 8);
    buffer_enqueue(buff, 9, 10);
    buffer_enqueue(buff, 11, 12);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    buffer_enqueue(buff, 13, 14);
    buffer_enqueue(buff, 15, 16);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);

    return 0;
}