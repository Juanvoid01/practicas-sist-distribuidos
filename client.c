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
        printf("Enter a move [0-%d]: ", BOARD_WIDTH - 1);

        // Read move
        fgets(enteredMove, STRING_LENGTH - 1, stdin);

        // Remove new-line char
        enteredMove[strlen(enteredMove) - 1] = 0;

        // Check if the input is a valid move
        if (strlen(enteredMove) != 1 || !isdigit(enteredMove[0]))
        {
            printf("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH - 1);
        }
        else
        {
            // Convert move to an int
            move = enteredMove[0] - '0';

            if (move >= BOARD_WIDTH)
                printf("Entered move is not correct. It must be a number in the interval [0-%d]\n", BOARD_WIDTH - 1);
            else
                isRightMove = TRUE;
        }
    }

    free(enteredMove);
    return move;
}

int main(int argc, char **argv)
{
    struct soap soap;                /** Soap struct */
    char *serverURL;                 /** Server URL */
    unsigned int endOfGame;          /** Flag to control the end of the game */
    conecta4ns__tMessage playerName; /** Player name */
    conecta4ns__tBlock gameStatus;   /** Game status */
    unsigned int playerMove;         /** Player move */
    int gameId;                      /** Game ID */
    int resCode;                     /** Return code from server */

    // Init gSOAP environment
    soap_init(&soap);

    // Check arguments
    if (argc != 2)
    {
        printf("Usage: %s http://server:port\n", argv[0]);
        exit(0);
    }

    // Obtain server address
    serverURL = argv[1];

    // Allocate memory for player name and game status
    playerName.msg = (xsd__string)malloc(STRING_LENGTH);
    memset(playerName.msg, 0, STRING_LENGTH);
    allocClearBlock(&soap, &gameStatus);

    // Initialize variables
    resCode = -1;
    endOfGame = FALSE;
    gameStatus.code = 0;

    // Initialize player's name
    do
    {
        printf("Enter player name: ");
        fgets(playerName.msg, STRING_LENGTH - 1, stdin);
        playerName.__size = strlen(playerName.msg);

        // Remove '\n' if it's there
        if (playerName.msg[playerName.__size - 1] == '\n')
        {
            playerName.msg[playerName.__size - 1] = 0;
            playerName.__size--;
        }
    } while (playerName.__size <= 2);

    // Register the player on the server
    if (soap_call_conecta4ns__register(&soap, serverURL, "", playerName, &resCode) == SOAP_OK)
    {
        if (resCode < 0)
        {
            printf("Error registering player: %d\n", resCode);
            exit(1);
        }
        printf("Player registered successfully with game ID: %d\n", resCode);
        gameId = resCode; // Save the game ID
    }
    else
    {
        soap_print_fault(&soap, stderr);
        exit(1);
    }

    // Game loop
    while (!endOfGame)
    {
        // Get the status of the game
        if (soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, gameId, &gameStatus) == SOAP_OK)
        {
            printBoard(gameStatus.board, gameStatus.msgStruct.msg);

            if (gameStatus.code == TURN_MOVE)
            {
                // It's the player's turn
                playerMove = readMove();

                // Insert the player's chip
                int status_insert;
                if (soap_call_conecta4ns__insertChip(&soap, serverURL, "", gameId, playerName, playerMove, &status_insert) == SOAP_OK)
                {

                    printBoard(gameStatus.board, gameStatus.msgStruct.msg);
                }
                else
                {
                    soap_print_fault(&soap, stderr);
                    endOfGame = TRUE;
                }
            }
            else if (gameStatus.code == TURN_WAIT)
            {
                // Wait for the opponent's move
                printf("Waiting for the other player...\n");
                sleep(2); // Delay before checking again
            }
            else if (gameStatus.code == GAMEOVER_LOSE)
            {
                endOfGame = TRUE;
                printf("Game ends, you lose\n");
                soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, gameId, &gameStatus);
                printBoard(gameStatus.board, gameStatus.msgStruct.msg);
            }
            else if (gameStatus.code == GAMEOVER_WIN)
            {
                endOfGame = TRUE;
                printf("Game ends, you win\n");
                soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, gameId, &gameStatus);
                printBoard(gameStatus.board, gameStatus.msgStruct.msg);
            }
            else if (gameStatus.code == GAMEOVER_DRAW)
            {
                endOfGame = TRUE;
                printf("Game ends in draw\n");
                soap_call_conecta4ns__getStatus(&soap, serverURL, "", playerName, gameId, &gameStatus);
                printBoard(gameStatus.board, gameStatus.msgStruct.msg);
            }
        }
        else
        {
            soap_print_fault(&soap, stderr);
            endOfGame = TRUE;
        }
    }

    // Clean the environment
    free(playerName.msg);
    soap_destroy(&soap);
    soap_end(&soap);
    soap_done(&soap);

    return 0;
}