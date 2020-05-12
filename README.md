# Operating Systems (Curtin University)

A 2nd-year unit taken in Semester 1 2020.

From the unit handbook:

The operating systems are a critical part of any computer systems, and therefore
it is important for all computing students to have sufficient knowledge of 
operating systems that evolve with the development of computer technologies. In 
this unit students will learn the following main OS components: process, thread, 
and their communication and synchronizations; CPU scheduling algorithms; 
deadlock detection, prevention, and avoidance; memory management, including 
memory allocation, memory paging and segmentation, and virtual memory; file 
system interface, implementation, and access methods; disk structure, 
scheduling, management and reliability; I/O management; and protection and 
security concepts and implementations.

## Assignment

A long term coding assignment about thread/process synchronisation in C.
Simulates 3 lifts serving lift requests created my a requester in a 
bounded-buffer producer/consumer problem. Part A of the assignment uses threads
to run the multiple lifts and reqeuester using POSIX Pthreads and Pthread's
mutex and cond for mutual exclusion, and Part B uses forked processes with POSIX
memory sharing and semaphores for mutual exclsion. See readme inside for details
on how to run/compile.