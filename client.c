#include "client.h"

#define DEBUG_CLIENT 1

unsigned int readMove()
{

	xsd__string enteredMove;
	unsigned int move;
	unsigned int isRightMove;

	// Init...
	enteredMove = (xsd__string)malloc(STRING_LENGTH);
	memset(enteredMove, 0, STRING_LENGTH);
	isRightMove = FALSE;
	move = STRING_LENGTH;

	while (!isRightMove)
	{

		printf("Enter a move [0-%d]:", BOARD_WIDTH - 1);

		// Read move
		fgets(enteredMove, STRING_LENGTH - 1, stdin);

		// Remove new-line char
		enteredMove[strlen(enteredMove) - 1] = 0;

		// Length of entered move is not correct
		if (strlen(enteredMove) != 1)
		{
			printf("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH - 1);
		}

		// Check if entered move is a number
		else if (isdigit(enteredMove[0]))
		{

			// Convert move to an int
			move = enteredMove[0] - '0';

			if (move >= BOARD_WIDTH)
				printf("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH - 1);
			else
				isRightMove = TRUE;
		}

		// Entered move is not a number
		else
			printf("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH - 1);
	}

	return move;
}

int main(int argc, char **argv)
{

	struct soap soap;				 /** Soap struct */
	char *serverURL;				 /** Server URL */
	unsigned int endOfGame;			 /** Flag to control the end of the game */
	conecta4ns__tMessage playerName; /** Player name */
	conecta4ns__tBlock gameStatus;	 /** Game status */
	unsigned int playerMove;		 /** Player move */
	int resCode;					 /** Return code from server */

	// Init gSOAP environment
	soap_init(&soap);

	// Obtain server address
	serverURL = argv[1];

	// Allocate memory for player name and init
	playerName.msg = (xsd__string)malloc(STRING_LENGTH);
	memset(playerName.msg, 0, STRING_LENGTH);

	// Allocate memory for game status and init
	allocClearBlock(&soap, &gameStatus);

	// Init
	resCode = -1;
	endOfGame = FALSE;
	gameStatus.code = 0;

	// Check arguments
	if (argc != 2)
	{
		printf("Usage: %s http://server:port\n", argv[0]);
		exit(0);
	}

	// Init player's name
	do
	{
		printf("Enter player name:");
		fgets(playerName.msg, STRING_LENGTH - 1, stdin);

		playerName.__size = strlen(playerName.msg);

		// Remove '\n'
		playerName.msg[playerName.__size - 1] = 0;

	} while (playerName.__size <= 2);

	soap_call_conecta4ns__register(&soap, serverURL, "", playerName, &resCode);

	// Clean the environment
	soap_destroy(&soap);
	soap_end(&soap);
	soap_done(&soap);

	return 0;
}

/*
#include "client.h"

#define DEBUG_CLIENT 1


unsigned int readMove (){

	xsd__string enteredMove;
	unsigned int move;
	unsigned int isRightMove;

		// Init...
		enteredMove = (xsd__string) malloc (STRING_LENGTH);
		memset (enteredMove, 0, STRING_LENGTH);
		isRightMove = FALSE;
		move = STRING_LENGTH;

		while (!isRightMove){

			printf ("Enter a move [0-%d]:", BOARD_WIDTH-1);

			// Read move
			fgets (enteredMove, STRING_LENGTH-1, stdin);

			// Remove new-line char
			enteredMove[strlen(enteredMove)-1] = 0;

			// Length of entered move is not correct
			if (strlen(enteredMove) != 1){
				printf ("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH-1);
			}

			// Check if entered move is a number
			else if (isdigit(enteredMove[0])){

				// Convert move to an int
				move =  enteredMove[0] - '0';

				if (move >= BOARD_WIDTH)
					printf ("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH-1);
				else
					isRightMove = TRUE;
			}

			// Entered move is not a number
			else
				printf ("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH-1);
		}

	return move;
}


int main(int argc, char **argv){



		// Init gSOAP environment
		soap_init(&soap);

		// Obtain server address
		serverURL = argv[1];

		// Allocate memory for player name and init
		playerName.msg = (xsd__string) malloc (STRING_LENGTH);
		memset(playerName.msg, 0, STRING_LENGTH);

		// Allocate memory for game status and init
        allocClearBlock (&soap, &gameStatus);
  
		// Init
		resCode = -1;
		endOfGame = FALSE;
		gameStatus.code = 0;

		// Check arguments
		if (argc !=2) {
			printf("Usage: %s http://server:port\n",argv[0]);
			exit(0);
		}


				memset(playerName.msg, 0, STRING_LENGTH);
				printf ("Enter player name:");
				fgets(playerName.msg, STRING_LENGTH-1, stdin);

				// Remove '\n'
				playerName.msg[strlen(playerName.msg)-1] = 0;



			// Try to register the player
			    // Llama a soap_call_conecta4ns__register para registrar el jugador
    if (soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, "", &gameStatus)){
        if (resCode < 0) {
            printf("Error registering player: %d\n", resCode);
        } else {
            printf("Player registered successfully with game ID: %d\n", resCode);
        }
    } else {
		       soap_print_fault(&soap, stderr);
    

			// Check for errors...
			if (soap.error) {

				soap_print_fault(&soap, stderr);
				exit(1);
			
			}
	}

 
		
	
		// While game continues...
		while (!endOfGame){
			if(gameStatus.code != GAMEOVER_WIN || gameStatus.code != GAMEOVER_DRAW){ //this if prevents the player who won to get in the getStatus function without freeing the game

				soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, "", &gameStatus);

				
				if(gameStatus.code != 1){//while the code is not gameWaitingPlayer
					
					if(gameStatus.code == GAMEOVER_LOSE || gameStatus.code == GAMEOVER_DRAW){

						endOfGame = TRUE;

					}else{
						
						printBoard(gameStatus.board, gameStatus.msgStruct.msg);
						
						if(gameStatus.code == TURN_MOVE){

							playerMove = readMove();
							//soap_call_conecta4ns__insertChip(&soap, serverURL, "", playerName, playerMove, &gameStatus);

						}
					}	
				}
			}else{ 
				endOfGame = TRUE;
			}		
		}
		//print the final board
		printBoard(gameStatus.board, gameStatus.msgStruct.msg);

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

		// Clean the environment
		soap_destroy(&soap);
		soap_end(&soap);
		soap_done(&soap);

  return 0;
}
*/