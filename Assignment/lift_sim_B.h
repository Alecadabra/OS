/* Simulates 3 consumer lifts serving floor requests created by a producer using
 * a bounded buffer. Implemented using processes with POSIX shared memory */

#ifndef LIFT_SIM_B_H

    #define LIFT_SIM_B_H

    /* Function foward declarations */
    void lift   (int, int);
    void request();

#endif