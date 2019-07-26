#ifndef PARSER_INC
#define PARSER_INC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>

typedef enum{
	//ADD,
	//RUN,
	SELECT,
	INSERT,
	CREATE,
	DESCRIBE,
	DROP,
	JOURNALCOMANDO,
	INVALID_COMMAND,
	SALIR
}command_t;


typedef struct{
	//Entero identificando el comando. Ventaja: se puede usar en un switch
	command_t command;
	//String del comando
	char *command_str;
	//String del request
	char *request_str;
	//Vector de argumento, todos ellos strings
	char **args;
	//Cantidad de argumentos
	int cant_args;
}request_t;

/**
* @NAME: borrar_request
* @DESC: libera la memoria ocupada por una estructura del tipo request_t
*/
void borrar_request(request_t req);

/**
* @NAME: parser
* @DESC: recibe un request en un string y lo devuelve en una estructura del tipo request_t
*/
request_t parser(char* req);

#endif
