/* Control and store FIFO array buffer initialised with either heap or shared
 * memory allocation */

#ifndef BUFFER_H

    #define BUFFER_H

    #define BUFF_NAME  "/lift_buffer"       /* Shared name of buffer          */
    #define ARRAY_NAME "/lift_buffer_array" /* Shared name of array in buffer */

    /* Buffer struct */
    typedef struct
    {
        int** array;    /* 2D malloc Array, size [size][2]                    */
        int   size;     /* Size of 1st dimension of array                     */
        int   head;     /* Index of starting element                          */
        int   tail;     /* Index of last element                              */
        int   count;    /* Number of populated elements                       */
        int   complete; /* Boolean if requester is finished                   */
    } buffer;

    /* Function forward declarations */
    buffer* buffer_init       (int);
    buffer* buffer_open       (int, int*);
    int     buffer_isEmpty    (buffer*);
    int     buffer_isFull     (buffer*);
    int     buffer_isComplete (buffer*);
    int     buffer_enqueue    (buffer*, int,  int );
    int     buffer_dequeue    (buffer*, int*, int*);
    void    buffer_setComplete(buffer*);
    void    buffer_destroy    (buffer*);
    void    buffer_close      (buffer*, int*);

#endif