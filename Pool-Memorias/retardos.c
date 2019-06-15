/*
 * retardos.c
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */
#include <time.h>

void iniciarSemaforosRetados(){
	mutexIniciar(&mutex_retardo_memoria);
	mutexIniciar(&mutex_retardo_gossiping);
	mutexIniciar(&mutex_retardo_fs);
	mutexIniciar(&mutex_retardo_journal);
}

void retardo_memoria(int milisegundos){
	mutexBloquear(&mutex_retardo_memoria);
	usleep(milisegundos*1000);
	mutexDesbloquear(&mutex_retardo_memoria);
}

void retardo_fs(int milisegundos){
	mutexBloquear(&mutex_retardo_fs);
	usleep(milisegundos*1000);
	mutexDesbloquear(&mutex_retardo_fs);
}

void retardo_gossiping(int milisegundos){
	while(1){
		mutexBloquear(&mutex_retardo_gossiping);
		usleep(milisegundos*1000);
		//LUEGO DE ESTO ACTIVARA LA FUNCION GOSSIPING
	//	GOSSIPING();
		mutexDesbloquear(&mutex_retardo_gossiping);
	}
}

void retardo_journal(int milisegundos){
	while(1){
		mutexBloquear(&mutex_retardo_journal);
		usleep(milisegundos*1000);
		//LUEGO DE ESTO EMPIEZA UN JOURNAL;
		JOURNAL();
		mutexDesbloquear(&mutex_retardo_journal);
	}
}
