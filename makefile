# OS Assignment

# Makefile variables

gcc = gcc -Wall -ansi -Werror -pedantic -pthread -g
execA = lift_sim_A
execB = lift_sim_B
all = Lift.o Request.o Buffer.o ${execA}

all: ${all}

# Compilation

Buffer.o : Buffer.c Buffer.h
	${gcc} Buffer.c -c
	
Lift.o : Lift.c Lift.h Buffer.o
	${gcc} Lift.c -c

Request.o : Request.c Request.h Buffer.o
	${gcc} Request.c -c

${execA} : ${execA}.c Lift.o Request.o Buffer.o
	${gcc} Lift.o Request.o Buffer.o ${execA}.c -o ${execA}

# Clean

clean :
	rm ${all}

# NOTE - Remove -g flag