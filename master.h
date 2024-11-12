#include "graph.h"
#include "mpi.h"

// Enables/Disables the log messages from the master process
#define DEBUG_MASTER 0

// Probability that a cataclysm may occur [0-100] :(
#define PROB_CATACLYSM 100

// Number of iterations between two possible cataclysms
#define ITER_CATACLYSM 5


