MIXSim : Simulator.o Word.o
	gcc -o MIXSim Simulator.o Word.o


Simulator.o : Simulator.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o Simulator.o Simulator.c

Word.o : Word.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o Word.o Word.c