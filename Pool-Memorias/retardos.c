/*
 * retardos.c
 *
 *  Created on: 13 jun. 2019
 *      Author: utnso
 */
#include <time.h>

void retardo_memoria(int milisegundos){
	usleep(milisegundos*1000);
}

void retardo_fs(int milisegundos){
	usleep(milisegundos*1000);
}

void retardo_gossiping(int milisegundos){
	while(1){
		usleep(milisegundos*1000);
		//LUEGO DE ESTO ACTIVARA LA FUNCION GOSSIPING
	//	GOSSIPING();
	}
}

void retardo_journal(int milisegundos){
	while(1){
		usleep(milisegundos*1000);
		//LUEGO DE ESTO EMPIEZA UN JOURNAL;
		JOURNAL();
	}
}
