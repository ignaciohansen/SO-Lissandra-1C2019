/*
 * retardos.h
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */
/*
#ifndef RETARDOS_H_
#define RETARDOS_H_
*/
#include <sys/sem.h>
#include <time.h>
#include <pthread.h>
//#include "../Biblioteca/src/Biblioteca.h"

//#include "estructuras.h"

pthread_mutex_t mutex_retardo_memoria;
pthread_mutex_t mutex_retardo_fs;
pthread_mutex_t mutex_retardo_gossiping;
pthread_mutex_t mutex_retardo_journal;

void iniciarSemaforosRetados();
void retardo_memoria(int milisegundos);
void retardo_fs(int milisegundos);
void retardo_gossiping(int milisegundos);
void retardo_journal(int milisegundos);

//#endif /* RETARDOS_H_ */
