/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
// STANDARD
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
#include <semaphore.h>

#define PATH_KERNEL_CONFIG "../Config/KERNEL.txt"
#define LOG_PATH "../Log/LOG_KERNEL.txt"


int socket_CMemoria,tamanio,countPID,multiProcesamiento;
t_log* log_kernel;
t_list* list_queries;

//Socket

int resultado_Conectar, resultado_sendMsj;

//Semaforos
int valorMultiprocesamiento;
Mutex countProcess;
sem_t multiprocesamiento;

typedef struct{

	//Estos son los datos obligatorios que deben estar
	char* ip_memoria;
	int puerto_memoria;
	int quantum; //PARA ALGORITMO ROUND ROBIN
	int multiprocesamiento;
	int metadata_refresh;
	int sleep_ejecucion;
}t_kernel_config;

t_kernel_config* arc_config;

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
char **comandoSeparado;
int comando;
int confirmacionRecibida;
FILE* fd;
char *separador2 = "\n";
char *separator = " ";
void consola();
void menu();

/*
 * PCB
 * */
typedef struct{

	int pid;
	int estado;
	int progamCounter;
	int rafaga;
	int comando;
	int argumentos;
	t_list* stack;
}t_pcb;

t_pcb* crearPcb(char* comando);


/*
 * Planificador
 * */

void inicializarListasPlanificador();
void planificar(char* linea);
void agregarAListo(t_pcb* procesoNuevo);
void agregarANuevo(char* linea);
t_pcb* crearEstructurasAdministrativas(char* linea);
void agregarAEjecutar(char* linea);
void agregarAExit();
int rafagaComandoRun(char* path);

pthread_mutex_t mutexColaNuevos;
pthread_mutex_t mutexColaListos;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaEjecucion;

t_list* colaNuevos;
t_list* colaListos;
t_list* colaExit;
t_list* colaEjecucion;

enum estados{

	nuevo = 0,
	ejecucion,
	listo,
	exiT
};

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

#endif /* KERNEL_H_ */
