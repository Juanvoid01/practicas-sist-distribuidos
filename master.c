#include "master.h"
#include "world.h"
#include "types.h"

static void cataclysm(unsigned short *world, const int iteration, const int worldWidth, const int worldHeight);

void executeMaster(SDL_Window *window, SDL_Renderer *renderer, int worldWidth, int worldHeight, int numProcess,
                   int totalIterations, int autoMode, int distModeStatic, int grainSize, char *outputFile)
{
    unsigned short **processWorldPart = (unsigned short **)malloc(numProcess * sizeof(unsigned short *));

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

    unsigned short *firstRow = world;
    unsigned short *lastRow = world + worldSize - worldWidth;

    for (int iteration = 0; iteration < totalIterations; iteration++)
    {

        memcpy(currentWorld, world, worldSize * sizeof(unsigned short));

        // Distribución estática o dinámica
        if (distModeStatic)
        {

            int worldPartHeight = worldHeight / numWorkers;
            int worldPartSize = worldPartHeight * worldWidth;
            int lastWorkerWorldPartHeight = worldPartHeight + worldHeight % numWorkers;
            int lastWorkerWorlPartSize = lastWorkerWorldPartHeight * worldWidth;

            for (int j = 1; j <= numWorkers; j++)
            {
                int heightPart = j < numWorkers ? worldPartHeight : lastWorkerWorldPartHeight;
                MPI_Send(&heightPart, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
            }

            for (int j = 1; j <= numWorkers; j++)
            {

                processWorldPart[j] = world + worldPartSize * (j - 1);
                unsigned short *topWorldPart = j > 1 ? processWorldPart[j] - worldWidth : lastRow;
                unsigned short *bottomWorldPart = j < numWorkers ? processWorldPart[j] + worldWidth : firstRow;

                int sizePart = j < numWorkers ? worldPartSize : lastWorkerWorlPartSize;

                MPI_Send(processWorldPart[j], sizePart, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 3, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 4, MPI_COMM_WORLD);
            }

            // Recibe los resultados de cada worker
            for (int j = 1; j <= numWorkers; j++)
            {
                int worldPartSizeReceived = 0;
                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, j, 5, MPI_COMM_WORLD, &status);
                MPI_Recv(processWorldPart[j], worldPartSizeReceived, MPI_UNSIGNED_SHORT, j, 6, MPI_COMM_WORLD, &status);
            }
        }
        else
        {
            // Distribución dinámica: asigna porciones de tamaño `grainSize`
            int contRows = 0;
            int nextWorker = 1;
            int toReceive = 0;

            while (contRows < worldHeight)
            {
                int worldPartSize = grainSize * worldWidth;

                unsigned short *worldPart = world + contRows * worldWidth;

                int rowsLeft = worldHeight - contRows;
                if (grainSize <= rowsLeft)
                {
                    MPI_Send(&grainSize, 1, MPI_INT, nextWorker, 1, MPI_COMM_WORLD);

                    unsigned short *topWorldPart = contRows > 0 ? worldPart - worldWidth : lastRow;
                    unsigned short *bottomWorldPart = grainSize < rowsLeft ? worldPart + worldWidth : firstRow;

                    MPI_Send(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, nextWorker, 2, MPI_COMM_WORLD);
                    MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, nextWorker, 3, MPI_COMM_WORLD);
                    MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, nextWorker, 4, MPI_COMM_WORLD);

                    processWorldPart[nextWorker] = worldPart;
                }
                else
                {
                    MPI_Send(&rowsLeft, 1, MPI_INT, nextWorker, 1, MPI_COMM_WORLD);

                    unsigned short *topWorldPart = contRows > 0 ? worldPart - worldWidth : lastRow;
                    unsigned short *bottomWorldPart = firstRow;

                    MPI_Send(worldPart, worldPartSize, MPI_UNSIGNED_SHORT, nextWorker, 2, MPI_COMM_WORLD);
                    MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, nextWorker, 3, MPI_COMM_WORLD);
                    MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, nextWorker, 4, MPI_COMM_WORLD);

                    break;
                }

                contRows += grainSize;
                toReceive++;

                int foundNext = 0;

                nextWorker = nextWorker == numWorkers ? 1 : nextWorker++;

                if (toReceive >= numWorkers)
                {
                    int worldPartSizeReceived = 0;
                    MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status);
                    MPI_Recv(processWorldPart[status.MPI_SOURCE], worldPartSizeReceived, MPI_UNSIGNED_SHORT, status.MPI_SOURCE, 6, MPI_COMM_WORLD, &status);
                    toReceive--;
                }
            }

            while (toReceive > 0)
            {
                int worldPartSizeReceived = 0;

                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status);
                MPI_Recv(processWorldPart[status.MPI_SOURCE], worldPartSizeReceived, MPI_UNSIGNED_SHORT, status.MPI_SOURCE, 6, MPI_COMM_WORLD, &status);
                toReceive--;
            }
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

    int signal_end = END_PROCESSING;

    for (int j = 1; j <= numWorkers; j++)
    {
        MPI_Send(&signal_end, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
    }

    free(processWorldPart);
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