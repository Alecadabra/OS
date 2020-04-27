#ifndef BUFFER_H

    #define BUFFER_H

    typedef struct {
        int** array; /* 2D malloc Array of ints, size [m][2] */
        int m;       /* Size of 1st dimension of array */
        int start;   /* Index of starting element */
        int end;     /* Index of last element */
        int count;   /* Number of elements */
    } buffer;

    buffer* createBuffer(int);

#endif