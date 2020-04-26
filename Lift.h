#ifndef LIFT_H

    #define LIFT_H

    typedef struct {
        int*** bufferPtr;
        int m;
        int t;
        int liftNum;
    } liftInput;

    void* lift(void*);

#endif

