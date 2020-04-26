# OS Assignment

# Makefile variables

gcc = gcc -Wall -ansi -Werror -pedantic -pthread -g
execA = lift_sim_A
execB = lift_sim_B
all = Lift.o Request.o ${execA}

all: ${all}

# Compilation

Lift.o : Lift.c Lift.h
	${gcc} Lift.c -c

Request.o : Request.c Request.h
	${gcc} Request.c -c

${execA} : ${execA}.c Lift.o Request.o
	${gcc} Lift.o Request.o ${execA}.c -o ${execA}

# Clean

clean :
	rm ${all}

# NOTE - Remove -g flag