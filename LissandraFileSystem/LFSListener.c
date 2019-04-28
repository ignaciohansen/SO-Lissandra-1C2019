/*
 * LFSListener.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"
#include "LFSListener.h"

// LOGGEA todo lo que escucha.
int listenSomeLQL(t_log* logger) {

	while(1) {

	}

	imprimirMensaje(logger, " \n ====== LFS Listener: waiting for client connections ====== \n ");

	conexionEntrante = aceptarConexionSocket(socketEscuchaMemoria,logger);

	Puntero buffer = (void*)string_new(); // malloc(sizeof(char)*100);

	recibiendoMensaje = socketRecibir(conexionEntrante, buffer, 25,  logger);

	int size = sizeof(buffer);
	buffer[25] = '\0';

	char* msg = string_new();
	string_append(&msg,"Mensaje recibido: \""); string_append(&msg,buffer ); string_append(&msg,"\"." );

	imprimirVerde(logger, msg);

	free(buffer);

}
