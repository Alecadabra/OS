# OS Assignment

# Makefile variables

gcc   = gcc -Wall -ansi -Werror -pedantic -g ################################
link = -lrt -pthread
execA = lift_sim_A
execB = lift_sim_B
all   = Buffer.o ${execA} ${execB} test

all: ${all}

# Compilation

Buffer.o : Buffer.c Buffer.h
	${gcc} Buffer.c -c ${link}

${execA} : ${execA}.c ${execA}.h Buffer.o
	${gcc} Buffer.o ${execA}.c -o ${execA} ${link}

${execB} : ${execB}.c ${execB}.h Buffer.o
	${gcc} ${execB}.c Buffer.o -o ${execB} ${link}

test : test.c Buffer.o
	${gcc} Buffer.o test.c  -o test ${link}

# Clean

clean :
	rm ${all}

# Run

a : clean all
	./lift_sim_A 50 0

b : ${execB}
	valgrind --leak-check=full --track-origins=yes ./lift_sim_B 5 0
	ipcs -m