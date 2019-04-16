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
char* linea;
t_header* buffer;

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

    //NUEVO CAMPO AÑADIDO, MAXIMO VALOR EN BYTE DE LA KEY
    //ESTA SOLO SE OBTIENE A PARTIR DE CONECTAR CON FS
    int max_value_key;
}t_memoria_config;



typedef struct {
	long timestamp;
	int16_t key;
	char* value;
}valor_pagina;

typedef struct {
	int numero;
	valor_pagina* pagina;
	bool flag;
}pagina;

typedef struct {
	int tamanio_segmento;
	int base_segmento_en_memoria;
	pagina* pagina []; //DEBE SER LISTA DE PAGINAS
}segmento;

typedef struct {
	int tamanioMemoria;
	segmento* segmentoMemoria[];
}memoria_principal;

memoria_principal* memoria;

t_memoria_config* arc_config;

/*---------------------------------------------------
 * FUNCIONES PARA MEMORIA PRINCIPAL
 ---------------------------------------------------*/
void armarMemoriaPrincipal();

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



/*---------------------------------------------------
 * MODIFICAR TIEMPO DE RETARDO DE CONFIGURACION
 *---------------------------------------------------*/

void modificarTiempoRetardoMemoria(int nuevoCampo);

void modificarTiempoRetardoFileSystem(int nuevoCampo);

void modificarTiempoRetardoGossiping(int nuevoCampo);

void modificarTiempoRetardoJournal(int nuevoCampo);

void modificarTIempoRetardo(int nuevoCampo, char* campoAModificar);

#endif /* MEMORIA_H_ */


