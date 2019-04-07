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

#define PATH_MEMORIA_CONFIG "MEMORIA.txt"
#define LOG_PATH "logMEMORIA.txt"

t_log* log_memoria;
int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
char* buffer;

typedef struct{
    int puerto;
    char* ip_fs;
    int puerto_fs;
    char* ip_seeds;
    char** puerto_seeds[10];
    int retardo_mem;
    int retardo_fs;
    int tam_mem;
    int retardo_journal;
    int retardo_gossiping;
    int memory_number;
}t_memoria_config;

t_memoria_config* arc_config;


void cargarConfiguracion();


#endif /* MEMORIA_H_ */


