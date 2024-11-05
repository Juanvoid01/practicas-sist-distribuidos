#include "server.h"

/** Flag to enable debugging */
#define DEBUG_SERVER 1

/** Array with games */
tGame games[MAX_GAMES];

/** Mutex to protect the game status field in the array of games */
pthread_mutex_t mutexStatusArray;

void initServerStructures()
{

	if (DEBUG_SERVER)
		printf("Initializing...\n");

	// Init seed
	srand(time(NULL));

	// Init each game
	for (int i = 0; i < MAX_GAMES; i++)
	{

		// Allocate and init board
		games[i].board = (xsd__string)malloc(BOARD_WIDTH * BOARD_HEIGHT);
		initBoard(games[i].board);

		// Calculate the first player to play
		if ((rand() % 2) == 0)
			games[i].currentPlayer = player1;
		else
			games[i].currentPlayer = player2;

		// Allocate and init player names
		games[i].player1Name = (xsd__string)malloc(STRING_LENGTH);
		games[i].player2Name = (xsd__string)malloc(STRING_LENGTH);
		memset(games[i].player1Name, 0, STRING_LENGTH);
		memset(games[i].player2Name, 0, STRING_LENGTH);

		// Game status
		games[i].endOfGame = FALSE;
		games[i].status = gameEmpty;

		// Init mutex and cond variable
	}
}

conecta4ns__tPlayer switchPlayer(conecta4ns__tPlayer currentPlayer)
{
	return (currentPlayer == player1) ? player2 : player1;
}

int searchEmptyGame()
{
	for (int i = 0; i < MAX_GAMES; i++)
	{
		if (games[i].status == gameEmpty ||
			games[i].status == gameWaitingPlayer)
		{
			return i;
		}
	}

	return ERROR_SERVER_FULL;
}

int checkPlayer(xsd__string playerName, int gameId)
{
	return games[gameId].player1Name == playerName ||
		   games[gameId].player2Name == playerName;
}

void freeGameByIndex(int index)
{
	initBoard(games[index].board);
	games[index].endOfGame = TRUE;
	games[index].status = gameEmpty;
	memset(games[index].player1Name, 0, STRING_LENGTH);
	memset(games[index].player2Name, 0, STRING_LENGTH);
}

void copyGameStatusStructure(conecta4ns__tBlock *status, char *message, xsd__string board, int newCode)
{

	// Set the new code
	status->code = newCode;

	// Copy the message
	memset((status->msgStruct).msg, 0, STRING_LENGTH);
	strcpy((status->msgStruct).msg, message);
	(status->msgStruct).__size = strlen((status->msgStruct).msg);

	// Copy the board, only if it is not NULL
	if (board == NULL)
	{
		status->board = NULL;
		status->__size = 0;
	}
	else
	{
		strncpy(status->board, board, BOARD_WIDTH * BOARD_HEIGHT);
		status->__size = BOARD_WIDTH * BOARD_HEIGHT;
	}
}

int conecta4ns__register(struct soap *soap, conecta4ns__tMessage playerName, int *code)
{

	int gameIndex = -1;
	int result = 0;

	// Set \0 at the end of the string
	playerName.msg[playerName.__size] = 0;

	if (DEBUG_SERVER)
	{
		printf("[Register] Registering new player -> [%s]\n", playerName.msg);
	}
	int game_index = searchEmptyGame();

	if (game_index == ERROR_SERVER_FULL)
	{
		result = ERROR_SERVER_FULL;
		printf("error register game SERVER FULL\n");
		return SOAP_OK;
	}

	if (games[game_index].status == gameWaitingPlayer)
	{
		if (strcmp(games[game_index].player1Name, playerName.msg))
		{
			result = ERROR_PLAYER_REPEATED;
			return SOAP_OK;
		}
		freeGameByIndex(game_index);

		strcpy(games[game_index].player2Name, playerName.msg);
		games[game_index].status = gameReady;
	}
	else if (games[game_index].status == gameEmpty)
	{
		freeGameByIndex(game_index);
		strcpy(games[game_index].player1Name, playerName.msg);
		games[game_index].status = gameWaitingPlayer;
	}

	result = game_index;

	*code = result;

	return SOAP_OK;
}

int conecta4ns__getStatus(struct soap *soap, conecta4ns__tMessage playerName, int gameId, conecta4ns__tBlock *status)
{

	if(games[gameId].status == gameWaitingPlayer)
	{
		status->code = 69;
		return SOAP_OK;
	}
	else if(games[gameId].status == gameEmpty)
	{
		status->code = 69;
		return SOAP_OK;
	}

	if (!strcmp(playerName.msg, games[gameId].player1Name) ||
		!strcmp(playerName.msg, games[gameId].player2Name))
	{
		printf("Player not found\n");
		status->code = ERROR_PLAYER_NOT_FOUND;
		return SOAP_OK;
	}

	char messageToPlayer[STRING_LENGTH];

	// Set \0 at the end of the string and alloc memory for the status
	playerName.msg[playerName.__size] = 0;
	allocClearBlock(soap, status);

	if (DEBUG_SERVER)
	{
		printf("Receiving getStatus() request from -> %s [%d] in game %d\n", playerName.msg, playerName.__size, gameId);
	}

	conecta4ns__tPlayer player = strcmp(playerName.msg, games[gameId].player1Name) ? player1 : player2;

	if (games[gameId].endOfGame == TRUE)
	{
		if (checkWinner(games[gameId].board, player))
		{
			status->code = GAMEOVER_WIN;
		}
		else if (isBoardFull(games[gameId].board))
		{
			status->code = GAMEOVER_DRAW;
		}
		else
		{
			status->code = GAMEOVER_LOSE;
		}
	}

	status->msgStruct;
	status->__size;
	strcpy(status->board, games[gameId].board);

	return SOAP_OK;
}

int conecta4ns__insertChip(struct soap *soap, int id_game, conecta4ns__tMessage playerName, int column, int *status)
{
	if (!strcmp(playerName.msg, games[id_game].player1Name) ||
		!strcmp(playerName.msg, games[id_game].player2Name))
	{
		printf("Player not found\n");
		*status = ERROR_PLAYER_NOT_FOUND;
		return SOAP_OK;
	}

	if (column < 0 || column > 6 || checkMove(games[id_game].board, column) != OK_move)
	{
		printf("Column not valid\n");
		return SOAP_OK;
	}

	conecta4ns__tPlayer player = strcmp(playerName.msg, games[id_game].player1Name) ? player1 : player2;

	insertChip(games[id_game].board, player, column);

	if (checkWinner(games[id_game].board, player))
	{
		*status = GAMEOVER_WIN;
		games[id_game].endOfGame = TRUE;
	}
	else if (isBoardFull(games[id_game].board))
	{
		*status = GAMEOVER_DRAW;
	}

	return SOAP_OK;
}

void *processRequest(void *soap)
{

	pthread_detach(pthread_self());

	if (DEBUG_SERVER)
		printf("Processing a new request...");

	soap_serve((struct soap *)soap);
	soap_destroy((struct soap *)soap);
	soap_end((struct soap *)soap);
	soap_done((struct soap *)soap);
	free(soap);

	return NULL;
}

int main(int argc, char **argv)
{

	struct soap soap;
	struct soap *tsoap;
	pthread_t tid;
	int port;
	SOAP_SOCKET m, s;

	// Init soap environment
	soap_init(&soap);

	// Configure timeouts
	soap.send_timeout = 60;		// 60 seconds
	soap.recv_timeout = 60;		// 60 seconds
	soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
	soap.max_keep_alive = 100;	// max keep-alive sequence

	initServerStructures();
	pthread_mutex_init(&mutexStatusArray, NULL);

	// Get listening port
	port = atoi(argv[1]);

	// Bind
	m = soap_bind(&soap, NULL, port, 100);

	if (!soap_valid_socket(m))
	{
		exit(1);
	}

	printf("Server is ON ...\n");

	while (TRUE)
	{

		// Accept a new connection
		s = soap_accept(&soap);

		// Socket is not valid :(
		if (!soap_valid_socket(s))
		{

			if (soap.errnum)
			{
				soap_print_fault(&soap, stderr);
				exit(1);
			}

			fprintf(stderr, "Time out!\n");
			break;
		}

		// Copy the SOAP environment
		tsoap = soap_copy(&soap);

		if (!tsoap)
		{
			printf("SOAP copy error!\n");
			break;
		}

		// Create a new thread to process the request
		pthread_create(&tid, NULL, (void *(*)(void *))processRequest, (void *)tsoap);
	}

	// Detach SOAP environment
	soap_done(&soap);
	return 0;
}
