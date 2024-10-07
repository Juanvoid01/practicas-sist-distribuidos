#include "clientGame.h"

void sendMessageToServer(int socketServer, char *message)
{

	int l = strlen(message);

	int bytes_sent = send(socketServer, &l, sizeof(l), 0);

	if (bytes_sent == -1 || bytes_sent != sizeof(l))
	{
		perror("Error sending message length\n");
		return;
	}
	bytes_sent = send(socketServer, message, l, 0);
	if (bytes_sent == -1 || bytes_sent != l)
	{
		perror("Error sending message content\n");
		return;
	}
}

void receiveMessageFromServer(int socketServer, char *message)
{

	int l;
	int bytes_received = recv(socketServer, &l, sizeof(l), 0);

	if (bytes_received == -1)
	{
		perror("Error receiving message length\n,");
		return;
	}

	bytes_received = recv(socketServer, message, l, 0);

	if (bytes_received == -1)
	{
		perror("Error receiving message\n");
		return;
	}
}

void receiveBoard(int socketServer, tBoard board)
{
	int l;
	recv(socketServer, &l, sizeof(l), 0);
	recv(socketServer, board, l, 0);
}

void sendMoveToServer(int socketServer, unsigned int move)
{
	int l = sizeof(move);
	send(socketServer, &l, sizeof(l), 0);
	send(socketServer, &move, l, 0);
}

unsigned int receiveCode(int socketServer)
{
	unsigned int code;
	int l;
	recv(socketServer, &l, sizeof(l), 0);
	recv(socketServer, &code, l, 0);

	return code;
}

unsigned int readMove()
{

	tString enteredMove;
	unsigned int move;
	unsigned int isRightMove;

	// Init...
	isRightMove = FALSE;
	move = STRING_LENGTH;

	while (!isRightMove)
	{

		printf("Enter a move [0-6]:");

		// Read move
		fgets(enteredMove, STRING_LENGTH - 1, stdin);

		// Remove new-line char
		enteredMove[strlen(enteredMove) - 1] = 0;

		// Length of entered move is not correct
		if (strlen(enteredMove) != 1)
		{
			printf("Entered move is not correct. It must be a number between [0-6]\n");
		}

		// Check if entered move is a number
		else if (isdigit(enteredMove[0]))
		{

			// Convert move to an int
			move = enteredMove[0] - '0';

			if (move > 6)
				printf("Entered move is not correct. It must be a number between [0-6]\n");
			else
				isRightMove = TRUE;
		}

		// Entered move is not a number
		else
			printf("Entered move is not correct. It must be a number between [0-6]\n");
	}

	return move;
}

int main(int argc, char *argv[])
{

	int socketfd;					   /** Socket descriptor */
	unsigned int port;				   /** Port number (server) */
	struct sockaddr_in server_address; /** Server address structure */
	char *serverIP;					   /** Server IP */
	//int msgLenght;

	tBoard board;			/** Board to be displayed */
	tString playerName;		/** Name of the player */
	tString rivalName;		/** Name of the rival */
	tString message;		/** Message received from server */
	//unsigned int column;	/** Selected column */
	unsigned int code;		/** Code sent/receive to/from server */
	unsigned int endOfGame; /** Flag to control the end of the game */

	// Check arguments!
	if (argc != 3)
	{
		fprintf(stderr, "ERROR wrong number of arguments\n");
		fprintf(stderr, "Usage:\n$>%s serverIP port\n", argv[0]);
		exit(0);
	}

	// Get the server address
	serverIP = argv[1];

	// Get the port
	port = atoi(argv[2]);

	// Create socket
	socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check if the socket has been successfully created
	if (socketfd < 0)
	{
		fprintf(stderr, "ERROR socket\n");
		exit(1);
	}

	// Fill server address structure
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(serverIP);
	server_address.sin_port = htons(port);

	// Connect with server
	if (connect(socketfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "ERROR connect\n");
		exit(1);
	}

	printf("Connection established with server!\n");

	// Init player's name
	do
	{
		memset(playerName, 0, STRING_LENGTH);
		printf("Enter player name:");
		fgets(playerName, STRING_LENGTH - 1, stdin);

		// Remove '\n'
		playerName[strlen(playerName) - 1] = 0;

	} while (strlen(playerName) <= 2);

	// Send player's name to the server
	sendMessageToServer(socketfd, playerName);

	// Clear buffer
	memset(rivalName, 0, STRING_LENGTH);

	// Receive rival's name
	receiveMessageFromServer(socketfd, rivalName);

	printf("You are playing against %s\n", rivalName);

	// Init
	endOfGame = FALSE;

	// Game starts
	printf("Game starts!\n\n");

	// While game continues...
	unsigned int move;

	while (!endOfGame)
	{
		code = receiveCode(socketfd);

		if (code == TURN_WAIT)
		{
			memset(message, 0, STRING_LENGTH);
			receiveMessageFromServer(socketfd, message);
			memset(board, 0, BOARD_SIZE);
			receiveBoard(socketfd, board);
			printBoard(board, message);
		}
		else if (code == TURN_MOVE)
		{
			memset(message, 0, STRING_LENGTH);
			receiveMessageFromServer(socketfd, message);
			memset(board, 0, BOARD_SIZE);
			receiveBoard(socketfd, board);
			printBoard(board, message);

			tMove moveResult = fullColumn_move;

			while (moveResult != OK_move)
			{
				move = readMove();
				sendMoveToServer(socketfd, move);
				moveResult = receiveCode(socketfd);

				if (moveResult == fullColumn_move)
				{
					memset(message, 0, STRING_LENGTH);
					receiveMessageFromServer(socketfd, message);
					printBoard(board, message);
				}
			}
		}
		else if (code == GAMEOVER_WIN)
		{
			memset(message, 0, STRING_LENGTH);
			receiveMessageFromServer(socketfd, message);
			memset(board, 0, BOARD_SIZE);
			receiveBoard(socketfd, board);
			printBoard(board, message);
			endOfGame = TRUE;
		}
		else if (code == GAMEOVER_LOSE)
		{
			memset(message, 0, STRING_LENGTH);
			receiveMessageFromServer(socketfd, message);
			memset(board, 0, BOARD_SIZE);
			receiveBoard(socketfd, board);
			printBoard(board, message);
			endOfGame = TRUE;
		}
		else if (code == GAMEOVER_DRAW)
		{
			memset(message, 0, STRING_LENGTH);
			receiveMessageFromServer(socketfd, message);
			memset(board, 0, BOARD_SIZE);
			receiveBoard(socketfd, board);
			printBoard(board, message);
			endOfGame = TRUE;
		}
	}

	// Close socket
	close(socketfd);
}
