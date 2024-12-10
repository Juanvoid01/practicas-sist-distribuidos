#include "master.h"
#include "world.h"
#include "types.h"
#include <unistd.h>

static void cataclysm(unsigned short *world, const int iteration, const int worldWidth, const int worldHeight);

void executeMaster(SDL_Window *window, SDL_Renderer *renderer, int worldWidth, int worldHeight, int numProcess,
                   int totalIterations, int autoMode, int distModeStatic, int grainSize, char *outputFile)
{
    unsigned short **processWorldPart = (unsigned short **)malloc(numProcess * sizeof(unsigned short *));

    MPI_Status status;
    SDL_Event event;
    int isquit = 0;
    char ch;

    const int worldSize = worldWidth * worldHeight;
    int numWorkers = numProcess - 1;

    unsigned short *world = (unsigned short *)malloc(worldSize * sizeof(unsigned short));
    unsigned short *newWorld = (unsigned short *)malloc(worldSize * sizeof(unsigned short));

    if (!world || !newWorld)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    clearWorld(world, worldWidth, worldHeight);

    initRandomWorld(world, worldWidth, worldHeight);

    unsigned short *firstRow = world;
    unsigned short *lastRow = world + worldSize - worldWidth;

    if (distModeStatic)
    {
        // static ---------------------------------------------------------------------------------------------

        if (numWorkers > worldHeight)
        {
            numWorkers = worldHeight;
        }

        int worldPartHeight = worldHeight / numWorkers;
        int worldPartSize = worldPartHeight * worldWidth;
        int lastWorkerWorldPartHeight = worldPartHeight + worldHeight % numWorkers;
        int lastWorkerWorldPartSize = lastWorkerWorldPartHeight * worldWidth;

        for (int iteration = 0; iteration < totalIterations && !isquit; iteration++)
        {

            for (int j = 1; j <= numWorkers; j++)
            {
                int heightPart = j < numWorkers ? worldPartHeight : lastWorkerWorldPartHeight;
                MPI_Send(&heightPart, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
            }

            for (int j = 1; j <= numWorkers; j++)
            {
                unsigned short *worldPart = world + worldPartSize * (j - 1);
                unsigned short *topWorldPart = j > 1 ? worldPart - worldWidth : lastRow;
                unsigned short *bottomWorldPart = j < numWorkers ? worldPart + worldWidth : firstRow;

                int sizePart = j < numWorkers ? worldPartSize : lastWorkerWorldPartSize;

                MPI_Send(worldPart, sizePart, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 3, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 4, MPI_COMM_WORLD);

                processWorldPart[j] = newWorld + worldPartSize * (j - 1);
            }

            // Recibe los resultados de cada worker
            for (int j = 1; j <= numWorkers; j++)
            {
                int worldPartSizeReceived = 0;
                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status);
                int workerId = status.MPI_SOURCE;
                MPI_Recv(processWorldPart[workerId], worldPartSizeReceived, MPI_UNSIGNED_SHORT, workerId, 6, MPI_COMM_WORLD, &status);
            }

            // Aplica el cataclismo
            cataclysm(newWorld, iteration, worldWidth, worldHeight);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(renderer);

            drawWorld(world,
                      newWorld,
                      renderer,
                      0,
                      worldHeight,
                      worldWidth,
                      worldHeight);

            unsigned short *tempWorld = world;
            world = newWorld;
            newWorld = tempWorld;

            // Update the surface
            SDL_RenderPresent(renderer);
            SDL_UpdateWindowSurface(window);

            // Read event
            if (SDL_PollEvent(&event))
                if (event.type == SDL_QUIT)
                    isquit = 1;

            if (!autoMode)
            {
                printf("Press Enter to continue...\n");
                ch = getchar();
            }
        }
        int signal_end = END_PROCESSING;

        for (int j = 1; j <= numWorkers; j++)
        {
            MPI_Send(&signal_end, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        // dynamic ---------------------------------------------------------------------------------------------

        for (int iteration = 0; iteration < totalIterations; iteration++)
        {
            // Distribución dinámica: asigna porciones de tamaño `grainSize`
            int currentRow = 0;
            unsigned short *auxPtrWorld = newWorld;

            for (int j = 1; j <= numWorkers && currentRow < worldHeight; j++)
            {
                int rowsLeft = worldHeight - currentRow;
                int rowsSent = rowsLeft >= grainSize ? grainSize : rowsLeft;

                int worldPartSize = rowsSent * worldWidth;

                // Send the number of rows to be processed
                MPI_Send(&rowsSent, 1, MPI_INT, j, 1, MPI_COMM_WORLD);

                // Send the rows data
                unsigned short *topWorldPart = currentRow > 0 ? auxPtrWorld - worldWidth : lastRow;
                unsigned short *bottomWorldPart = grainSize < rowsLeft ? auxPtrWorld + worldWidth : firstRow;

                MPI_Send(auxPtrWorld, worldPartSize, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 3, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 4, MPI_COMM_WORLD);

                // Update pointer and index

                processWorldPart[j] = auxPtrWorld;
                currentRow += rowsSent;
                auxPtrWorld += worldPartSize;
            }
            printf("terminada distribucion\n");

            int processedSize = 0;

            while (processedSize < worldSize)
            {
                printf("pidiendo processedRows\n");

                int worldPartSizeReceived = 0;
                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status);
                printf("recibido worldPartSizeReceived %d\n", worldPartSizeReceived);

                int workerId = status.MPI_SOURCE;

                MPI_Recv(processWorldPart[workerId], worldPartSizeReceived, MPI_UNSIGNED_SHORT, workerId, 6, MPI_COMM_WORLD, &status);

                printf("recibido processedSize %d\n", processedSize);

                processedSize += worldPartSizeReceived;

                // Send remaining rows...
                if (currentRow < worldHeight)
                {
                    int rowsLeft = worldHeight - currentRow;
                    int rowsSent = rowsLeft >= grainSize ? grainSize : rowsLeft;

                    int worldPartSize = rowsSent * worldWidth;

                    // Send the number of rows to be processed
                    MPI_Send(&rowsSent, 1, MPI_INT, workerId, 1, MPI_COMM_WORLD);

                    // Send the rows data
                    unsigned short *topWorldPart = currentRow > 0 ? auxPtrWorld - worldWidth : lastRow;
                    unsigned short *bottomWorldPart = grainSize < rowsLeft ? auxPtrWorld + worldWidth : firstRow;

                    MPI_Send(auxPtrWorld, worldPartSize, MPI_UNSIGNED_SHORT, workerId, 2, MPI_COMM_WORLD);
                    MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, workerId, 3, MPI_COMM_WORLD);
                    MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, workerId, 4, MPI_COMM_WORLD);

                    // Update pointer and index
                    processWorldPart[workerId] = auxPtrWorld;
                    currentRow += rowsSent;
                    auxPtrWorld += worldPartSize;
                }
                else
                {
                    /*if (iteration == totalIterations - 1)
                    {
                        int signal_end = END_PROCESSING;
                        MPI_Send(&signal_end, 1, MPI_INT, workerId, 1, MPI_COMM_WORLD);
                    }*/
                }
            }

            printf("terminado procesamiento\n");

            // Aplica el cataclismo
            cataclysm(newWorld, iteration, worldWidth, worldHeight);

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(renderer);

            drawWorld(world,
                      newWorld,
                      renderer,
                      0,
                      worldHeight,
                      worldWidth,
                      worldHeight);

            unsigned short *tempWorld = world;
            world = newWorld;
            newWorld = tempWorld;

            // Update the surface
            SDL_RenderPresent(renderer);
            SDL_UpdateWindowSurface(window);

            // Read event
            if (SDL_PollEvent(&event))
                if (event.type == SDL_QUIT)
                    isquit = 1;

            if (!autoMode)
            {
                printf("Press Enter to continue...\n");
                ch = getchar();
            }
        }
        int signal_end = END_PROCESSING;

        for (int j = 1; j <= numWorkers; j++)
        {
            MPI_Send(&signal_end, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
        }
    }

    // Save file?
    if (outputFile != NULL)
    {
        saveImage(renderer, outputFile, worldWidth * CELL_SIZE, worldHeight * CELL_SIZE);
    }

    // Game over
    printf("Game Over!!! Press Enter to continue...");
    ch = getchar();

    free(processWorldPart);
    free(world);
    free(newWorld);

    // Destroy window
    SDL_DestroyWindow(window);

    // Exiting...
    SDL_Quit();
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