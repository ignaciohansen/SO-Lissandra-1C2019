/*
 * ConsolaRemotaLFS.h
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#ifndef CONSOLAREMOTALFS_H_
#define CONSOLAREMOTALFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// SYS
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
// RED
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
// COMMONS
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
// BICLIOTECA
#include "../Biblioteca/src/Biblioteca.c"
// READLINE
#include <readline/readline.h>
#include <readline/history.h>
// HILOS
#include <pthread.h>

#define PATH_CREMOTA_CONFIG "../CREMOTA.txt"
#define LOG_PATH "LOG_CREMOTA.txt"

int socketCRemota,socketAsociadoCRemota,msgSize,sendedMsgStatus;
t_log* log_cremota;

//Socket
int sendResult;

typedef struct{

	//Estos son los datos obligatorios que deben estar
	char* ip_servidor;
	int puerto_servidor;
}t_cremota_config;

t_cremota_config* crem_config;

typedef struct {
	AddrInfo informacion;
	SockAddrIn address;
	Socklen tamanioAddress;
	String port;
	String ip;
}conexion;

t_header* protocoloHeader;

conexion* estructuraConexion;

/*Elementos de consola*/
#define MAXSIZE_COMANDO 200

char* linea;
char** lineaSeparada;
int comando;
int confirmacionRecibida;
FILE* fd;
char *separador2 = "\n";
char *separator = " ";
void consola();
void menu();



pthread_mutex_t mutexColaNuevos;
pthread_mutex_t mutexColaListos;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaEjecucion;



//
void  cargarConfiguracion();
int   conexionKernel();

// RED
int   enviarComando(char** comando,t_log* logger);
int   enviarMensaje(int comando, int tamanio,char* mensaje, t_log* logger);
void  armarMensajeBody(int tamanio,char* mensaje,char** comando);

// CONSOLA
int   buscarComando(char* comandoSeparado,t_log* logger);
void  validarLinea(char** lineaIngresada,t_log* logger);
void  comandoRun(char* path,t_log* logger);
void  validarComando(char** comandoSeparado,int tamanio,t_log* logger);

#endif /* CONSOLAREMOTALFS_H_ */
