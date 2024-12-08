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
    const int numWorkers = numProcess - 1;

    unsigned short *world = (unsigned short *)malloc(worldSize * sizeof(unsigned short));
    unsigned short *newWorld = (unsigned short *)malloc(worldSize * sizeof(unsigned short));

    if (!world || !newWorld)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    initRandomWorld(world, worldWidth, worldHeight);

    unsigned short *firstRow = world;
    unsigned short *lastRow = world + worldSize - worldWidth;

    if (distModeStatic)
    {
        // static ---------------------------------------------------------------------------------------------

        for (int iteration = 0; iteration < totalIterations && !isquit; iteration++)
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
                unsigned short *worldPart = world + worldPartSize * (j - 1);
                unsigned short *topWorldPart = j > 1 ? worldPart - worldWidth : lastRow;
                unsigned short *bottomWorldPart = j < numWorkers ? worldPart + worldWidth : firstRow;

                int sizePart = j < numWorkers ? worldPartSize : lastWorkerWorlPartSize;

                MPI_Send(worldPart, sizePart, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 3, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 4, MPI_COMM_WORLD);

                processWorldPart[j] = newWorld + worldPartSize * (j - 1);
            }

            // Recibe los resultados de cada worker
            for (int j = 1; j <= numWorkers; j++)
            {
                int worldPartSizeReceived = 0;
                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, j, 5, MPI_COMM_WORLD, &status);
                MPI_Recv(processWorldPart[j], worldPartSizeReceived, MPI_UNSIGNED_SHORT, j, 6, MPI_COMM_WORLD, &status);
            }

            // Aplica el cataclismo
            cataclysm(world, iteration, worldWidth, worldHeight);

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

            for (int j = 1; j <= numWorkers; j++)
            {

                int worldPartSize = grainSize * worldWidth;
                int rowsLeft = worldHeight - currentRow;

                // Send the number of rows to be processed
                MPI_Send(&grainSize, 1, MPI_INT, j, 1, MPI_COMM_WORLD);

                // Send the rows data
                unsigned short *topWorldPart = currentRow > 0 ? auxPtrWorld - worldWidth : lastRow;
                unsigned short *bottomWorldPart = grainSize < rowsLeft ? auxPtrWorld + worldWidth : firstRow;

                MPI_Send(auxPtrWorld, worldPartSize, MPI_UNSIGNED_SHORT, j, 2, MPI_COMM_WORLD);
                MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 3, MPI_COMM_WORLD);
                MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, j, 4, MPI_COMM_WORLD);

                processWorldPart[j] = auxPtrWorld;

                // Update pointer and index
                currentRow += grainSize;
                auxPtrWorld += (grainSize * worldPartSize);
            }

            printf("finished1\n");

            int processedRows = 0;

            while (processedRows < worldHeight)
            {
                int worldPartSizeReceived = 0;
                MPI_Recv(&worldPartSizeReceived, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status);
                int workerId = status.MPI_SOURCE;

                MPI_Recv(processWorldPart[workerId], worldPartSizeReceived, MPI_UNSIGNED_SHORT, status.MPI_SOURCE, 6, MPI_COMM_WORLD, &status);

                processedRows += worldPartSizeReceived / worldWidth;

                // Send remaining rows...
                if (currentRow < worldHeight)
                {
                    // Calculate number of rows to process, it can be grainSize or less if we are at the end of the processing
                    int sentRows = (currentRow + grainSize) > worldHeight ? worldHeight - currentRow : grainSize;
                    int worldPartSize = sentRows * worldWidth;

                    MPI_Send(&sentRows, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);

                    unsigned short *topWorldPart = currentRow > 0 ? auxPtrWorld - worldWidth : lastRow;
                    unsigned short *bottomWorldPart = sentRows < grainSize ? firstRow : auxPtrWorld + worldWidth;

                    MPI_Send(auxPtrWorld, worldPartSize, MPI_UNSIGNED_SHORT, workerId, 2, MPI_COMM_WORLD);
                    MPI_Send(topWorldPart, worldWidth, MPI_UNSIGNED_SHORT, workerId, 3, MPI_COMM_WORLD);
                    MPI_Send(bottomWorldPart, worldWidth, MPI_UNSIGNED_SHORT, workerId, 4, MPI_COMM_WORLD);

                    processWorldPart[workerId] = auxPtrWorld;

                    // Update pointer and index
                    currentRow += sentRows;
                    auxPtrWorld += (sentRows * worldPartSize);
                }
                else
                {
                    int signal_end = END_PROCESSING;
                    MPI_Send(&signal_end, 1, MPI_INT, workerId, 1, MPI_COMM_WORLD);
                }
            }

            // Aplica el cataclismo
            cataclysm(world, iteration, worldWidth, worldHeight);

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
    }

    // Save file?
    if (outputFile != NULL)
        saveImage(renderer, outputFile, worldWidth * CELL_SIZE, worldHeight * CELL_SIZE);

    // Destroy window
    SDL_DestroyWindow(window);

    // Game over
    printf("Game Over!!! Press Enter to continue...");
    ch = getchar();

    free(processWorldPart);
    free(world);
    free(newWorld);

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