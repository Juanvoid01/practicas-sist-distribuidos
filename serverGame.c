#include "serverGame.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// Maximum message length
#define MAX_MSG_LENGTH 256

// Maximum number of connections
#define MAX_CONNECTIONS 5

#define LAST_GAMES_FILE_NAME "game_results.txt"

#define MAX_RESULTS 3


struct GameResult
{
	tString winner;
	tString loser;
};

struct GameResult results[MAX_RESULTS];
int resultCount = 0;

pthread_mutex_t resultsMutex = PTHREAD_MUTEX_INITIALIZER;

struct ThreadArgs
{
	int socketPlayer1;
	int socketPlayer2;
};

int acceptConnection(int socketServer);
void *game(void *threadArgs);
int createBindListenSocket(unsigned short port);
void sendMessageToPlayer(int socketClient, char *message);
void receiveMessageFromPlayer(int socketClient, char *message);
void sendCodeToClient(int socketClient, unsigned int code);
void sendBoardToClient(int socketClient, tBoard board);
unsigned int receiveMoveFromPlayer(int socketClient);
int getSocketPlayer(tPlayer player, int player1socket, int player2socket);
tPlayer switchPlayer(tPlayer currentPlayer);
void saveResultsToFile(tString winner, tString loser);
void readResultsFromFile();

int main(int argc, char *argv[])
{

	// Check arguments
	if (argc != 2)
	{
		fprintf(stderr, "ERROR wrong number of arguments\n");
		fprintf(stderr, "Usage:\n$>%s port\n", argv[0]);
		exit(1);
	}

	// Init seed
	srand(time(NULL));

	// Create the socket
	int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check
	if (socketfd < 0)
	{
		fprintf(stderr, "ERROR socket\n");
		exit(1);
	}

	// Get listening port
	int port = atoi(argv[1]);
	printf("port:%d\n", port);

	socketfd = createBindListenSocket(port);

	while (1)
	{

		// Establish connection with a client
		int player1socket = acceptConnection(socketfd);
		int player2socket = acceptConnection(socketfd);

		struct ThreadArgs *threadArgs;
		pthread_t threadID;

		// Allocate memory
		if ((threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs))) == NULL)
			showError("Error while allocating memory");

		// Set socket to the thread's parameter structure
		threadArgs->socketPlayer1 = player1socket;
		threadArgs->socketPlayer2 = player2socket;
		// Create a new thread
		if (pthread_create(&threadID, NULL, game, (void *)threadArgs) != 0)
			showError("pthread_create() failed");
	}

	close(socketfd);
}

void sendMessageToPlayer(int socketClient, char *message)
{
	int l = strlen(message);

	int bytes_sent = send(socketClient, &l, sizeof(l), 0);

	if (bytes_sent == -1)
	{
		perror("Error sending message length\n");
		return;
	}
	bytes_sent = send(socketClient, message, l, 0);

	if (bytes_sent == -1)
	{
		perror("Error sending message content\n");
		return;
	}
}

void receiveMessageFromPlayer(int socketClient, char *message)
{

	int l;
	int bytes_received = recv(socketClient, &l, sizeof(l), 0);

	if (bytes_received == -1 || bytes_received != sizeof(l))
	{
		perror("Error receiving message length\n");
		return;
	}

	bytes_received = recv(socketClient, message, l, 0);

	if (bytes_received == -1 || bytes_received != l)
	{
		perror("Error receiving message\n");
		return;
	}
}

void sendCodeToClient(int socketClient, unsigned int code)
{
	int l = sizeof(code);
	send(socketClient, &l, sizeof(l), 0);
	send(socketClient, &code, l, 0);
}

void sendBoardToClient(int socketClient, tBoard board)
{
	int l = BOARD_SIZE;
	send(socketClient, &l, sizeof(l), 0);
	send(socketClient, board, l, 0);
}

unsigned int receiveMoveFromPlayer(int socketClient)
{
	unsigned int move;
	int l;
	recv(socketClient, &l, sizeof(l), 0);
	recv(socketClient, &move, l, 0);

	return move;
}

int getSocketPlayer(tPlayer player, int player1socket, int player2socket)
{

	int socket;

	if (player == player1)
		socket = player1socket;
	else
		socket = player2socket;

	return socket;
}

tPlayer switchPlayer(tPlayer currentPlayer)
{

	tPlayer nextPlayer;

	if (currentPlayer == player1)
		nextPlayer = player2;
	else
		nextPlayer = player1;

	return nextPlayer;
}

void saveResultsToFile(tString winner, tString loser) {
    pthread_mutex_lock(&resultsMutex);

    // Añadir el nuevo resultado al array circular
    if (resultCount < MAX_RESULTS) {
        resultCount++;
    }
    for (int i = resultCount - 1; i > 0; i--) {
        results[i] = results[i - 1];
    }
    strncpy(results[0].winner, winner, STRING_LENGTH);
    strncpy(results[0].loser, loser, STRING_LENGTH);

    // Guardar en archivo
    FILE *file = fopen(LAST_GAMES_FILE_NAME, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        pthread_mutex_unlock(&resultsMutex);
        return;
    }
    
    for (int i = 0; i < resultCount; i++) {
        fprintf(file, "Game %d: Winner: %s, Loser: %s\n", i + 1, results[i].winner, results[i].loser);
    }

    fclose(file);
    pthread_mutex_unlock(&resultsMutex);
}

// Lee y muestra los últimos 3 resultados desde el archivo
void readResultsFromFile() {
    pthread_mutex_lock(&resultsMutex);

    FILE *file = fopen(LAST_GAMES_FILE_NAME, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        pthread_mutex_unlock(&resultsMutex);
        return;
    }

    char line[MAX_MSG_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
    pthread_mutex_unlock(&resultsMutex);
}

void *game(void *threadArgs)
{

	int socketPlayer1 = ((struct ThreadArgs *)threadArgs)->socketPlayer1;
	int socketPlayer2 = ((struct ThreadArgs *)threadArgs)->socketPlayer2;

	tString player1Name; /** Name of player 1 */
	tString player2Name; /** Name of player 2 */

	memset(player1Name, 0, STRING_LENGTH);

	receiveMessageFromPlayer(socketPlayer1, player1Name);

	memset(player2Name, 0, STRING_LENGTH);

	receiveMessageFromPlayer(socketPlayer2, player2Name);

	// Cross player names
	sendMessageToPlayer(socketPlayer1, player2Name);
	sendMessageToPlayer(socketPlayer2, player1Name);

	tBoard board;		   /** Board of the game */
	tPlayer currentPlayer; /** Current player */
	tMove moveResult;	   /** Result of player's move */

	int endOfGame;		 /** Flag to control the end of the game*/
	unsigned int column; /** Selected column to insert the chip */

	/** Message sent to the players */
	tString messageWaitTurnP1;
	memset(messageWaitTurnP1, 0, STRING_LENGTH);
	sprintf(messageWaitTurnP1, "Your rival is thinking... please, wait! You play with: %c", PLAYER_1_CHIP);
	tString messageWaitTurnP2;
	memset(messageWaitTurnP2, 0, STRING_LENGTH);
	sprintf(messageWaitTurnP2, "Your rival is thinking... please, wait! You play with: %c", PLAYER_2_CHIP);
	tString messagePlayTurnP1;
	memset(messagePlayTurnP1, 0, STRING_LENGTH);
	sprintf(messagePlayTurnP1, "It's your turn. You play with: %c", PLAYER_1_CHIP);
	tString messagePlayTurnP2;
	memset(messagePlayTurnP2, 0, STRING_LENGTH);
	sprintf(messagePlayTurnP2, "It's your turn. You play with: %c", PLAYER_2_CHIP);

	// Init game
	initBoard(board);
	endOfGame = FALSE;
	currentPlayer = (tPlayer)(rand() % 2);

	while (!endOfGame)
	{
		if (currentPlayer == player1)
		{
			sendCodeToClient(socketPlayer1, TURN_MOVE);
			sendCodeToClient(socketPlayer2, TURN_WAIT);

			sendMessageToPlayer(socketPlayer1, messagePlayTurnP1);
			sendBoardToClient(socketPlayer1, board);
			sendMessageToPlayer(socketPlayer2, messageWaitTurnP2);
			sendBoardToClient(socketPlayer2, board);

			int moveOkey = 0;

			while (moveOkey == 0)
			{
				column = receiveMoveFromPlayer(socketPlayer1);
				moveResult = checkMove(board, column);
				sendCodeToClient(socketPlayer1, moveResult);
				if (moveResult == fullColumn_move)
				{
					sendMessageToPlayer(socketPlayer1, "Column is full, play other move");
				}
				else
				{
					moveOkey = 1;
				}
			}
		}
		else
		{
			sendCodeToClient(socketPlayer2, TURN_MOVE);
			sendCodeToClient(socketPlayer1, TURN_WAIT);
			sendMessageToPlayer(socketPlayer2, messagePlayTurnP2);
			sendBoardToClient(socketPlayer2, board);
			sendMessageToPlayer(socketPlayer1, messageWaitTurnP1);
			sendBoardToClient(socketPlayer1, board);

			int moveOkey = 0;

			while (moveOkey == 0)
			{
				column = receiveMoveFromPlayer(socketPlayer2);
				moveResult = checkMove(board, column);
				sendCodeToClient(socketPlayer2, moveResult);
				if (moveResult == fullColumn_move)
				{
					sendMessageToPlayer(socketPlayer2, "Column is full, play other move");
				}
				else
				{
					moveOkey = 1;
				}
			}
		}

		insertChip(board, currentPlayer, column);

		if (checkWinner(board, player1))
		{
			sendCodeToClient(socketPlayer1, GAMEOVER_WIN);
			sendCodeToClient(socketPlayer2, GAMEOVER_LOSE);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
			endOfGame = TRUE;
			saveResultsToFile(player1Name, player2Name);
			readResultsFromFile();
		}
		if (checkWinner(board, player2))
		{
			sendCodeToClient(socketPlayer1, GAMEOVER_LOSE);
			sendCodeToClient(socketPlayer2, GAMEOVER_WIN);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
			saveResultsToFile(player2Name, player1Name);
			readResultsFromFile();
			endOfGame = TRUE;
		}
		else if (isBoardFull(board))
		{
			sendCodeToClient(socketPlayer1, GAMEOVER_DRAW);
			sendCodeToClient(socketPlayer2, GAMEOVER_DRAW);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
			endOfGame = TRUE;
		}
		else
		{
			currentPlayer = switchPlayer(currentPlayer);
		}
	}

	return 0;
}

int acceptConnection(int socketServer)
{

	int clientSocket;
	struct sockaddr_in clientAddress;
	unsigned int clientAddressLength;

	// Get length of client address
	clientAddressLength = sizeof(clientAddress);

	// Accept
	if ((clientSocket = accept(socketServer, (struct sockaddr *)&clientAddress, &clientAddressLength)) < 0)
		showError("Error while accepting connection");

	printf("Connection established with client: %s\n", inet_ntoa(clientAddress.sin_addr));

	return clientSocket;
}

int createBindListenSocket(unsigned short port)
{

	int socketId;
	struct sockaddr_in echoServAddr;

	if ((socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		showError("Error while creating a socket");

	// Set server address
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServAddr.sin_port = htons(port);

	// Bind
	if (bind(socketId, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
		showError("Error while binding");

	if (listen(socketId, MAX_CONNECTIONS) < 0)
		showError("Error while listening");

	return socketId;
}
