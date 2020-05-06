# OS Assignment

# Makefile variables

gcc = gcc -Wall -ansi -Werror -pedantic -pthread
execA = lift_sim_A
all = Buffer.o ${execA}

all: ${all}

# Compilation

Buffer.o : Buffer.c Buffer.h
	${gcc} Buffer.c -c

${execA} : ${execA}.c ${execA}.h Buffer.o
	${gcc} Buffer.o ${execA}.c -o ${execA}

# Clean

clean :
	rm ${all}

# NOTE - Remove -g flag