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
#include "../Biblioteca/src/Biblioteca.h"
// READLINE
#include <readline/readline.h>
#include <readline/history.h>
// HILOS
#include <pthread.h>
#include <semaphore.h>

//GOSSIPING
#include "../Biblioteca/src/Gossiping.h"
#include "../Biblioteca/src/Gossiping.c"

#define PATH_KERNEL_CONFIG "../Config/KERNEL.txt"
#define LOG_PATH "../Log/LOG_KERNEL.txt"

id_com_t soy = KERNEL;
int socket_CMemoria,tamanio,countPID,multiProcesamiento;
t_log* log_kernel;
t_list* list_queries;
int aux_memoria;

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
//FILE* fd;
char *separador2 = "\n";
char *separator = " ";


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
	char* linea;
}t_pcb;

t_pcb* crearPcb(char* comando);

const char* comandosPermitidos[] = { "SELECT", "INSERT", "CREATE", "DESCRIBE",
		"DROP", "JOURNAL", "ADD", "RUN", "METRICS", "SALIR"

};

enum comandos{
	SELECT = 0,
	INSERT,
	CREATE,
	DESCRIBE,
	DROP,
	JOURNAL,
	ADD,
	RUN,
	METRICS,
	SALIR

};

/*
 * Planificador
 * */

void inicializarListasPlanificador();
void planificar(char* linea);
void agregarAListo(t_pcb* procesoNuevo);
void agregarANuevo(char* linea);
t_pcb* crearEstructurasAdministrativas(char* linea);
void agregarAEjecutar(t_pcb* procesoAgregar);
void agregarAExit();
int rafagaComandoRun(char* path);

pthread_mutex_t mutexColaNuevos;
pthread_mutex_t mutexColaListos;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaEjecucion;
pthread_mutex_t mutex_retardos_kernel;
sem_t s_Multiprocesamiento;

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

/*TODO Tablas */
typedef struct{
	char* nombreTabla;
	int criterio;
}t_tablas;

t_tablas tablaPrueba;

/*TODO Estructuras para memoria*///GOSSIPING

typedef struct{
	int criterio;
	gos_com_t listMemoriaas;
}t_criterios;

typedef struct{
	int criterio;
	seed_com_t* listMemoriaas;
}t_criterios_prueba;
t_criterios_prueba criterio_memoria;

t_criterios criterioSC;
t_criterios criterioSHC;
t_criterios criterioEC;

gos_com_t gossipingKernel;
seed_com_t memorias;
t_list *lista_memorias;

/*TODO Enum de Criterios*/

enum criterios{

	SC,
	SHC,
	EC
};
const char* criterios[] = { "SC", "SHC", "EC"};


int buscarCriterio(char* criterio);
void actualizarMemoriasDisponibles();
void gossiping_Kernel();
seed_com_t* buscarMemoria(char** pruebaPath);

//
void  cargarConfiguracion();
int   conexionKernel();
int conexionAMemoria(char ip[LARGO_IP], char puerto[LARGO_PUERTO]);

// RED
int conectar_a_memoria(char ip[LARGO_IP], char puerto[LARGO_PUERTO]);
int   enviarComando(char** comando,t_log* logger);
int   enviarMensaje(int comando, int tamanio,char* mensaje, t_log* logger);
void  armarMensajeBody(int tamanio,char* mensaje,char** comando);

// CONSOLA
void consola();
void menu();
int buscarComando(char* comandoSeparado);
void  validarLinea(char** lineaIngresada,t_log* logger);
void  comandoRun(char* path,t_log* logger);
void  validarComando(char** comandoSeparado,int tamanio,t_log* logger);
void comandoAdd(char** comandoSeparado);
void comandoJournal(char** comandoSeparado);
void comandoMetrics();
void recargarConfiguracion(char* path_config);

//Inotify para parametros de configuracion que cambian durante ejecucion
#include <sys/inotify.h>

//void inotifyAutomatico(char* pathDelArchivoAEscuchar);

// El tamaño de un evento es igual al tamaño de la estructura de inotify
// mas el tamaño maximo de nombre de archivo que nosotros soportemos
// en este caso el tamaño de nombre maximo que vamos a manejar es de 24
// caracteres. Esto es porque la estructura inotify_event tiene un array
// sin dimension ( Ver C-Talks I - ANSI C ).
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )

// El tamaño del buffer es igual a la cantidad maxima de eventos simultaneos
// que quiero manejar por el tamaño de cada uno de los eventos. En este caso
// Puedo manejar hasta 1024 eventos simultaneos.
#define BUF_LEN     ( 1024 * EVENT_SIZE )

void inotifyAutomatico(char* pathDelArchivoAEscuchar);

#endif /* KERNEL_H_ */
