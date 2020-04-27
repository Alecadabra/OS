#ifndef LIFT_SIM_A_H

    #define LIFT_SIM_A_H

    #include "Buffer.h"

    typedef struct {
        buffer* buffer;   /* Ptr to buffer that stores floor requests */
        int liftNum;      /* This lift's number */
        int t;            /* Setting of time taken for lift to move */
    } liftInput;

    void* lift(void*);
    void* request(void*);

#endif