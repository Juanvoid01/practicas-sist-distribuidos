#include "world.h"


void getCellUp (tCoordinate* c, tCoordinate* destCell){
    destCell->row = c->row-1;
    destCell->col = c->col;
}

void getCellDown (tCoordinate* c, tCoordinate* destCell){
    destCell->row = c->row+1;
    destCell->col = c->col;
}

void getCellLeft (tCoordinate* c, int worldWidth, tCoordinate* destCell){
    destCell->row = c->row;
    destCell->col = c->col >0 ? c->col-1 : worldWidth-1;
	
}

void getCellRight (tCoordinate* c, int worldWidth, tCoordinate* destCell){
    destCell->row = c->row;
    destCell->col = c->col < worldWidth-1 ? c->col+1 : 0;
}


unsigned short int getCellAtWorld (tCoordinate* c, 
			   unsigned short* world,
			   int worldWidth){
				   
	return (world[(c->row*worldWidth)+c->col]);	
}

void setCellAt (tCoordinate* c, 
			   unsigned short* world,
			   int worldWidth,
			   unsigned short int type){
	
	world[(c->row*worldWidth)+c->col] = type;	
}

void initRandomWorld (unsigned short *w, 
					int worldWidth, 
					int worldHeight){

	tCoordinate cell;
	
		for (int col=0; col<worldWidth; col++)
			for (int row=0; row<worldHeight; row++)
				if ((rand()%100)<INITIAL_CELLS_PERCENTAGE){					
					cell.row = row;
					cell.col = col;	
					setCellAt (&cell, w, worldWidth, CELL_LIVE);	
				}
}

void clearWorld (unsigned short *w, 
				int worldWidth, 
				int worldHeight){
	
	tCoordinate cell;

		for (int col=0; col<worldWidth; col++)
			for (int row=0; row<worldHeight; row++){
				cell.row = row;
				cell.col = col;
				setCellAt (&cell, w, worldWidth, CELL_EMPTY);				
			}
}

void calculateLonelyCell (){
	
	int value, total=0;
	tMatrix matrixA = (tMatrix) malloc (MATRIX_SIZE*MATRIX_SIZE*sizeof(int));
	tMatrix matrixB = (tMatrix) malloc (MATRIX_SIZE*MATRIX_SIZE*sizeof(int));
	tMatrix matrixC = (tMatrix) malloc (MATRIX_SIZE*MATRIX_SIZE*sizeof(int));	
	
		// Random matrix A
		for (int i=0; i<(MATRIX_SIZE*MATRIX_SIZE); i++)
			matrixA[i] = (rand() % 1000);
		
		// Random matrix B
		for (int i=0; i<(MATRIX_SIZE*MATRIX_SIZE); i++)
			matrixB[i] = (rand() % 1000);
	
		for (int i=0; i<MATRIX_SIZE; i++)
			for (int j=0; j<MATRIX_SIZE; j++){
				matrixC[(i*MATRIX_SIZE)+j]=0;
				for (int k=0; k<MATRIX_SIZE; k++)
					matrixC[(i*MATRIX_SIZE)+j] += matrixA[(i*MATRIX_SIZE)+k]*matrixB[(k*MATRIX_SIZE)+j];
			}
	
	free (matrixA);
	free (matrixB);
	free (matrixC);
}





