#include "master.h"
#include "world.h"
#include "types.h"

void executeMaster(SDL_Window *window, SDL_Renderer *renderer, int worldWidth, int worldHeight, 
                   int totalIterations, int autoMode, int distModeStatic, int grainSize, char *outputFile) {
    
    unsigned short *world = (unsigned short*) malloc(worldWidth * worldHeight * sizeof(unsigned short));
    initRandomWorld(world, worldWidth, worldHeight);
    
    for (int iteration = 0; iteration < totalIterations; iteration++) {
        // Distribución estática o dinámica
        if (distModeStatic) {
            // Envía partes del tablero a cada worker de forma estática
        } else {
            // Distribución dinámica: asigna porciones de tamaño `grainSize`
        }
        
        // Recibe los resultados de cada worker
        //for (int i = 1; i < size; i++) {
            // Recibe de cada worker
        //}

     /*drawWorld ( currentWorld, 
			     newWorld, 
				renderer,
				 firstRow,
				 lastRow,
				 worldWidth, 
				 worldHeight)
    */
        // Aplica el cataclismo si es necesario
        //if ((iteration + 1) % ITER_CATACLYSM == 0 && (rand() % 100 < PROB_CATACLYSM)) {
            // Código para aplicar cataclismo
        //}

        // Modo paso a paso
        //if (autoMode == 0) getchar();
    }

    saveImage(renderer, outputFile, worldWidth * CELL_SIZE, worldHeight * CELL_SIZE);
    free(world);
}
