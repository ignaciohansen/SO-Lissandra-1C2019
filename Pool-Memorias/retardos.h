/*
 * retardos.h
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */

#ifndef RETARDOS_H_
#define RETARDOS_H_

#include <sys/sem.h>
#include <time.h>
#include <pthread.h>
#include "../Biblioteca/src/Biblioteca.h"

//#include "estructuras.h"

pthread_mutex_t mutex_retardo_memoria;
pthread_mutex_t mutex_retardo_fs;
pthread_mutex_t mutex_retardo_gossiping;
pthread_mutex_t mutex_retardo_journal;

//void iniciarSemaforosRetados(void);
void retardo_memoria(void);
void retardo_fs(void);
void *retardo_journal(void);

#endif /* RETARDOS_H_ */
