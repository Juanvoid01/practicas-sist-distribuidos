#include "worker.h"
#include "types.h"

static void updateCell(tCoordinate *cellCoord,
                       unsigned short *worldPart,
                       unsigned short *topWorldPart,
                       unsigned short *bottomWorldPart,
                       int worldWidth,
                       int worldPartHeight);

static void updateWorld(unsigned short *worldPart,
                        unsigned short *topWorldPart,
                        unsigned short *bottomWorldPart,
                        int worldWidth,
                        int worldPartHeight);

void executeWorker(int worldWidth)
{
    MPI_Status status;
    int worldPartHeight = 0;
    int worldPartSize = 0;

    unsigned short *worldPart = NULL;
    unsigned short *topWorldPart = (unsigned short *)malloc(worldWidth * sizeof(unsigned short));
    unsigned short *bottomWorldPart = (unsigned short *)malloc(worldWidth * sizeof(unsigned short));

    while (1)
    {
        int newWorldPartHeight = 0;
        // recibe el numero de filas a procesar
        MPI_Recv(&newWorldPartHeight, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD, &status);

        if (newWorldPartHeight == END_PROCESSING)
            break;

        // resize buffer only if necessary
        if (newWorldPartHeight != worldPartHeight)
        {
            if (worldPart != NULL)
            {
                free(worldPart);
            }

            worldPartHeight = newWorldPartHeight;
            worldPartSize = worldWidth * worldPartHeight;

            worldPart = (unsigned short *)malloc(worldPartSize * sizeof(unsigned short));
        }

        // Recibe la porción de mundo a procesar
        MPI_Recv(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, MASTER, 2, MPI_COMM_WORLD, &status);
        // Recibe la fila superior de la parte del mundo
        MPI_Recv(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, MASTER, 3, MPI_COMM_WORLD, &status);
        // Recibe la fila inferior de la parte del mundo
        MPI_Recv(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, MASTER, 4, MPI_COMM_WORLD, &status);

        updateWorld(worldPart, topWorldPart, bottomWorldPart, worldWidth, worldPartHeight);

        // Envía el resultado de la porción procesada de vuelta al master
        MPI_Send(&worldPartSize, 1, MPI_INT, MASTER, 5, MPI_COMM_WORLD);
        MPI_Send(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, MASTER, 6, MPI_COMM_WORLD);
    }

    if (worldPart != NULL)
    {
        free(worldPart);
    }
    free(topWorldPart);
    free(bottomWorldPart);
}

static void updateCell(tCoordinate *cellCoord,
                       unsigned short *worldPart,
                       unsigned short *topWorldPart,
                       unsigned short *bottomWorldPart,
                       int worldWidth,
                       int worldPartHeight)
{
    tCoordinate cellCoordUp;
    tCoordinate cellCoordDown;
    tCoordinate cellCoordRight;
    tCoordinate cellCoordLeft;
    tCoordinate cellCoordUpLeft;
    tCoordinate cellCoordDownLeft;
    tCoordinate cellCoordUpRight;
    tCoordinate cellCoordDownRight;

    getCellLeft(cellCoord, worldWidth, &cellCoordLeft);
    getCellRight(cellCoord, worldWidth, &cellCoordRight);

    if (cellCoord->row > 0)
    {
        getCellUp(cellCoord, &cellCoordUp);
        getCellUp(&cellCoordLeft, &cellCoordUpLeft);
        getCellUp(&cellCoordRight, &cellCoordUpRight);
    }

    if (cellCoord->row < worldPartHeight - 1)
    {
        getCellDown(cellCoord, &cellCoordDown);
        getCellDown(&cellCoordLeft, &cellCoordDownLeft);
        getCellDown(&cellCoordRight, &cellCoordDownRight);
    }

    unsigned short cell = getCellAtWorld(cellCoord, worldPart, worldWidth);
    unsigned int neighbours = 0;

    // Check left
    if (getCellAtWorld(&cellCoordLeft, worldPart, worldWidth) == CELL_LIVE)
    {
        neighbours++;
    }
    // Check right
    if (getCellAtWorld(&cellCoordRight, worldPart, worldWidth) == CELL_LIVE)
    {
        neighbours++;
    }

    // Check up, upLeft and upRight
    if (cellCoord->row > 0)
    {
        if (getCellAtWorld(&cellCoordUp, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
        if (getCellAtWorld(&cellCoordUpLeft, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
        if (getCellAtWorld(&cellCoordUpRight, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
    }
    else
    {
        int rightCol = cellCoord->col < worldWidth - 1 ? cellCoord->col + 1 : 0;
        int leftCol = cellCoord->col > 0 ? cellCoord->col - 1 : worldWidth - 1;

        if (topWorldPart[cellCoord->col] == CELL_LIVE)
        {
            neighbours++;
        }
        if (topWorldPart[rightCol] == CELL_LIVE)
        {
            neighbours++;
        }
        if (topWorldPart[leftCol] == CELL_LIVE)
        {
            neighbours++;
        }
    }

    // Check down, downLeft and downRight
    if (cellCoord->row < worldPartHeight - 1)
    {
        if (getCellAtWorld(&cellCoordDown, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
        if (getCellAtWorld(&cellCoordDownLeft, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
        if (getCellAtWorld(&cellCoordDownRight, worldPart, worldWidth) == CELL_LIVE)
        {
            neighbours++;
        }
    }
    else
    {
        int rightCol = cellCoord->col < worldWidth - 1 ? cellCoord->col + 1 : 0;
        int leftCol = cellCoord->col > 0 ? cellCoord->col - 1 : worldWidth - 1;

        if (bottomWorldPart[cellCoord->col] == CELL_LIVE)
        {
            neighbours++;
        }
        if (bottomWorldPart[rightCol] == CELL_LIVE)
        {
            neighbours++;
        }
        if (bottomWorldPart[leftCol] == CELL_LIVE)
        {
            neighbours++;
        }
    }

    // Lonely cell?
    if (neighbours == 0)
    {
        calculateLonelyCell();
    }

    if (cell == CELL_LIVE && ((neighbours == 2) || (neighbours == 3)))
    { // Cell is still alive
        setCellAt(cellCoord, worldPart, worldWidth, CELL_LIVE);
    }
    else if (cell == CELL_EMPTY && (neighbours == 3))
    { // New cell is born
        setCellAt(cellCoord, worldPart, worldWidth, CELL_LIVE);
    }
    else
    { // Cell is dead
        setCellAt(cellCoord, worldPart, worldWidth, CELL_EMPTY);
    }

    // cataclysm
    if (cell == CELL_CATACLYSM)
    {
        setCellAt(cellCoord, worldPart, worldWidth, CELL_DEAD);
    }
}

static void updateWorld(unsigned short *worldPart,
                        unsigned short *topWorldPart,
                        unsigned short *bottomWorldPart,
                        int worldWidth,
                        int worldPartHeight)
{

    tCoordinate cell;

    for (int col = 0; col < worldWidth; col++)
        for (int row = 0; row < worldPartHeight; row++)
        {
            cell.row = row;
            cell.col = col;
            updateCell(&cell, worldPart, topWorldPart, bottomWorldPart, worldWidth, worldPartHeight);
        }
}
