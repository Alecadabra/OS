#ifndef BUFFER_H

    #define BUFFER_H

    typedef struct {
        int** array; /* 2D malloc Array of ints, size [size][2] */
        int size;    /* Size of 1st dimension of array */
        int start;   /* Index of starting element */
        int end;     /* Index of last element */
        int count;   /* Number of populated elements */
    } buffer;

    buffer* createBuffer(int);

#endif