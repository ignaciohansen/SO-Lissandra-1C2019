/*
 * retardos.c
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */
#include "retardos.h"
#include <unistd.h>
#include "gestionMemoria.h"
//#include "../Biblioteca/src/Biblioteca.c"

void retardo_memoria(int milisegundos){
	pthread_mutex_lock(&mutex_retardo_memoria);
	usleep(milisegundos*1000);
	pthread_mutex_unlock(&mutex_retardo_memoria);
}

void retardo_fs(int milisegundos){
	pthread_mutex_lock(&mutex_retardo_fs);
	usleep(milisegundos*1000);
	pthread_mutex_unlock(&mutex_retardo_fs);
}

void retardo_gossiping(int milisegundos){
	while(1){
		pthread_mutex_lock(&mutex_retardo_gossiping);
		usleep(milisegundos*1000);
		//LUEGO DE ESTO ACTIVARA LA FUNCION GOSSIPING //	GOSSIPING();
		pthread_mutex_unlock(&mutex_retardo_gossiping);
	}
}

void retardo_journal(int milisegundos){
	while(1){
		printf("\n\nPROXIMO JOURNAL EN %d milisegundos\n\n>",milisegundos);
		activo_retardo_journal=false;
		usleep(milisegundos*1000);
		//LUEGO DE ESTO EMPIEZA UN JOURNAL;
		pthread_mutex_lock(&JOURNALHecho);
		activo_retardo_journal = true;
		JOURNAL();
	}
}
