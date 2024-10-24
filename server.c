#include "server.h"

/** Flag to enable debugging */
#define DEBUG_SERVER 1

/** Array with games */
tGame games[MAX_GAMES];

/** Mutex to protect the game status field in the array of games */
pthread_mutex_t mutexStatusArray;


void initServerStructures (){

    if (DEBUG_SERVER)
        printf ("Initializing...\n");

    // Init seed
    srand (time(NULL));

    // Init each game
    for (int i=0; i<MAX_GAMES; i++){

        // Allocate and init board
        games[i].board = (xsd__string) malloc (BOARD_WIDTH*BOARD_HEIGHT);
        initBoard (games[i].board);

        // Calculate the first player to play
        if ((rand()%2)==0)
            games[i].currentPlayer = player1;
        else
            games[i].currentPlayer = player2;

        // Allocate and init player names
        games[i].player1Name = (xsd__string) malloc (STRING_LENGTH);
        games[i].player2Name = (xsd__string) malloc (STRING_LENGTH);
        memset (games[i].player1Name, 0, STRING_LENGTH);
        memset (games[i].player2Name, 0, STRING_LENGTH);

        // Game status
        games[i].endOfGame = FALSE;
        games[i].status = gameEmpty;

        // Init mutex and cond variable
    }
}

conecta4ns__tPlayer switchPlayer (conecta4ns__tPlayer currentPlayer){
    return (currentPlayer == player1) ? player2 : player1;
}

int searchEmptyGame (){
	for(int i = 0;i<MAX_GAMES;i++){
		if(games[i].status == gameEmpty);
			return i;
	}
	
}

int checkPlayer (xsd__string playerName, int gameId){

  
}

void freeGameByIndex (int index){

	
}

void copyGameStatusStructure (conecta4ns__tBlock* status, char* message, xsd__string board, int newCode){
    
    // Set the new code
    status->code = newCode;
    
    // Copy the message
    memset((status->msgStruct).msg, 0, STRING_LENGTH);
    strcpy ((status->msgStruct).msg, message);
    (status->msgStruct).__size = strlen ((status->msgStruct).msg);
    
    // Copy the board, only if it is not NULL
    if (board == NULL){
        status->board = NULL;
        status->__size = 0;
    }
    else{
        strncpy (status->board, board, BOARD_WIDTH*BOARD_HEIGHT);
        status->__size = BOARD_WIDTH*BOARD_HEIGHT;
    }
}


int conecta4ns__register(struct soap *soap, conecta4ns__tMessage playerName, int *code) {
    int gameIndex = -1;
    int result = 0;
    
    // Asegurarse de que el string termine en '\0'
    playerName.msg[playerName.__size] = '\0';

    if (DEBUG_SERVER)
        printf("[Register] Registering new player -> [%s]\n", playerName.msg);

    // Buscar un juego con espacio disponible
    for (int i = 0; i < MAX_GAMES; i++) {
        pthread_mutex_lock(&games[i].status); // Proteger acceso concurrente

        if (games[i].status == gameEmpty || games[i].status == gameWaitingPlayer) {
            gameIndex = i;
            // Verificar si el jugador ya está registrado en esta partida
            if ((games[i].player1Name != NULL && strcmp(games[i].player1Name, playerName.msg) == 0) ||
                (games[i].player2Name != NULL && strcmp(games[i].player2Name, playerName.msg) == 0)) {
                *code = ERROR_PLAYER_REPEATED; // Jugador ya registrado en esta partida
                pthread_mutex_unlock(&games[i].status); // Liberar mutex antes de retornar
                return SOAP_OK;
            }

            // Registrar jugador en la partida
            if (games[i].status == gameEmpty) {
                games[i].player1Name = strdup(playerName.msg); // Registrar como jugador 1
                games[i].status = gameWaitingPlayer;
            } else if (games[i].status == gameWaitingPlayer) {
                games[i].player2Name = strdup(playerName.msg); // Registrar como jugador 2
                games[i].status = gameReady;
            }

            *code = gameIndex; // Retornar el ID del juego
            pthread_mutex_unlock(&games[i].status); // Liberar mutex
            return SOAP_OK;
        }
        pthread_mutex_unlock(&games[i].status); // Liberar mutex si no se encontró espacio
    }

    // Si no se encontró espacio, devolver error de servidor lleno
    if (gameIndex == -1) {
        *code = ERROR_SERVER_FULL;
        return SOAP_OK;
    }

    return SOAP_OK;
}

int conecta4ns__getStatus(struct soap *soap, conecta4ns__tMessage playerName, int gameId, conecta4ns__tBlock* status) {
    char messageToPlayer[STRING_LENGTH];

    // Asegurarse de que el string termine en '\0' y reservar memoria para el status
    playerName.msg[playerName.__size] = '\0';
    allocClearBlock(soap, status);

    if (DEBUG_SERVER)
        printf("Receiving getStatus() request from -> %s [%d] in game %d\n", playerName.msg, playerName.__size, gameId);

    // Verificar si el gameId es válido
    if (gameId < 0 || gameId >= MAX_GAMES || games[gameId].status == gameEmpty) {
        strcpy(messageToPlayer, "Invalid game ID.");
        copyGameStatusStructure(status, messageToPlayer, NULL, "error");
        return SOAP_OK;
    }

    // Proteger el acceso concurrente al juego
    pthread_mutex_lock(&games[gameId]);

    // Verificar si el jugador es parte de este juego
    if ((games[gameId].player1Name != NULL && strcmp(games[gameId].player1Name, playerName.msg) != 0) &&
        (games[gameId].player2Name != NULL && strcmp(games[gameId].player2Name, playerName.msg) != 0)) {
        strcpy(messageToPlayer, "Player not registered in this game.");
        copyGameStatusStructure(status, messageToPlayer, NULL, ERROR_PLAYER_NOT_FOUND);
        pthread_mutex_unlock(&games[gameId]);
        return SOAP_OK;
    }

    // Comprobar si es el turno del jugador o si tiene que esperar
    /*if (strcmp(games[gameId].currentPlayer, playerName.msg) == 0) {
        strcpy(messageToPlayer, "It's your turn.");
        copyGameStatusStructure(status, messageToPlayer, games[gameId].board, TURN_MOVE);
    } else {
        strcpy(messageToPlayer, "Waiting for the other player.");
        copyGameStatusStructure(status, messageToPlayer, games[gameId].board, TURN_WAIT);
    }*/

    // Comprobar si la partida ha terminado (victoria, empate, derrota)
    if (games[gameId].status == GAMEOVER_WIN) {
        strcpy(messageToPlayer, "You won!");
        copyGameStatusStructure(status, messageToPlayer, games[gameId].board, GAMEOVER_WIN);
    } else if (games[gameId].status == GAMEOVER_DRAW) {
        strcpy(messageToPlayer, "It's a draw.");
        copyGameStatusStructure(status, messageToPlayer, games[gameId].board, GAMEOVER_DRAW);
    } else if (games[gameId].status == GAMEOVER_LOSE) {
        strcpy(messageToPlayer, "You lost.");
        copyGameStatusStructure(status, messageToPlayer, games[gameId].board, GAMEOVER_LOSE);
    }

    pthread_mutex_unlock(&games[gameId]);

    return SOAP_OK;
}


void *processRequest(void *soap){

	pthread_detach(pthread_self());

	if (DEBUG_SERVER)
		printf ("Processing a new request...");

	soap_serve((struct soap*)soap);
	soap_destroy((struct soap*)soap);
	soap_end((struct soap*)soap);
	soap_done((struct soap*)soap);
	free(soap);

	return NULL;
}

int main(int argc, char **argv){ 

	struct soap soap;
	struct soap *tsoap;
	pthread_t tid;
	int port;
	SOAP_SOCKET m, s;

		// Init soap environment
		soap_init(&soap);

		// Configure timeouts
		soap.send_timeout = 60; // 60 seconds
		soap.recv_timeout = 60; // 60 seconds
		soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
		soap.max_keep_alive = 100; // max keep-alive sequence

		initServerStructures();
        pthread_mutex_init(&mutexStatusArray, NULL);

		// Get listening port
		port = atoi(argv[1]);

		// Bind
		m = soap_bind(&soap, NULL, port, 100);

		if (!soap_valid_socket(m)){
			exit(1);
		}

		printf("Server is ON ...\n");

		while (TRUE){

			// Accept a new connection
			s = soap_accept(&soap);

			// Socket is not valid :(
			if (!soap_valid_socket(s)){

				if (soap.errnum){
					soap_print_fault(&soap, stderr);
					exit(1);
				}

				fprintf(stderr, "Time out!\n");
				break;
			}

			// Copy the SOAP environment
			tsoap = soap_copy(&soap);

			if (!tsoap){
				printf ("SOAP copy error!\n");
				break;
			}

			// Create a new thread to process the request
			pthread_create(&tid, NULL, (void*(*)(void*))processRequest, (void*)tsoap);
		}

	// Detach SOAP environment
	soap_done(&soap);
	return 0;
}
