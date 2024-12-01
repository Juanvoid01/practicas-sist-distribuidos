#include "master.h"
#include "world.h"
#include "types.h"

static void cataclysm(unsigned short *world, const int iteration, const int worldWidth, const int worldHeight);

void executeMaster(SDL_Window *window, SDL_Renderer *renderer, int worldWidth, int worldHeight, int numProcess,
                   int totalIterations, int autoMode, int distModeStatic, int grainSize, char *outputFile)
{
    MPI_Status status;

    const int worldSize = worldWidth * worldHeight;
    const int numWorkers = numProcess - 1;

    unsigned short *world = (unsigned short *)malloc(worldSize * sizeof(unsigned short));
    unsigned short *currentWorld = (unsigned short *)malloc(worldSize * sizeof(unsigned short));

    initRandomWorld(world, worldWidth, worldHeight);

    int worldPartHeight = worldHeight / numWorkers;
    int worldPartSize = worldPartHeight * worldWidth;

    unsigned short *firstRow = world;
    unsigned short *lastRow = world + worldSize - worldWidth;

    for (int iteration = 0; iteration < totalIterations; iteration++)
    {
        memcpy(currentWorld, world, worldSize);

        // Distribución estática o dinámica
        unsigned short* worldPart[numWorkers];
        if (distModeStatic)
        {
            for (int j = 0; j < numWorkers; j++)
            {
                MPI_Send(&worldPartHeight, 1, MPI_UNSIGNED, j + 1, 7, MPI_COMM_WORLD);

                worldPart[j] = world + worldPartSize * j;
                unsigned short *topWorldPart = j > 1 ? worldPart [j]- worldWidth : lastRow;
                unsigned short *bottomWorldPart = j < numWorkers - 1 ? worldPart[j] + worldWidth : firstRow;

                MPI_Send(worldPart[j], worldPartSize, MPI_UNSIGNED_SHORT, j + 1, 0, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j + 1, 1, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j + 1, 2, MPI_COMM_WORLD);
            }
        }
        else
        {
            // Distribución dinámica: asigna porciones de tamaño `grainSize`
        }

        // Recibe los resultados de cada worker
        for (int j = 0; j < numWorkers; j++)
        {
            // Recibe de cada worker
            MPI_Recv(worldPart[j], worldPartSize, MPI_UNSIGNED_SHORT, j + 1, 0, MPI_COMM_WORLD, &status);
        }

        // Aplica el cataclismo
        cataclysm(world, iteration, worldWidth, worldHeight);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        drawWorld(currentWorld,
                  world,
                  renderer,
                  0,
                  worldHeight,
                  worldWidth,
                  worldHeight);

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);

        saveImage(renderer, outputFile, worldWidth * CELL_SIZE, worldHeight * CELL_SIZE);
        // Modo paso a paso
        if (autoMode == 0)
            getchar();
    }

    free(world);
    free(currentWorld);
}

static void cataclysm(unsigned short *world, const int iteration, const int worldWidth, const int worldHeight)
{
    if ((iteration + 1) % ITER_CATACLYSM == 0 && (rand() % 100 < PROB_CATACLYSM))
    {
        const int middleRow = worldHeight / 2 + 1;
        const int middleCol = worldWidth / 2 + 1;

        for (int row = 0; row < worldWidth; row++)
        {
            tCoordinate middleAux = (tCoordinate){row, middleCol};
            setCellAt(&middleAux, world, worldWidth, CELL_CATACLYSM);
        }

        for (int col = 0; col < worldHeight; col++)
        {
            tCoordinate middleAux = (tCoordinate){middleRow, col};
            setCellAt(&middleAux, world, worldWidth, CELL_CATACLYSM);
        }
    }
}