#ifndef BUFFER_H

    #define BUFFER_H

    /* Buffer struct */
    typedef struct
    {
        int** array;    /* 2D malloc Array, size [size][2]                    */
        int   size;     /* Size of 1st dimension of array                     */
        int   start;    /* Index of starting element                          */
        int   end;      /* Index of last element                              */
        int   count;    /* Number of populated elements                       */
        int   complete; /* Boolean if requester is finished                   */
    } buffer;

    /* Function forward declarations */
    buffer* buffer_init       (int);
    int     buffer_isEmpty    (buffer*);
    int     buffer_isFull     (buffer*);
    int     buffer_isComplete (buffer*);
    int     buffer_enqueue    (buffer*, int,  int );
    int     buffer_dequeue    (buffer*, int*, int*);
    void    buffer_setComplete(buffer*);
    void    buffer_destroy    (buffer*);
    void    buffer_print      (buffer*);

#endif