#ifndef LIFT_H

    #define LIFT_H

    typedef struct {
        buffer* buffer;   /* Ptr to buffer that stores floor requests */
        int liftNum;      /* This lift's number */
        int t;            /* Setting of time taken for lift to move */
    } liftInput;

    void* lift(void*);

#endif

