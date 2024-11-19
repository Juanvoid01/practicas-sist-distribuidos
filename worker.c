#include "worker.h"
#include "types.h"

//int countLiveNeighbors(portion, row, col, worldWidth, worldHeight);

void executeWorker(int worldIni, int worldPartWidth, int worldPartHeight)
{
    MPI_Status status;
    void *worldPart;
    const int worldPartSize = worldPartWidth * worldPartHeight;

    // Recibe la porción de tablero a procesar
    MPI_Recv(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, MASTER, worldIni, MPI_COMM_WORLD, &status);

    unsigned char * cell = worldPart;
    unsigned char * cellUp;
    unsigned char * cellDown;
    unsigned char * cellLeft;
    unsigned char * cellRight;

    unsigned int neighbours = 0;

    for (int i = 0; i < worldPartSize; i++)
    {
        cell++;

        getCellUp(cell, cellUp);
        getCellDown(cell, cellDown);
        getCellLeft(cell, worldPartWidth, cellLeft);
        getCellRight(cell, worldPartWidth, cellRight);

        neighbours = *cellUp + *cellDown + *cellLeft + *cellRight;

        if(neighbours == 0)
        {
            calculateLonelyCell();
            *cell = CELL_DEAD;
        }
        else if(neighbours < 2)
        {
            *cell = CELL_DEAD;
        }
        else if(neighbours <= 3)
        {
            *cell = CELL_LIVE;
        }
        else if(neighbours > 3)
        {
            *cell = CELL_DEAD;
        }

    }
    
    // Envía el resultado de la porción procesada de vuelta al master
    MPI_Send(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD);
    
}
