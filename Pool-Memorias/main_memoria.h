/*
 * main_memoria.h
 *
 *  Created on: 6 jul. 2019
 *      Author: martin
 */

#ifndef MAIN_MEMORIA_H_
#define MAIN_MEMORIA_H_

//#define LOGGEAR_EN_CONSOLA

#include "parser.h"
#include "memoria.h"
#include "gestionMemoria.h"
#include "Inotify.h"

#define MAX_LONG_INSERT 100

typedef struct{
	int socket_cliente;
	bool requiere_lfs;
}hilo_cliente_args_t;

int conectar_a_lfs(bool inicializando, int *tam_pagina);
int levantar_servidor_memoria(void);
void* hilo_consola(int * socket_p);
resp_com_t resolver_pedido(request_t req, int socket_lfs);
resp_com_t resolver_select(int socket_lfs,request_t req);
resp_com_t resolver_insert(request_t req, int modif);
resp_com_t resolver_describe(int socket_lfs,request_t req);
resp_com_t resolver_create(int socket_lfs,request_t req);
resp_com_t resolver_drop(int socket_lfs,request_t req);
resp_com_t resolver_journal(int socket_lfs,request_t req);
void* hilo_servidor(int * socket_p);
void * hilo_cliente(hilo_cliente_args_t *args);
int responder_request(int socket,char *msg, resp_tipo_com_t tipo_resp);
int inicializar_gossiping_memoria(void);
int responder_gossiping(gos_com_t recibido,id_com_t id_proceso,int socket);
void cliente_dar_de_alta(int socket);
void cliente_dar_de_baja(int socket);
void cerrar_todos_clientes(void);
void  INThandler(int sig);

char *armar_insert(char *respuesta_select, char *tab, int key);

#endif /* MAIN_MEMORIA_H_ */
