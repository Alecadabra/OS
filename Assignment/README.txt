Lift Simulator Readme

--------------------------------------------------------------------------------

Curtin University - Operating Systems (COMP2006) 2020 S1
Assignment - Lift Simulator
Alec Maughan

--------------------------------------------------------------------------------

Simulates 3 consumer lifts serving floor requests created by a producer using a
bounded buffer. Implemented using both threads (Part A) and processes (Part B)

--------------------------------------------------------------------------------

Compile entire program with `make`

--------------------------------------------------------------------------------

Run part A with `./lift_sim_A m t`
Run part B with `./lift_sim_B m t`

m is the maximum size of buffer
t is the time taken for a lift to move in seconds

sim_input file provided, each line has a source and destination floor separated
by a space. This file must be between 50 and 100 lines inclusive.

Program will write logs to sim_out

--------------------------------------------------------------------------------

Samples in /samples directory, see /samples/README.txt
