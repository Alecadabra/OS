/* Simulates 3 consumer lifts serving floor requests created by a producer using
 * a bounded buffer. Implemented using threads with POSIX pthreads */

#ifndef LIFT_SIM_A_H

    #define LIFT_SIM_A_H

    /* Function foward declarations */
    void* lift    (void*);
    void* request (void*);

#endif