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
#include "LissandraFileSystem.h"

void inicializar_comunicacion(t_log *logger,int tam_valor);
void* hilo_servidor(int * socket_p);
void * hilo_cliente(int * socket_p);
void finalizar_comunicacion(void);
resp_com_t resolver_describe(request_t req);
resp_com_t resolver_create(request_t req);
resp_com_t resolver_drop(request_t req);
resp_com_t resolver_select(request_t req);
resp_com_t resolver_insert(request_t req);
resp_com_t resolver_pedido(request_t req);



#endif /* LFSCOMUNICACION_H_ */
