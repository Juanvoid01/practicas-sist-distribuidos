#include "serverGame.h"

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
	printf("receiving message from server...\n");
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

int main(int argc, char *argv[])
{

	int socketfd;							  /** Socket descriptor */
	struct sockaddr_in serverAddress;		  /** Server address structure */
	unsigned int port;						  /** Listening port */
	struct sockaddr_in player1Address;		  /** Client address structure for player 1 */
	struct sockaddr_in player2Address;		  /** Client address structure for player 2 */
	int socketPlayer1 = 0, socketPlayer2 = 0; /** Socket descriptor for each player */
	unsigned int clientLength;				  /** Length of client structure */

	tBoard board;		   /** Board of the game */
	tPlayer currentPlayer; /** Current player */
	tMove moveResult;	   /** Result of player's move */
	tString player1Name;   /** Name of player 1 */
	tString player2Name;   /** Name of player 2 */
	int endOfGame;		   /** Flag to control the end of the game*/
	unsigned int column;   /** Selected column to insert the chip */
	tString message;	   /** Message sent to the players */

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
	socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check
	if (socketfd < 0)
	{
		fprintf(stderr, "ERROR socket\n");
		exit(1);
	}

	// Init server structure
	memset(&serverAddress, 0, sizeof(serverAddress));

	// Get listening port
	port = atoi(argv[1]);
	printf("port:%d\n", port);
	// Fill server structure
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);

	// Bind
	if (bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		fprintf(stderr, "ERROR bind\n");
		exit(1);
	}

	// Listen
	if (listen(socketfd, 10) != 0)
	{
		fprintf(stderr, "ERROR listen\n");
		exit(1);
	}

	clientLength = sizeof(player1Address);

	socketPlayer1 = accept(socketfd, (struct sockaddr *)&player1Address, &clientLength);

	if(socketPlayer1 == -1)
	{
		fprintf(stderr, "ERROR accept player1\n");
		exit(1);
	}

	// Clear buffer
	memset(player1Name, 0, STRING_LENGTH);

	receiveMessageFromPlayer(socketPlayer1, player1Name);

	printf("Nombre jugador1 recibido: %s\n", player1Name);

	socketPlayer2 = accept(socketfd, (struct sockaddr *)&player2Address, &clientLength);

	if(socketPlayer2 == -1)
	{
		fprintf(stderr, "ERROR accept player2\n");
		exit(1);
	}

	// Clear buffer
	memset(player2Name, 0, STRING_LENGTH);

	receiveMessageFromPlayer(socketPlayer2, player2Name);
	printf("Nombre jugador2 recibido: %s\n", player2Name);

	sendMessageToPlayer(socketPlayer1, player2Name);

	sendMessageToPlayer(socketPlayer2, player1Name);


	initBoard(board);
	endOfGame = FALSE;
	currentPlayer = (tPlayer)(rand() % 2);

	while (!endOfGame)
	{
		if (currentPlayer == player1)
		{
			sendCodeToClient(socketPlayer1, TURN_MOVE);
			sendCodeToClient(socketPlayer2, TURN_WAIT);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);

			column = receiveMoveFromPlayer(socketPlayer1);
			printf("columna Jugador 1 = %d",column);
		}
		else
		{
			sendCodeToClient(socketPlayer2, TURN_MOVE);
			sendCodeToClient(socketPlayer1, TURN_WAIT);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);

			column = receiveMoveFromPlayer(socketPlayer2);
			printf("columna Jugador 2 = %d",column);
		}

		insertChip(board, currentPlayer, column);

		if (checkWinner(board, player1))
		{
			sendCodeToClient(socketPlayer1, GAMEOVER_WIN);
			sendCodeToClient(socketPlayer2, GAMEOVER_LOSE);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
			endOfGame = TRUE;
		}
		if (checkWinner(board, player2))
		{
			sendCodeToClient(socketPlayer1, GAMEOVER_LOSE);
			sendCodeToClient(socketPlayer2, GAMEOVER_WIN);
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
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
			sendBoardToClient(socketPlayer1, board);
			sendBoardToClient(socketPlayer2, board);
		}
	}

	close(socketfd);
}
