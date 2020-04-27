#ifndef LIFT_H

    #define LIFT_H

    typedef struct {
        int*** bufferPtr; /* Ptr to buffer that stores floor requests */
        int m;            /* Setting of buffer size */
        int t;            /* Setting of time taken for lift to move */
        int liftNum;      /* This lift's number */
    } liftInput;

    void* lift(void*);

#endif

