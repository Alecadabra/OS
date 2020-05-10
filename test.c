#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>

#include "Buffer.h"

int main()
{
    buffer* buff = buffer_init_process(5);
    int a, b;

    buffer_enqueue(buff, 1, 2);
    buffer_enqueue(buff, 3, 4);
    buffer_enqueue(buff, 5, 6);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);
    buffer_dequeue(buff, &a, &b);
    printf("%d, %d\n", a, b);

    return 0;
}