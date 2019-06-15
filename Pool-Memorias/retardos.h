/*
 * retardos.h
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */

#ifndef RETARDOS_H_
#define RETARDOS_H_


Mutex mutex_retardo_memoria;
Mutex mutex_retardo_fs;
Mutex mutex_retardo_gossiping;
Mutex mutex_retardo_journal;

void iniciarSemaforosRetados();

void retardo_memoria(int milisegundos);

void retardo_fs(int milisegundos);

void retardo_gossiping(int milisegundos);

void retardo_journal(int milisegundos);





















#endif /* RETARDOS_H_ */
