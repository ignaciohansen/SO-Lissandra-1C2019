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

void retardo_memoria(){
	int milisegundos= arc_config->retardo_mem;
	pthread_mutex_lock(&mutex_retardo_memoria);
	usleep(milisegundos*1000);
	pthread_mutex_unlock(&mutex_retardo_memoria);
}

void retardo_fs(){
	int milisegundos= arc_config->retardo_fs;
	pthread_mutex_lock(&mutex_retardo_fs);
	usleep(milisegundos*1000);
	pthread_mutex_unlock(&mutex_retardo_fs);
}

void retardo_gossiping(){
	while(1){
		int milisegundos= arc_config->retardo_gossiping;
		pthread_mutex_lock(&mutex_retardo_gossiping);
		usleep(milisegundos*1000);
		//LUEGO DE ESTO ACTIVARA LA FUNCION GOSSIPING //	GOSSIPING();
		pthread_mutex_unlock(&mutex_retardo_gossiping);
	}
}

void retardo_journal(){
	while(1){
		int milisegundos= arc_config->retardo_journal;
		imprimirAviso1(log_memoria, "\n\nPROXIMO JOURNAL EN %d milisegundos\n\n>",milisegundos);
		activo_retardo_journal=false;
		usleep(milisegundos*1000);
		//LUEGO DE ESTO EMPIEZA UN JOURNAL;
	//	char* journalAutomatico = malloc(sizeof("**********JOURNAL AUTOMATICO ACTIVADO**********\n\n")+1);
	//	memcpy(journalAutomatico, "**********JOURNAL AUTOMATICO ACTIVADO**********\n\n", strlen("**********JOURNAL AUTOMATICO ACTIVADO**********\n\n")+1);

		pthread_mutex_lock(&JOURNALHecho);
		mutexBloquear(&verificarSiBitmapLleno);
		activo_retardo_journal = true;
		JOURNAL();
		mutexDesbloquear(&verificarSiBitmapLleno);
	//	retardo_journal(milisegundos);
	}
}
