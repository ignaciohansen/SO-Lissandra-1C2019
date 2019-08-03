/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */


#ifndef MEMORIA_H_
#define MEMORIA_H_

//#include "estructurasMemoria.h"
#include "estructuras.h"
#include "retardos.h"

//#include "../Biblioteca/src/Biblioteca.c"
//#include "../Biblioteca/src/Biblioteca.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "parser.h"
#include "../Biblioteca/src/Biblioteca.h"
#include "../Biblioteca/src/Gossiping.h"

#define PATH_MEMORIA_CONFIG "../Config/MEMORIA.txt"
#define LOG_PATH "../Log/logMEMORIA.txt"
#define LOG_TABLAS_PATH "../Log/log_TABLAS_MEMORIA.txt"
#define MAXSIZE_COMANDO 200
#define ERROR -1

t_config *configFile;
t_header* buffer;

int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
int sockeConexionLF, resultado_sendMsj;
char* linea;
char* argumentosComando;
char** argumentosParseados;


/*
 * 									HILOS
 */

pthread_t hiloConsolaMemoria;


/*---------------------------------------------------------------------------------
 * 								MUTEX Y SEMAFOROS
 ----------------------------------------------------------------------------------*/


void iniciarSemaforosYMutex();


int main();


/*---------------------------------------------------
 * FUNCIONES PARA MEMORIA PRINCIPAL
 ---------------------------------------------------*/
//void obtenerInfoDePagina(int i, void** informacion);

/*---------------------------------------------------
 * FUNCIONES PARA LA CONFIGURACION Y FINALIZACION
 ---------------------------------------------------*/

void cargarConfiguracion(char *path_config);
void recargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);
void inicioLogYConfig(int numMemoria, bool loggearEnConsola);
void cerrarTodosLosHilosPendientes();

/*---------------------------------------------------
 * FUNCIONES PARA LA CONSOLA
 ---------------------------------------------------*/

void ejecutarHiloConsola();
void consola();
void menu();
void validarComando(char*,t_log*);
int buscarComando(char*,t_log*);
int enviarComando(char**, t_log*);

void imprimirPorPantallaTodosLosComandosDisponibles();

/*---------------------------------------------------
 * FUNCIONES PARA LAS CONEXIONES
 *---------------------------------------------------*/

void crearConexionesConOtrosProcesos();
void levantarServidor();
void conectarConServidorLisandraFileSystem();
void crearHIloEscucharKernel();
void escucharConexionKernel();
void crearHIloEscuchaLFS();

void hiloInsert(request_t* req);
void hiloSelect(request_t* req);
void hiloDescribe(request_t* req);
void hiloDrop(request_t* req);

/*---------------------------------------------------
 * PROCESO JOURNAL
 *---------------------------------------------------*/

//void JOURNAL();

//void pasar_valores_modificados_a_Lisandra(segmento* elSegmento, unidad_memoria* unidad_de_memoria);

/*---------------------------------------------------
 * MODIFICAR TIEMPO DE RETARDO DE CONFIGURACION
 *---------------------------------------------------*/


void modificarTiempoRetardoMemoria(int nuevoCampo);

void modificarTiempoRetardoFileSystem(int nuevoCampo);

void modificarTiempoRetardoGossiping(int nuevoCampo);

void modificarTiempoRetardoJournal(int nuevoCampo);

void modificarTIempoRetardo(int nuevoCampo, int campoAModificar);

/*---------------------------------------------------
 * FUNCIONES OBTENER VALORES MEDIANTE UNA KEY
 *---------------------------------------------------*/

#endif /* MEMORIA_H_ */
