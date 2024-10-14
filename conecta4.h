//gsoap conecta4ns service name: conecta4
//gsoap conecta4ns service style: rpc
//gsoap conecta4ns service location: http//localhost:10000
//gsoap conecta4ns service encoding: encoded
//gsoap conecta4ns service namespace: urn:conecta4ns

/** Server is full. No more games are allowed */
#define ERROR_SERVER_FULL -1000

/** Wrong ID. Game does not exist */
#define ERROR_WRONG_GAMEID -2000

/** Player not found */
#define ERROR_PLAYER_NOT_FOUND -3000

/** Player not found */
#define ERROR_PLAYER_REPEATED -4000

/** Code for performing a move */
#define TURN_MOVE 70001

/** Code for waiting */
#define TURN_WAIT 70002

/** Code to show that the player wins */
#define GAMEOVER_WIN 50052

/** Code to show a draw game */
#define GAMEOVER_DRAW 50053

/** Code to show that the player loses */
#define GAMEOVER_LOSE 50054

/** Board width (in number of cells) */
#define BOARD_WIDTH 7

/** Board height (in number of cells) */
#define BOARD_HEIGHT 6

/** Character for player 1 chip */
#define PLAYER_1_CHIP 'o'

/** Character for player 2 chip */
#define PLAYER_2_CHIP 'x'

/** Character for empty cell */
#define EMPTY_CELL ' '

/** True value */
#define TRUE 1

/** False value */
#define FALSE 0

/** Length for tString */
#define STRING_LENGTH 128

/** Players */
typedef enum players {player1, player2} conecta4ns__tPlayer;

/** Result for moves */
typedef enum moves {OK_move, fullColumn_move} conecta4ns__tMove;

/** Dynamic array of chars */
typedef char *xsd__string;

/** Message used to send the player's name and messages sent from server */
typedef struct tMessage{
	int __size;
	xsd__string msg;
}conecta4ns__tMessage;

/** Response from the server */
typedef struct tBlock{
	int code;
	conecta4ns__tMessage msgStruct;
	int __size;
	xsd__string board;
}conecta4ns__tBlock;

int conecta4ns__register  	(conecta4ns__tMessage playerName, int *code);
int conecta4ns__getStatus 	(conecta4ns__tMessage playerName, int gameId, conecta4ns__tBlock* status);

