PATH=$PATH:$MPI_HOME/bin
/home/juan/openmpi-2.0.1/bin/mpicc -o lifeGame *.c -I/usr/include/SDL2 -L/usr/lib/ -lSDL2