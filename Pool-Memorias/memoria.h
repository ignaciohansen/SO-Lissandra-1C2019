/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

/*
#ifndef MEMORIA_H_
#define MEMORIA_H_
*/
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
#include "../Biblioteca/src/Biblioteca.c"


#define PATH_MEMORIA_CONFIG "../Config/MEMORIA.txt"
#define LOG_PATH "../Log/logMEMORIA.txt"
#define MAXSIZE_COMANDO 200
#define ERROR -1

t_config* configFile;

int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
int sockeConexionLF, resultado_sendMsj;
int tablaPaginaArmada;
short memoriaArmada;
int tamanio, limpiandoMemoria;
char* linea;
char* argumentosComando;
char** argumentosParseados;
t_header* buffer;

/*
 * 									HILOS
 */

pthread_t hiloConsolaMemoria;


/*---------------------------------------------------------------------------------
 * 								MUTEX Y SEMAFOROS
 ----------------------------------------------------------------------------------*/





int main();


/*---------------------------------------------------
 * FUNCIONES PARA MEMORIA PRINCIPAL
 ---------------------------------------------------*/
void armarMemoriaPrincipal();
//void obtenerInfoDePagina(int i, void** informacion);

/*---------------------------------------------------
 * FUNCIONES PARA LA CONFIGURACION Y FINALIZACION
 ---------------------------------------------------*/

void cargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);
void inicioLogYConfig();
void liberar_todo_por_cierre_de_modulo();
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

int funcionDescribe(char* nombreTablaAIr);
int funcionInsert(char* nombreTabla, u_int16_t keyBuscada, char* valorAPoner, bool estadoAPoner);
int funcionSelect(char* nombreTablaAIr, u_int16_t keyBuscada, pagina_a_devolver** datos_a_devolver,
		char** valorADevolver);
/*---------------------------------------------------
 * PROCESO JOURNAL
 *---------------------------------------------------*/

void JOURNAL();

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

//#endif /* MEMORIA_H_ */
