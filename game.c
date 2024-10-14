#include "game.h"


void initBoard (xsd__string board){
	
	int empty = (int) EMPTY_CELL;

	for (int i=0; i<(BOARD_WIDTH * BOARD_HEIGHT); i++)
		board[i] = (char) empty;
}


conecta4ns__tMove checkMove (xsd__string board, unsigned int column){

    conecta4ns__tMove move = OK_move;
		
		if (board[(BOARD_WIDTH*(BOARD_HEIGHT-1)) + column] != EMPTY_CELL)
			move = fullColumn_move;

	return move;
}


void insertChip (xsd__string board, conecta4ns__tPlayer player, unsigned int column){

    char c = PLAYER_1_CHIP;
    unsigned int cell = column;
    unsigned int found = FALSE;

		// Set chip type
		if (player == player2)
			c = PLAYER_2_CHIP;

		// Get the right row
		while (!found){

			if (board[cell] == EMPTY_CELL)
				found = TRUE;
			else
				cell += BOARD_WIDTH;

			// re-Check!
			if (cell >= (BOARD_WIDTH * BOARD_HEIGHT))
				showError ("[insertChip] Checking out of bounds!!! :(\n");
		}

		// Insert the chip!
		board[cell] = c;
}


int checkUp (xsd__string board, unsigned int cell, char c){

    int found = FALSE;
		
		// Check second chip
		if ((cell+BOARD_WIDTH) < (BOARD_WIDTH * BOARD_HEIGHT))
			found = (board[cell+BOARD_WIDTH] == c);

		// Check third chip
		if (((cell+(2*BOARD_WIDTH)) < (BOARD_WIDTH * BOARD_HEIGHT)) && found)
			found = (board[cell+(2*BOARD_WIDTH)] == c);

		// Check fourth chip
		if (((cell+(3*BOARD_WIDTH)) < (BOARD_WIDTH * BOARD_HEIGHT)) && found )
			found = (board[cell+(3*BOARD_WIDTH)] == c);

	return found;
}


int checkRight (xsd__string board, unsigned int cell, char c){

    int found = FALSE;
    int limit = ((cell/BOARD_WIDTH) +1) * BOARD_WIDTH;
		
		// Check second chip
		if ((cell+1) < limit)
			found = (board[cell+1] == c);

		// Check third chip
		if (((cell+2) < limit) && found)
			found = (board[cell+2] == c);

		// Check fourth chip
		if (((cell+3) < limit) && found )
			found = (board[cell+3] == c);

	return found;
}


int checkUpLeft (xsd__string board, unsigned int cell, char c){

    int found = FALSE;
		
		if (((cell%BOARD_WIDTH) >= 3) && (cell < (BOARD_WIDTH*3))){

			// Check second chip
			if ((cell+BOARD_WIDTH-1) < (BOARD_WIDTH * BOARD_HEIGHT))
				found = (board[cell+BOARD_WIDTH-1] == c);

			// Check third chip
			if (((cell+(2*BOARD_WIDTH)-2) < (BOARD_WIDTH * BOARD_HEIGHT)) && found)
				found = (board[cell+(2*BOARD_WIDTH)-2] == c);

			// Check fourth chip
			if (((cell+(3*BOARD_WIDTH)-3) < (BOARD_WIDTH * BOARD_HEIGHT)) && found )
				found = (board[cell+(3*BOARD_WIDTH)-3] == c);
		}

	return found;
}


int checkUpRight (xsd__string board, unsigned int cell, char c){

    int found = FALSE;
		
		if (((cell%BOARD_WIDTH) <= 3) && (cell < (BOARD_WIDTH*3))){

			// Check second chip
			if ((cell+BOARD_WIDTH+1) < (BOARD_WIDTH * BOARD_HEIGHT))
				found = (board[cell+BOARD_WIDTH+1] == c);

			// Check third chip
			if (((cell+(2*BOARD_WIDTH)+2) < (BOARD_WIDTH * BOARD_HEIGHT)) && found)
				found = (board[cell+(2*BOARD_WIDTH)+2] == c);

			// Check fourth chip
			if (((cell+(3*BOARD_WIDTH)+3) < (BOARD_WIDTH * BOARD_HEIGHT)) && found )
				found = (board[cell+(3*BOARD_WIDTH)+3] == c);
		}

	return found;
}


int checkWinner (xsd__string board, conecta4ns__tPlayer player){

    char c = PLAYER_1_CHIP;
    unsigned int found = FALSE;
    unsigned int i = 0;
		
		// Set chip type
		if (player == player2)
			c = PLAYER_2_CHIP;

		// Check every cell in the board
		while ((!found) && (i<(BOARD_WIDTH * BOARD_HEIGHT))){

			// Is this cell occupied by current player?
			if (board[i] == c){

				// Check 4-line chips up
				found = checkUp (board, i, c);

				// Check 4-line chips right
				if (!found)
					found = checkRight (board, i, c);

				// Check 4-line chips upLeft
				if (!found)
					found = checkUpLeft (board, i, c);

				// Check 4-line chips upRight
				if (!found)
					found = checkUpRight (board, i, c);
			}

			// Check next cell
			if (!found)
				i++;
		}

	return found;
}

int isBoardFull (xsd__string board){

    int i = BOARD_WIDTH*(BOARD_HEIGHT-1);
    int emptyFound = FALSE;
		
        // Check last row
		while ((!emptyFound) && (i<(BOARD_WIDTH*BOARD_HEIGHT))){

			if (board[i] == EMPTY_CELL)
				emptyFound = TRUE;
			else
				i++;
		}

	return (!emptyFound);
}

void showError(const char *msg){
    perror(msg);
    exit(0);
}


void printBoard (xsd__string board, xsd__string message){

    int cell;

        // Clear screen
        printf("\n%s\n", message);

        // Show column numbers
        printf ("  0   1   2   3   4   5   6  \n");

        // Draw each row
        for (int row = (BOARD_HEIGHT - 1); row>=0; row--){

            // Current cell in the board
            cell = row * BOARD_WIDTH;

            // Separator line
            printf ("|---|---|---|---|---|---|---|\n");

            // Print a row of cells
            for (int i=cell; i<(cell+BOARD_WIDTH); i++){
                printf("| %c ",board[i]);
            }

            // End of the row!
            printf ("|\n");
        }

        // Print the base
        printf ("|---------------------------|\n\n\n");
}

void allocClearMessage (struct soap *soap, conecta4ns__tMessage* msg){
    msg->msg = (xsd__string) soap_malloc (soap, STRING_LENGTH);
    memset(msg->msg, 0, STRING_LENGTH);
    msg->__size = STRING_LENGTH;
}

void allocClearBlock (struct soap *soap, conecta4ns__tBlock* block){
    block->code = -1;
    allocClearMessage (soap, &(block->msgStruct));
    block->__size = 0;
    block->board = (xsd__string) soap_malloc (soap, BOARD_WIDTH * BOARD_HEIGHT * sizeof (char));    
}
