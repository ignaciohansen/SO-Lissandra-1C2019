/*
 * lfsComunicacion.h
 *
 *  Created on: 6 jul. 2019
 *      Author: martin
 */

#ifndef LFSCOMUNICACION_H_
#define LFSCOMUNICACION_H_

#include "../Biblioteca/src/Biblioteca.h"

#include "parser.h"

void inicializar_comunicacion(t_log *logger,int tam_valor);
void* hilo_servidor(int * socket_p);
void * hilo_cliente(int * socket_p);
void finalizar_comunicacion(void);



#endif /* LFSCOMUNICACION_H_ */
