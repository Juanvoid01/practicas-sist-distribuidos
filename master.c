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

    if (!world || !currentWorld)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    initRandomWorld(world, worldWidth, worldHeight);

    unsigned int worldPartHeight = worldHeight / numWorkers;
    unsigned int worldPartSize = worldPartHeight * worldWidth;

    unsigned short *firstRow = world;
    unsigned short *lastRow = world + worldSize - worldWidth;

    for (int j = 1; j < numProcess; j++)
    {
        printf("Sending worldPartHeight %d\n", j );
        MPI_Send(&worldPartHeight, 1, MPI_UNSIGNED, j , 7, MPI_COMM_WORLD);
    }
    printf("Finish sending worldPartHeight \n");

    for (int iteration = 0; iteration < totalIterations; iteration++)
    {
        printf("Starting iteration %d\n", iteration);

        memcpy(currentWorld, world, worldSize * sizeof(unsigned short));

        printf("Finish memcpy\n");

        // Distribución estática o dinámica
        unsigned short *worldPart[numWorkers];
        if (distModeStatic)
        {
            for (int j = 1; j < numProcess; j++)
            {
                int signal = j >= numProcess - 1 ? END_PROCESSING : j;

                printf("Sending signal %d\n", j);

                MPI_Send(&signal, 1, MPI_INTEGER, j , 9, MPI_COMM_WORLD);

                printf("Finish sending signal\n");

                worldPart[j-1] = world + worldPartSize * (j-1);
                unsigned short *topWorldPart = j > 1 ? worldPart[0] - worldWidth : lastRow;
                unsigned short *bottomWorldPart = j >= numProcess - 1 ? worldPart[numWorkers - 1] + worldWidth : firstRow;

                printf("Sending worldPart %d\n", j);

                MPI_Send(worldPart[j], worldPartSize, MPI_UNSIGNED_SHORT, j, 0, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 1, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);

                printf("Finish sending worldPart\n");
            }
        }
        else
        {
            // Distribución dinámica: asigna porciones de tamaño `grainSize`
        }

        // Recibe los resultados de cada worker
        for (int j = 1; j < numProcess; j++)
        {
            // Recibe de cada worker
            MPI_Recv(worldPart[j-1], worldPartSize, MPI_UNSIGNED_SHORT, j, 0, MPI_COMM_WORLD, &status);
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