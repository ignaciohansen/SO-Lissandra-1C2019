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

void retardo_memoria(void){
	pthread_mutex_lock(&mutex_retardo_memoria);
	int milisegundos= arc_config->retardo_mem;
	pthread_mutex_unlock(&mutex_retardo_memoria);
	usleep(milisegundos*1000);
}

void retardo_fs(void){
	pthread_mutex_lock(&mutex_retardo_fs);
	int milisegundos= arc_config->retardo_fs;
	pthread_mutex_unlock(&mutex_retardo_fs);
	usleep(milisegundos*1000);
}

void * retardo_journal(void){
	while(1){
		int milisegundos= arc_config->retardo_journal;
		imprimirMensaje1(log_memoria, "[JOURNAL AUTOMATICO] PROXIMO JOURNAL EN %d milisegundos",milisegundos);
		activo_retardo_journal=false;
		usleep(milisegundos*1000);
		//LUEGO DE ESTO EMPIEZA UN JOURNAL;
	//	char* journalAutomatico = malloc(sizeof("**********JOURNAL AUTOMATICO ACTIVADO**********\n\n")+1);
	//	memcpy(journalAutomatico, "**********JOURNAL AUTOMATICO ACTIVADO**********\n\n", strlen("**********JOURNAL AUTOMATICO ACTIVADO**********\n\n")+1);


		pthread_mutex_lock(&JOURNALHecho);
		mutexBloquear(&verificarSiBitmapLleno);
		activo_retardo_journal = true;
		rwLockEscribir(&sem_insert_select);
		int cant_pasados = JOURNAL(-1);
		rwLockDesbloquear(&sem_insert_select);
		fprintf(tablas_fp,"\nEjecutado JOURNAL AUTOMÁTICO");
		loggearEstadoActual(tablas_fp);
		imprimirMensaje1(log_memoria, "[JOURNAL AUTOMATICO] %d registros recibidos ok por LFS", cant_pasados);
		mutexDesbloquear(&verificarSiBitmapLleno);
	//	retardo_journal(milisegundos);
	}
}
