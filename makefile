# Lift Simulator Makefile ------------------------------------------------------

# Variables --------------------------------------------------------------------

gcc   = gcc -Wall -ansi -Werror -pedantic
link  = -lrt -pthread
execA = lift_sim_A
execB = lift_sim_B
all   = Buffer.o ${execA} ${execB}

# All --------------------------------------------------------------------------

all: ${all}

# Compilation ------------------------------------------------------------------

Buffer.o : Buffer.c Buffer.h
	${gcc} Buffer.c -c ${link}

${execA} : ${execA}.c ${execA}.h Buffer.o
	${gcc} Buffer.o ${execA}.c -o ${execA} ${link}

${execB} : ${execB}.c ${execB}.h Buffer.o
	${gcc} ${execB}.c Buffer.o -o ${execB} ${link}

# Clean ------------------------------------------------------------------------

clean :
	rm ${all}
