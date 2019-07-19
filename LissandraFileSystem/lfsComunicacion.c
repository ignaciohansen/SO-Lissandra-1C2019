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

void* hilo_servidor(int * socket_p) {
	if (!inicializado) {
		printf(
				"\nEn lfsComunicacion. Se debe inicializar el módulo antes de arrancar\n");
		return NULL;
	}
	int socket_servidor = *socket_p;
	cliente_com_t cliente;
	pthread_t thread;
	imprimirMensaje(logger_com_lfs,
			"[SERVIDOR] Entrando a hilo de escucha del servidor del LFS");
	while (1) {
		imprimirMensaje(logger_com_lfs,
				"[SERVIDOR] Esperando recibir un cliente");
		cliente = esperar_cliente(socket_servidor);
		imprimirMensaje(logger_com_lfs,
				"[SERVIDOR] Cliente intentando conectarse");
		switch (cliente.id) {
		case MEMORIA:
			dar_bienvenida_cliente(cliente.socket, LFS, g_str_tam_valor);
			pthread_create(&thread, NULL, (void *) hilo_cliente,
					&cliente.socket);
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

void * hilo_cliente(int * socket_p) {
	imprimirMensaje(logger_com_lfs,
			"[CLIENTE] Entrando a hilo de atención a un cliente");
	int socket_cliente = *socket_p;
	imprimirMensaje(logger_com_lfs, "[CLIENTE] Empiezo a esperar mensajes");
	msg_com_t msg;
	bool fin = false;
	req_com_t request;
	resp_com_t respuesta;
	int aux_enviado_ok;
	while (fin == false) {
		msg = recibir_mensaje(socket_cliente);
		imprimirMensaje(logger_com_lfs, "[CLIENTE] Recibí un mensaje");

		switch (msg.tipo) {
		case REQUEST:

			imprimirMensaje(logger_com_lfs,
					"[CLIENTE] El mensaje recibido es un request");
			request = procesar_request(msg);
			borrar_mensaje(msg);
			request_t requestParser = parser(request.str);
			respuesta = resolver_pedido(requestParser);
			//printf("la respuesta a enviar es: %s",respuesta.msg.str);
			aux_enviado_ok = enviar_respuesta(socket_cliente, respuesta);

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
						"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
			} else {
				imprimirError(logger_com_lfs,
						"[CLIENTE] La respuesta no pudo ser enviada al cliente");
			}
			break;

		case DESCONECTADO:
			imprimirMensaje(logger_com_lfs,
					"[CLIENTE] El cliente se desconectó");
			borrar_mensaje(msg);
			if (socket_cliente != -1)
				close(socket_cliente);
			fin = true;
			break;
		default:
			imprimirAviso(logger_com_lfs,
					"[CLIENTE] El tipo de mensaje no está permitido para este cliente o no lo reconozco");
			borrar_mensaje(msg);
			break;
		}
	}
	imprimirMensaje(logger_com_lfs, "[CLIENTE] Finalizando el hilo");
	return NULL;
}

resp_com_t resolver_pedido(request_t req) {
	{
		resp_com_t respuesta;

		switch (req.command) {
		case INSERT:
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] Voy a resolver INSERT");
			respuesta = resolver_insert(req);
			if (respuesta.tipo == RESP_OK) {
				imprimirMensaje(logger,
						"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
			} else {
				imprimirError(logger,
						"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
			}
			break;
		case SELECT:
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] Voy a resolver SELECT");
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
			imprimirAviso(logger,
					"[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE");
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
			imprimirMensaje(logger, "[RESOLVIENDO PEDIDO] Voy a resolver DROP");
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
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] Voy a resolver CREATE\n\n");
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
}

resp_com_t resolver_describe(request_t req) {
	char *ret_val;
	imprimirMensaje(logger, "[RESOLVIENDO DESCRIBE] Entro a función");

	if (req.cant_args == 1) {
		char *nombre_tabla = req.args[0];
		ret_val = comandoDescribeEspecifico(nombre_tabla);
		if (ret_val == NULL) {
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, ret_val);
		} else {
			return armar_respuesta(RESP_OK, ret_val);
		}
	}

	else if (req.cant_args == 0) {
		ret_val = comandoDescribe();

		return armar_respuesta(RESP_OK, ret_val);

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_create(request_t req) {
	int ret_val;
	imprimirMensaje(logger, "[RESOLVIENDO CREATE] Entro a función");

	if (req.cant_args == 4) {
		char *nombre_tabla = req.args[0];
		char *consistencia = req.args[1];
		char *particiones = req.args[2];
		char *tiempoCompactacion = req.args[3];
		ret_val = comandoCreate(nombre_tabla, consistencia, particiones,
				tiempoCompactacion);
		if (ret_val == -2) {
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, NULL);
		} else if (ret_val == -1) {

			return armar_respuesta(RESP_ERROR_METADATA, NULL);

		}
		return armar_respuesta(RESP_OK, NULL);

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO CREATE] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_drop(request_t req) {
	int ret_val;
	imprimirMensaje(logger, "[RESOLVIENDO DROP] Entro a función");

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
	imprimirMensaje(logger, "[RESOLVIENDO SELECT] Entro a función");

	if (req.cant_args == 2) {
		char *nombre_tabla = req.args[0];
		char *key = req.args[1];
		//string_to_upper(nombre_tabla);
		ret_val = comandoSelect(nombre_tabla,key);

		if (ret_val->value == NULL) {
			borrarRegistro(ret_val);
			return armar_respuesta(RESP_ERROR_TABLA_NO_EXISTE, NULL);
		} else if (ret_val->tam_registro == -2) {
			free(ret_val);
			return armar_respuesta(RESP_ERROR_METADATA, NULL);
		}
		int tamanio = strlen(ret_val->value)+40;
		char* valueRetorno = malloc(tamanio);
		snprintf(valueRetorno,tamanio, "%s|%llu", ret_val->value,ret_val->timestamp); // value|timestamp
		return armar_respuesta(RESP_OK, valueRetorno);

	}

	else {
		imprimirError(logger,
				"[RESOLVIENDO SELECT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}

}

resp_com_t resolver_insert(request_t req) {
	int ret_val;
	imprimirMensaje(logger, "[RESOLVIENDO INSERT] Entro a función");


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

