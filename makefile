# OS Assignment

# Makefile variables

gcc   = gcc -Wall -ansi -Werror -pedantic
linkA = -pthread
linkB = -lrt -pthread
execA = lift_sim_A
execB = lift_sim_B
all   = Buffer.o ${execA}

all: ${all}

# Compilation

Buffer.o : Buffer.c Buffer.h
	${gcc} Buffer.c -c

${execA} : ${execA}.c ${execA}.h Buffer.o
	${gcc} ${linkA} Buffer.o ${execA}.c -o ${execA}

${execB} : ${execB}.c ${execB}.h Buffer.o
	${gcc} ${linkB} Buffer.o ${execB}.c -o ${execB}

# Clean

clean :
	rm ${all}

# Run

a : clean all
	./lift_sim_A 50 0

b : clean all ${execB}
	./lift_sim_B 50 0