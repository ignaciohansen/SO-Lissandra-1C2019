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
<<<<<<< HEAD
//#include "../../Biblioteca/src/Biblioteca.c"
=======
>>>>>>> 8877cc8822efa0f8fd848b4f4a86cbb7447213b7
#include "../Biblioteca/src/Biblioteca.c"

#define PATH_MEMORIA_CONFIG "MEMORIA.txt"
#define LOG_PATH "logMEMORIA.txt"

t_log* log_memoria;
int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
char* buffer;

typedef struct{
    int puerto;
    char* ip_fs;
    int puerto_fs;
<<<<<<< HEAD
    char* ip_seeds[10];    // esto debe ser flexible
    int puerto_seeds[10];  // esto debe ser flexible
=======
    char* ip_seeds;
    char** puerto_seeds[10];
>>>>>>> 8877cc8822efa0f8fd848b4f4a86cbb7447213b7
    int retardo_mem;
    int retardo_fs;
    int tam_mem;
    int retardo_journal;
    int retardo_gossiping;
    int memory_number;
}t_memoria_config;

t_memoria_config* arc_config;


void cargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);


#endif /* MEMORIA_H_ */


