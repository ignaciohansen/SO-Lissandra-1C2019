/*
 * lfsComunicacion.c
 *
 *  Created on: 6 jul. 2019
 *      Author: martin
 */

#include "lfsComunicacion.h"



t_log *logger_com_lfs=NULL;
char *g_str_tam_valor = NULL;
int inicializado = 0;

void inicializar(t_log *logger, int tam_valor)
{
	char aux[100];
	logger_com_lfs = logger;
	snprintf(aux,99,"%d",tam_valor);
	g_str_tam_valor = malloc(strlen(aux)+1);
	strcpy(g_str_tam_valor,aux);
	inicializado = 1;
}

void finalizar_comunicacion(void)
{
	free(g_str_tam_valor);
}

void* hilo_servidor(int * socket_p){
	if(!inicializado){
		printf("\nEn lfsComunicacion. Se debe inicializar el módulo antes de arrancar\n");
		return NULL;
	}
	int socket_servidor = *socket_p;
	cliente_com_t cliente;
	pthread_t thread;
	imprimirMensaje(logger_com_lfs,"[SERVIDOR] Entrando a hilo de escucha del servidor del LFS");
	while(1){
		imprimirMensaje(logger_com_lfs,"[SERVIDOR] Esperando recibir un cliente");
		cliente = esperar_cliente(socket_servidor);
		imprimirMensaje(logger_com_lfs,"[SERVIDOR] Cliente intentando conectarse");
		switch(cliente.id){
			case MEMORIA:
				dar_bienvenida_cliente(cliente.socket, LFS, g_str_tam_valor);
				pthread_create(&thread,NULL,(void *)hilo_cliente, &cliente.socket );
				pthread_detach(thread);
				break;
			default:
				rechazar_cliente(cliente.socket,NULL);
				close(cliente.socket);
				break;
		}
	}
	return NULL;
}

void * hilo_cliente(int * socket_p)
{
	imprimirMensaje(logger_com_lfs,"[CLIENTE] Entrando a hilo de atención a un cliente");
	int socket_cliente = *socket_p;
	imprimirMensaje(logger_com_lfs,"[CLIENTE] Empiezo a esperar mensajes");
	msg_com_t msg;
	bool fin = false;
	req_com_t request;
	int aux_enviado_ok;
	while(fin == false){
		msg = recibir_mensaje(socket_cliente);
		imprimirMensaje(logger_com_lfs,"[CLIENTE] Recibí un mensaje");

		switch(msg.tipo){
			case REQUEST:

				imprimirMensaje(logger_com_lfs,"[CLIENTE] El mensaje recibido es un request");
				request = procesar_request(msg);
				borrar_mensaje(msg);
				request_t requestParser = parser(request.str);
				//resolver_pedido(requestParser);

				/* Acá tienen en request.str el request recibido
				 * Deben resolverlo de alguna forma con las funciones que tienen
				 * Probablemente puedan usar la función validarComando(...) pero con algún cambio porque es necesario saber la respuesta
				*/

				//Luego liberan la memoria de la estructura request
				borrar_request_com(request); //Con esto alcanza
				borrar_request(requestParser);

				/* Ahora tienen que enviar la respuesta al cliente que la pidió
				 *
				 * ej en caso de un select que pueden resolver ok (recuerden que la respuesta es del tipo VALOR|TIMESTAMP)
				 * aux_enviado_ok = responder_request(socket_cliente,"HOLA|1202",RESP_OK);
				 *
				 * ej en caso de no tener la tabla
				 * aux_enviado_ok = responder_request(socket_cliente,NULL,RESP_ERROR_TABLA_NO_EXISTE);
				*/

				//En cualquiera de los casos, deberían consultar el valor de retorno de la función, para validar que el mensaje se haya mandado ok
				if(aux_enviado_ok != -1) {
					imprimirMensaje(logger_com_lfs,"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
				}
				else {
					imprimirError(logger_com_lfs,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				break;

			case DESCONECTADO:
				imprimirMensaje(logger_com_lfs,"[CLIENTE] El cliente se desconectó");
				borrar_mensaje(msg);
				if(socket_cliente != -1)
					close(socket_cliente);
				fin = true;
				break;
			default:
				imprimirAviso(logger_com_lfs,"[CLIENTE] El tipo de mensaje no está permitido para este cliente o no lo reconozco");
				borrar_mensaje(msg);
				break;
		}
	}
	imprimirMensaje(logger_com_lfs,"[CLIENTE] Finalizando el hilo");
	return NULL;
}
/*
resp_com_t resolver_pedido(request_t req){
	{
		resp_com_t respuesta;
	//	char *ret_val=NULL;
	//	char *ret_ok_generico = malloc(3);
	//	strcpy(ret_ok_generico,"OK");
		switch(req.command){
			case INSERT:
				imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] Voy a resolver INSERT");
				respuesta = resolver_insert(req,true);
				if( respuesta.tipo == RESP_OK){
					imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
				}
				else{
					imprimirError(logger,"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
				}
				break;
			case SELECT:
				imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] Voy a resolver SELECT");
				//respuesta = resolver_select(socket_lfs,req);
				if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
					imprimirMensaje1(logger,"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",respuesta.msg.str);
				}
				else{
					imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
				}
				break;
			case DESCRIBE:
				imprimirAviso(logger,"[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE");
				//respuesta = resolver_describe(socket_lfs,req);
				if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
					imprimirMensaje1(logger,"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",respuesta.msg.str);
				}
				else{
					imprimirError(logger,"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
				}
				break;
			case DROP:
				imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] Voy a resolver DROP");
				//respuesta = resolver_drop(socket_lfs,req);
				if(respuesta.tipo == RESP_OK){
					imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
				}
				else{
					imprimirError(logger,"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
				}
				break;
			case CREATE:
				imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] Voy a resolver CREATE\n\n");
				//respuesta = resolver_create(socket_lfs,req);
				if(respuesta.tipo == RESP_OK){
					imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
				}
				else{
					imprimirError(logger,"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");
				}
				break;

			default:
				respuesta = armar_respuesta(RESP_ERROR_PEDIDO_DESCONOCIDO,NULL);
				break;
		}

		//fprintf(tablas_fp,"\nEjecutado comando %s",req.request_str);
		//loggearEstadoActual(tablas_fp);
		return respuesta;
	}
}

resp_com_t resolver_insert(request_t req, int modif)
{
	imprimirMensaje(logger,"[RESOLVIENDO INSERT] Voy a resolver INSERT");
	double timestamp_val;
	if(req.cant_args == 3)
		 timestamp_val = -1;
	else if(req.cant_args == 4)
		timestamp_val = atof(req.args[3]);
	else{
		imprimirError(logger,"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	char *valor = req.args[2];
	imprimirMensaje3(logger,"[RESOLVIENDO INSERT] Voy a agregar %s en la key %d de la tabla %s",valor,key,nombre_tabla);
	if(timestamp_val == -1){
		comandoInsertSinTimestamp(nombre_tabla,key,valor);
	}else{
		comandoInsert(nombre_tabla,key,valor,timestamp);
	}
	if(funcionInsert(nombre_tabla, key, valor, modif, timestamp_val)== -1){
		imprimirError(logger, "[RESOLVIENDO INSERT]ERROR: Mayor al pasar max value");
		return armar_respuesta(RESP_ERROR_MAYOR_MAX_VALUE,NULL);
	}
	return armar_respuesta(RESP_OK,NULL);
}
*/

