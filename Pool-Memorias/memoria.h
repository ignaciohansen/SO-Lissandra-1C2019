/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../Biblioteca/src/Biblioteca.c"

#define PATH_MEMORIA_CONFIG "../MEMORIA.txt"
#define LOG_PATH "logMEMORIA.txt"
#define MAXSIZE_COMANDO 200
#define ERROR -1

t_log* log_memoria;
int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
int sockeConexionLF, resultado_sendMsj;
int tamanio;
char* buffer, linea;


typedef struct{
    int puerto;
    char* ip_fs;
    int puerto_fs;
/*
    char* ip_seeds[10];    // esto debe ser flexible
    int puerto_seeds[10];  // esto debe ser flexible
*/
    char* ip_seeds;
    //SACO EL [10] A PUERTO_SEEDS
    char** puerto_seeds;
    int retardo_mem;
    int retardo_fs;
    int tam_mem;
    int retardo_journal;
    int retardo_gossiping;
    int memory_number;
}t_memoria_config;

t_memoria_config* arc_config;


/*---------------------------------------------------
 * FUNCIONES PARA LA CONFIGURACION Y FINALIZACION
 ---------------------------------------------------*/

void cargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);
void inicioLogYConfig();

/*---------------------------------------------------
 * FUNCIONES PARA LA CONSOLA
 ---------------------------------------------------*/

void ejecutarHiloConsola();
void consola();
void menu();
void validarComando(char*,t_log*);
int buscarComando(char*,t_log*);
int enviarComando(char**, t_log*);

/*---------------------------------------------------
 * FUNCIONES PARA LAS CONEXIONES
 *---------------------------------------------------*/

void crearConexionesConOtrosProcesos();
void levantarServidor();
void conectarConServidorLisandraFileSystem();
void crearHIloEscucharKernel();
void escucharConexionKernel();
void crearHIloEscuchaLFS();


#endif /* MEMORIA_H_ */


