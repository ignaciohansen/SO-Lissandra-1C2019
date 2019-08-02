/*
 * lfsComunicacion.c
 *
 *  Created on: 6 jul. 2019
 *      Author: martin
 */

#include "lfsComunicacion.h"

t_log *logger_com_lfs = NULL;
char *g_str_tam_valor = NULL;
int inicializado = 0;

//CLIENTES @Nacho
t_list *clientes_activos;
pthread_mutex_t mutex_clientes_activos = PTHREAD_MUTEX_INITIALIZER;

void inicializar_comunicacion(t_log *logger, int tam_valor) {
	char aux[100];
	logger_com_lfs = logger;
	snprintf(aux, 99, "%d", tam_valor);
	g_str_tam_valor = malloc(strlen(aux) + 1);
	strcpy(g_str_tam_valor, aux);
	inicializado = 1;
}

void finalizar_comunicacion(void) {
	free(g_str_tam_valor);
}

void cliente_dar_de_alta(int socket)
{
	int *copy = malloc(sizeof(int)); //NO HACER UN FREE ACÁ SINO VA A ROMPER, LO HAGO MÁS ADELANTE
	//log_info(logger,"[CLIENTE] Dando de alta cliente en socket %d",socket);
	memcpy(copy,&socket,sizeof(int));
	pthread_mutex_lock(&mutex_clientes_activos);
	list_add(clientes_activos,copy);
	int activos = list_size(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
	//log_info(logger,"[CLIENTE] Socket cliente %d agregado a la lista de activos",socket);
	log_info(logger,"[CLIENTE] Hay %d clientes activos",activos);
}

void cliente_dar_de_baja(int socket)
{
	int *aux;
	bool encontrado = false;
	//log_info(logger,"[CLIENTE] Voy a dar de baja socket %d",socket);
	pthread_mutex_lock(&mutex_clientes_activos);
	for(int i=0;i<list_size(clientes_activos);i++){
		aux = list_get(clientes_activos,i);
		if(*aux == socket){
			list_remove(clientes_activos,i);
			free(aux);
			//log_info(logger,"[CLIENTE] Socket cliente %d sacado de la lista de activos",socket);
			encontrado = true;
			break;
		}
	}
	int activos = list_size(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
	if(encontrado == false)
		log_info(logger,"[CLIENTE] Socket cliente %d no se dió de baja porque no se encontró en la lista");
	log_info(logger,"[CLIENTE] Hay %d clientes activos",activos);
}

void cerrar_todos_clientes(void)
{
	pthread_mutex_lock(&mutex_clientes_activos);
	log_info(logger,"[CLIENTE] Cerrando sockets de clientes activos. Hay %d",list_size(clientes_activos));
	int *aux;
	for(int i=0;i<list_size(clientes_activos);i++){
		aux = list_get(clientes_activos, i);
		if(*aux != -1){
			log_info(logger,"[CLIENTE] Cerrando socket %d",*aux);
			close(*aux);
		}
		free(aux);
	}
	list_destroy(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
	log_info(logger,"[CLIENTE] Todos los sockets de clientes fueron cerrados");
}


void* hilo_servidor(int * socket_p) {
	if (!inicializado) {
		printf("\nEn lfsComunicacion. Se debe inicializar el módulo antes de arrancar\n");
		return NULL;
	}
	int socket_servidor = *socket_p;
	cliente_com_t cliente;
	pthread_t thread;
	clientes_activos = list_create();
	imprimirMensaje(logger_com_lfs,"[SERVIDOR] Entrando a hilo de escucha del servidor del LFS");
	while (1) {
		imprimirMensaje(logger_com_lfs,
				"[SERVIDOR] Esperando recibir un cliente");
		cliente = esperar_cliente(socket_servidor);
		imprimirMensaje(logger_com_lfs,"[SERVIDOR] Cliente intentando conectarse");
		switch (cliente.id) {
		case MEMORIA:
			dar_bienvenida_cliente(cliente.socket, LFS, g_str_tam_valor);
			cliente_dar_de_alta(cliente.socket); //@NACHO
			int *socket = malloc(sizeof(int));
			*socket = cliente.socket;
			pthread_create(&thread, NULL, (void *) hilo_cliente,socket);
			pthread_detach(thread);
			break;
		default:
			rechazar_cliente(cliente.socket, NULL);
			close(cliente.socket);
			break;
		}
	}

	return NULL;
}

//void * hilo_cliente(int * socket_p) @NACHO
void * hilo_cliente(int *socket){
	//imprimirMensaje(logger_com_lfs,"[CLIENTE] Entrando a hilo de atención a un cliente");
	int socket_cliente = *socket;
	//imprimirMensaje(logger_com_lfs, "[CLIENTE] Empiezo a esperar mensajes");
	msg_com_t msg;
	bool fin = false;
	req_com_t request;
	resp_com_t respuesta;
	int aux_enviado_ok;
	while (fin == false) {
		msg = recibir_mensaje(socket_cliente);
		//imprimirMensaje(logger_com_lfs, "[CLIENTE] Recibí un mensaje");

		switch (msg.tipo) {
		case REQUEST:


			request = procesar_request(msg);
//log_info(logger_com_lfs,"[CLIENTE] El mensaje recibido es un request: %s ", request.str);
			borrar_mensaje(msg);
			request_t requestParser = parser(request.str);
			respuesta = resolver_pedido(requestParser);
			//printf("la respuesta a enviar es: %s",respuesta.msg.str);
			aux_enviado_ok = enviar_respuesta(socket_cliente, respuesta);

			borrar_respuesta(respuesta);

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
			if (aux_enviado_ok != -1) {
				imprimirMensaje(logger_com_lfs,
						"[CLIENTE] La respuesta fue enviada con éxito al cliente");
			} else {
				imprimirError(logger_com_lfs,
						"[CLIENTE] La respuesta no pudo ser enviada al cliente");
			}
			break;

		case DESCONECTADO:
			imprimirMensaje(logger_com_lfs,"[CLIENTE] El cliente se desconectó");
			borrar_mensaje(msg);
			fin = true;
			cliente_dar_de_baja(socket_cliente);
			if(socket_cliente != -1){
				close(socket_cliente);
			}
			break;
		default:
			imprimirAviso(logger_com_lfs,
					"[CLIENTE] El tipo de mensaje no está permitido para este cliente o no lo reconozco");
			borrar_mensaje(msg);
			break;
		}
	}
	free(socket);
	imprimirMensaje(logger_com_lfs, "[CLIENTE] Finalizando el hilo");
	return NULL;
}

resp_com_t resolver_pedido(request_t req) {
	resp_com_t respuesta;
	log_info(logger,"[RESOLVIENDO PEDIDO] Llego %s",req.request_str);

	switch (req.command) {
	case INSERT:
		respuesta = resolver_insert(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,	"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
		} else {
			imprimirError(logger,"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
		}
		break;
	case SELECT:
		respuesta = resolver_select(req);
		if (respuesta.tipo == RESP_OK && respuesta.msg.tam > 0) {
			imprimirMensaje1(logger,
					"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",
					respuesta.msg.str);
		} else {
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
		}
		break;
	case DESCRIBE:
		respuesta = resolver_describe(req);
		if (respuesta.tipo == RESP_OK && respuesta.msg.tam > 0) {
			imprimirMensaje1(logger,
					"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",
					respuesta.msg.str);
		} else {
			imprimirError(logger,
					"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
		}
		break;
	case DROP:
		respuesta = resolver_drop(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
		} else {
			imprimirError(logger,
					"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
		}
		break;
	case CREATE:
		respuesta = resolver_create(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
		} else {
			imprimirError(logger,
					"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");
		}
		break;

	default:
		respuesta = armar_respuesta(RESP_ERROR_PEDIDO_DESCONOCIDO, NULL);
		break;
	}

	//fprintf(tablas_fp,"\nEjecutado comando %s",req.request_str);
	//loggearEstadoActual(tablas_fp);
	return respuesta;
}

resp_com_t resolver_describe(request_t req) {
	char *ret_val;
	resp_com_t respuesta;

	if (req.cant_args == 1) {
		char *nombre_tabla = req.args[0];
		ret_val = comandoDescribeEspecifico(nombre_tabla);
		if (ret_val == NULL) {
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, ret_val);
		} else {
			respuesta = armar_respuesta(RESP_OK, ret_val);
			free(ret_val);
			return respuesta;
		}
	}

	else if (req.cant_args == 0) {
		ret_val = comandoDescribe();
		if(ret_val == NULL){
			return armar_respuesta(RESP_ERROR_NO_HAY_TABLAS, ret_val);
		}
		else{
			respuesta = armar_respuesta(RESP_OK, ret_val);
			free(ret_val);
			return respuesta;
		}

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_create(request_t req) {
	int ret_val;

	if (req.cant_args == 4) {
		char *nombre_tabla = req.args[0];
		char *consistencia = req.args[1];
		char *particiones = req.args[2];
		char *tiempoCompactacion = req.args[3];
		ret_val = comandoCreate(nombre_tabla, consistencia, particiones,tiempoCompactacion);
		if (ret_val == -2) {
			return armar_respuesta(RESP_ERROR_TABLA_YA_EXISTE, NULL);
		} else if (ret_val == -1) {
			return armar_respuesta(RESP_ERROR_METADATA, NULL);
		}
		return armar_respuesta(RESP_OK, NULL);

	}

	else {
		imprimirError(logger,"[RESOLVIENDO CREATE] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_drop(request_t req) {
	int ret_val;

	if (req.cant_args == 1) {
		char *nombre_tabla = req.args[0];

		ret_val = comandoDrop(nombre_tabla);
		if (ret_val == -1) {
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, NULL);
		}
		return armar_respuesta(RESP_OK, NULL);

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO DROP] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_select(request_t req) {
	t_registroMemtable* ret_val;
	resp_com_t respuesta;
//	imprimirMensaje(logger, "[RESOLVIENDO SELECT] Entro a función");

	if (req.cant_args == 2) {
		char *nombre_tabla = req.args[0];
		char *key = req.args[1];
		//string_to_upper(nombre_tabla);
		ret_val = comandoSelect(nombre_tabla,key);

		if (ret_val->tam_registro == -1) {
			borrarRegistro(ret_val);
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, NULL);
		} else if (ret_val->tam_registro == -2) {
			borrarRegistro(ret_val);
			return armar_respuesta(RESP_ERROR_METADATA, NULL);
		}else if(ret_val->timestamp == 0){
			borrarRegistro(ret_val);
			return armar_respuesta(RESP_ERROR_KEY_NO_EXISTE, NULL);
		}

		int tamanio = strlen(ret_val->value)+40;
		char* valueRetorno = malloc(tamanio); //@VALGRIND ESTO NO SE BORRA CON BORRAR RESPUESTA?
		snprintf(valueRetorno,tamanio, "%s|%llu", ret_val->value,ret_val->timestamp); // value|timestamp
		borrarRegistro(ret_val);
//		char* valueRta = malloc(strlen(valueRetorno) + 1); //Agrege esto
//		strcpy(valueRta, valueRetorno);	////Agrege esto
//		respuesta = armar_respuesta(RESP_OK, valueRta);
		respuesta = armar_respuesta(RESP_OK, valueRetorno);
		free(valueRetorno); // para descomentar este free
		return respuesta;

	}

	else {
		imprimirError(logger,"[RESOLVIENDO SELECT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_insert(request_t req) {
	int ret_val;
	//imprimirMensaje(logger, "[RESOLVIENDO INSERT] Entro a función");


	if (req.cant_args == 4) {
		char *nombre_tabla = req.args[0];
		char *key = req.args[1];
		char *value = req.args[2];
		char *timestamp = req.args[3];
		ret_val = comandoInsert(nombre_tabla, key, value,timestamp);


	} else if(req.cant_args == 3){
		char *nombre_tabla = req.args[0];
		char *key = req.args[1];
		char *value = req.args[2];
		ret_val = comandoInsertSinTimestamp(nombre_tabla,key,value);

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

	if (ret_val == -1) {
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, NULL);
	}
		return armar_respuesta(RESP_OK, NULL);

}

