/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../Biblioteca/src/Biblioteca.c"

#define PATH_KERNEL_CONFIG "KERNEL.txt"
#define LOG_PATH "logKERNEL.txt"

t_log* log_kernel;

typedef struct{
    int puerto_escucha;
    char* punto_montaje;
    int retardo;
    int tamanio_value;
    int tiempo_dump;
}t_kernel_config;

t_kernel_config* configFile;


void cargarConfiguracion();


#endif /* KERNEL_H_ */
