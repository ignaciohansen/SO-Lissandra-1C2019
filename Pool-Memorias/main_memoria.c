/*
 * main_memoria.c
 *
 *  Created on: 15 jun. 2019
 *      Author: martin
 */

#define COMPILAR_MAIN_MEMORIA
#ifdef COMPILAR_MAIN_MEMORIA

#include "main_memoria.h"

//#define AUTOLANZAR_SERVER
//#define AUTOLANZAR_CLIENTE
#define EJECUTAR_GOSSIPING
#define TESTEAR_GOSSIPING

int main(int argc, char **argv)
{
	// LOGGING
	printf("INICIANDO EL MODULO MEMORIA \n COMINEZA EL TP PIBE\n\n");

#ifdef TESTEAR_GOSSIPING
	if(argc>1)
		inicioLogYConfig(argv[1]);
	else
		inicioLogYConfig(PATH_MEMORIA_CONFIG);
#else
	inicioLogYConfig(PATH_MEMORIA_CONFIG);
#endif

#ifdef AUTOLANZAR_SERVER
	char cmd_server[100];
	snprintf(cmd_server,99,"gnome-terminal -- ./servidor %s %d",arc_config->ip_fs,arc_config->puerto_fs);
	system(cmd_server);
#endif

	int socket_lfs = conectar_a_lfs(true,&max_valor_key);

	if(socket_lfs == -1 || max_valor_key <= 0){
		imprimirError(log_memoria,"[MAIN] Error al conectar al LFS, terminando!\n");
		return 1;
	}
	arc_config->max_value_key = max_valor_key;
	imprimirMensaje1(log_memoria,"[MAIN] Iniciando con tamaño máximo de valor %d",arc_config->max_value_key);

	armarMemoriaPrincipal();

	iniciarSemaforosYMutex();

	int socket_servidor = levantar_servidor_memoria();

#ifdef AUTOLANZAR_CLIENTE
	char cmd_cliente[100];
	snprintf(cmd_cliente,99,"gnome-terminal -- ./cliente %s %d",arc_config->ip,arc_config->puerto);
	system(cmd_cliente);
#endif

	pthread_t servidor_h, consola_h, gossiping_h, inotify_c;

#ifdef EJECUTAR_GOSSIPING
	id_com_t mi_id = MEMORIA;
	inicializar_gossiping_memoria();
	iniciar_hilo_gossiping(&mi_id,&gossiping_h);
#endif

	pthread_create(&servidor_h,NULL,(void *)hilo_servidor,&socket_servidor);
	pthread_detach(servidor_h);

	pthread_create(&consola_h,NULL,(void *)hilo_consola,&socket_lfs);

	char* path_de_memoria = malloc(strlen(PATH_MEMORIA_CONFIG)+1);
	strcpy(path_de_memoria, PATH_MEMORIA_CONFIG);

	pthread_create(&inotify_c,NULL, (void *)inotifyAutomatico,path_de_memoria);
	pthread_detach(inotify_c);


//pthread_detach(journalHilo);
	pthread_join(consola_h,NULL);

	//ESTO ESTA MAL, PERO QUIERO VER SI FUNCA LO MIO
//	pthread_cancel(servidor_h, SIGKILL);
//	pthread_cancel(gossiping_h, SIGKILL);

	liberar_todo_por_cierre_de_modulo();

#ifdef EJECUTAR_GOSSIPING
	liberar_memoria_gossiping();
#endif
	return 1;
}

int levantar_servidor_memoria(void)
{
	char puerto[20];
	snprintf(puerto,19,"%d",arc_config->puerto);
	imprimirMensaje2(log_memoria,"[LEVANTANDO SERVIDOR] Me voy a intentar conectar en ip: <%s> puerto: <%s>", arc_config->ip, puerto);

	int socket = iniciar_servidor(arc_config->ip,puerto);

	if(socket == -1){
		imprimirError(log_memoria,"[LEVANTANDO SERVIDOR] Error al levantar el servidor. Por favor, reintente");
	}
	else
		imprimirMensaje(log_memoria,"[LEVANTANDO SERVIDOR] Servidor conectado");
	return socket;
}

int conectar_a_lfs(bool inicializando, int *tam_valor)
{
	id_com_t memoria = MEMORIA;
	char puerto_fs[20];
	snprintf(puerto_fs,19,"%d",arc_config->puerto_fs);
	imprimirMensaje2(log_memoria,"[CONECTANDO A LFS] Me voy a intentar conectar a ip: <%s> puerto: <%s>", arc_config->ip_fs, puerto_fs);
	int socket = conectar_a_servidor(arc_config->ip_fs,puerto_fs,memoria);
	if(socket == -1){
		imprimirError(log_memoria,"[CONECTANDO A LFS] No fue posible conectarse con lissandra. TERMINANDO\n");
		return -1;
	}

	imprimirMensaje(log_memoria,"[CONECTANDO A LFS] Me conecté con éxito al lfs. Espero su hs");
	//Si me conecté, espero su msg de bienvenida

	msg_com_t msg = recibir_mensaje(socket);

	if(msg.tipo != HANDSHAKECOMANDO){
		borrar_mensaje(msg);
		imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs no responde el hs. TERMINANDO\n");
		return -1;
	}

	handshake_com_t hs = procesar_handshake(msg);
	borrar_mensaje(msg);

	if(hs.id == RECHAZADO){
		if(hs.msg.tam == 0 )
			imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión. TERMINANDO\n");
		else
			imprimirError1(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión [%s]. TERMINANDO\n",hs.msg.str);
		borrar_handshake(hs);
		close(socket);
		return -1;
	}
	if(inicializando){
		if(hs.msg.tam == 0){
			imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs no mandó el tamaño de los valores. TERMINANDO\n");
			borrar_handshake(hs);
			close(socket);
			return -1;
		}
		else{
			imprimirMensaje1(log_memoria,"[CONECTANDO A LFS] Recibido el máximo tamaño para los valores. Es %s",hs.msg.str);
			*tam_valor = atoi(hs.msg.str);
		}
	}
	borrar_handshake(hs);
	imprimirMensaje(log_memoria,"[CONECTANDO A LFS] Me conecté con éxito al lfs");

	return socket;
}

void* hilo_consola(int * socket_p){
	request_t req;
	int socket_lfs = *socket_p;
	char *linea_leida;
	resp_com_t respuesta;
	int fin = 0;
	imprimirMensaje(log_memoria,"[CONSOLA] Entrando a hilo consola");
	using_history();
	imprimirPorPantallaTodosLosComandosDisponibles();
	pthread_create(&journalHilo, NULL, retardo_journal, arc_config->retardo_journal);
	pthread_detach(journalHilo);
	while(!fin){
		linea_leida=readline("\n>");
		if(linea_leida)
			add_history(linea_leida);
		req = parser(linea_leida);
		free(linea_leida);
		switch(req.command){
			case SALIR:
				fin = 1;
				break;
			case SELECT:
			case INSERT:
			case DESCRIBE:
			case CREATE:
			case DROP:
			case JOURNALCOMANDO:
				respuesta = resolver_pedido(req,socket_lfs);
				if(respuesta.tipo == RESP_OK)
					imprimirMensaje1(log_memoria,"\n[CONSOLA] Resuelto OK. Respuesta obtenida: %s",respuesta.msg.str);
				else
					imprimirAviso1(log_memoria,"\n[CONSOLA] Error al resolver pedido. Respuesta %s",respuesta.msg.str);
				borrar_respuesta(respuesta);
				break;
			default:
				printf("\nNO IMPLEMENTADO\n");
				break;
		}
		borrar_request(req);
	}
	clear_history();
	printf("\nSaliendo de hilo consola\n");
	return NULL;
}

void* hilo_servidor(int * socket_p){
	int socket = *socket_p;
	cliente_com_t cliente;
	pthread_t thread;
	imprimirMensaje(log_memoria,"[SERVIDOR] Entrando a hilo de escucha del servidor de la memoria");
	while(1){
		imprimirMensaje(log_memoria,"[SERVIDOR] Esperando recibir un cliente");
		cliente = esperar_cliente(socket);
		imprimirMensaje(log_memoria,"[SERVIDOR] Cliente intentando conectarse");
		switch(cliente.id){
			hilo_cliente_args_t args;
			case MEMORIA:
				args.socket_cliente = cliente.socket;
				args.requiere_lfs = false;
				dar_bienvenida_cliente(cliente.socket, MEMORIA, "Bienvenido!");
				pthread_create(&thread,NULL,(void *)hilo_cliente, &args );
				pthread_detach(thread);
				break;
			case KERNEL:
				args.socket_cliente = cliente.socket;
				args.requiere_lfs = true;
				dar_bienvenida_cliente(cliente.socket, MEMORIA, "Bienvenido!");
				pthread_create(&thread,NULL,(void *)hilo_cliente, &args );
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

void * hilo_cliente(hilo_cliente_args_t *args)
{
	imprimirMensaje(log_memoria,"[CLIENTE] Entrando a hilo de atención a un cliente");
	int socket_cliente = args->socket_cliente;
	int socket_lfs = -1;
	if(args->requiere_lfs){
		socket_lfs = conectar_a_lfs(false,NULL);
		if(socket_lfs == -1){
			imprimirError(log_memoria,"[CLIENTE] No pude conectarme al lfs");
		}
		else{
			imprimirMensaje(log_memoria,"[CLIENTE] Ya tengo un canal para comunicarme con el lfs");
		}
	}
	imprimirMensaje(log_memoria,"[CLIENTE] Empiezo a esperar mensajes");
	msg_com_t msg;
	bool fin = false;
	while(fin == false){
		msg = recibir_mensaje(socket_cliente);
		imprimirMensaje(log_memoria,"[CLIENTE] Recibí un mensaje");
		req_com_t request;
		request_t req_parseado;
		gos_com_t gossip;
		resp_com_t respuesta;
		switch(msg.tipo){
			case REQUEST:
				imprimirMensaje(log_memoria,"[CLIENTE] El mensaje recibido es un request");
				request = procesar_request(msg);
				borrar_mensaje(msg);
				req_parseado = parser(request.str);
				borrar_request_com(request);
				respuesta = resolver_pedido(req_parseado,socket_lfs);
				if(respuesta.tipo == RESP_OK)
					imprimirMensaje1(log_memoria,"[CLIENTE] Pedido resuelto OK. La resupuesta obtenida para el pedido es %s",respuesta.msg.str);
				else
					imprimirMensaje1(log_memoria,"[CLIENTE] Pedido no pudo ser resuelto. La resupuesta obtenida para el pedido es %s",respuesta.msg.str);
				if(enviar_respuesta(socket_cliente,respuesta) != -1) {
					imprimirMensaje(log_memoria,"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
				}
				else {
					imprimirError(log_memoria,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				borrar_respuesta(respuesta);
				break;
			case GOSSIPING:
				imprimirMensaje(log_memoria,"[CLIENTE] El mensaje recibido es un pedido de gossiping");
				gossip = procesar_gossiping(msg);
				borrar_mensaje(msg);
				if(responder_gossiping(gossip,MEMORIA,socket_cliente) != -1) {
					imprimirMensaje(log_memoria,"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
				}
				else {
					imprimirError(log_memoria,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				break;
			case DESCONECTADO:
				imprimirMensaje(log_memoria,"[CLIENTE] El cliente se desconectó");
				borrar_mensaje(msg);
				//close(socket_cliente);
				if(socket_lfs != -1)
					close(socket_lfs);
				fin = true;
				if(socket_cliente != -1)
					close(socket_cliente);
				break;
			default:
				imprimirAviso(log_memoria,"[CLIENTE] El tipo de mensaje no está permitido para este cliente");
				borrar_mensaje(msg);
				break;
		}
	}
	imprimirMensaje(log_memoria,"[CLIENTE] Finalizando el hilo");
	return NULL;
}

int responder_gossiping(gos_com_t recibido,id_com_t id_proceso,int socket)
{
#ifdef EJECUTAR_GOSSIPING
	gos_com_t conocidas = armar_vector_seeds(id_proceso);
	incorporar_seeds_gossiping(recibido);
	if(enviar_gossiping(socket,conocidas) == -1){
		borrar_gossiping(conocidas);
		return -1;
	}
	borrar_gossiping(conocidas);
#endif
	return 1;
}

resp_com_t resolver_pedido(request_t req, int socket_lfs)
{
	resp_com_t respuesta;
//	char *ret_val=NULL;
//	char *ret_ok_generico = malloc(3);
//	strcpy(ret_ok_generico,"OK");
	switch(req.command){
		case INSERT:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver INSERT");
			respuesta = resolver_insert(req,true);
			if( respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
			}
			break;
		case SELECT:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver SELECT");
			respuesta = resolver_select(socket_lfs,req);
			if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",respuesta.msg.str);
			}
			else{
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
			}
			break;
		case DESCRIBE:
			imprimirAviso(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE");
			respuesta = resolver_describe(socket_lfs,req);
			if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",respuesta.msg.str);
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
			}
			break;
		case DROP:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver DROP");
			respuesta = resolver_drop(socket_lfs,req);
			if(respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
			}
			break;
		case CREATE:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver CREATE\n\n");
			respuesta = resolver_create(socket_lfs,req);
			if(respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");
			}
			break;
		case JOURNALCOMANDO:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver JOURNAL");
			respuesta = resolver_journal(socket_lfs,req);
			if( respuesta.tipo == RESP_OK ){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] JOURNAL hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El JOURNAL no pudo realizarse");
			}
			break;

		default:
			respuesta = armar_respuesta(RESP_ERROR_PEDIDO_DESCONOCIDO,NULL);
			break;
	}

	fprintf(tablas_fp,"\nEjecutado comando %s",req.request_str);
	loggearEstadoActual(tablas_fp);
	return respuesta;
}


resp_com_t resolver_drop(int socket_lfs,request_t req)
{
	resp_com_t respuesta;
	imprimirMensaje(log_memoria,"[RESOLVIENDO DROP] Voy a resolver DROP");
	if(req.cant_args != 1){
		imprimirError(log_memoria,"[RESOLVIENDO DROP] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	char *nombre_tabla = malloc(strlen(req.args[0])+1);
	strcpy(nombre_tabla,req.args[0]);
	retardo_memoria();
	if(funcionDrop(nombre_tabla)==-1){
		imprimirAviso1(log_memoria, "[RESOLVIENDO DROP] La tabla ya fue eliminada o no existe en memoria: <%s>", nombre_tabla);
	}

	//Le envio el DROP al filesystem
	req_com_t enviar;
	enviar.tam = strlen("DROP ") + strlen(nombre_tabla) + 1;
	enviar.str = malloc(enviar.tam);
	strcpy(enviar.str,"DROP ");
	strcat(enviar.str,nombre_tabla);
	free(nombre_tabla);
	retardo_fs();
	imprimirMensaje(log_memoria, "[RESOLVIENDO DROP] Voy a enviar el drop al filesystem");
	if(enviar_request(socket_lfs,enviar) == -1){
		imprimirError(log_memoria, "[RESOLVIENDO DROP] No se puedo enviar el drop al filesystem");
		borrar_request_com(enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(enviar);
	//Espero su respuesta
	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo == RESPUESTA){
		respuesta = procesar_respuesta(msg);
		borrar_mensaje(msg);
		if(respuesta.tipo == RESP_OK){
			imprimirMensaje(log_memoria, "[RESOLVIENDO DROP] El filesystem realizó el DROP con éxito");
		}
		else{
			imprimirError(log_memoria, "[RESOLVIENDO DROP] El filesystem no pudo realizar el DROP con éxito.");
//			borrar_respuesta(recibido);
//			return -1;
		}
		if(respuesta.msg.tam >0)
			imprimirMensaje1(log_memoria,"[RESOLVIENDO DROP] El filesystem contestó al DROP con %s",respuesta.msg.str);
//		borrar_respuesta(recibido);
	}
	else{
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	return respuesta;
}

resp_com_t resolver_create(int socket_lfs,request_t req)
{
	if(req.cant_args < 1){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}

	//Le envio el CREATE al filesystem
	req_com_t a_enviar;

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);
	retardo_fs();
	imprimirMensaje1(log_memoria, "[RESOLVIENDO CREATE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}

	imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] La respuesta vino vacía");
//			ret_val = NULL;
		}
		else{
			imprimirMensaje1(log_memoria, "[RESOLVIENDO CREATE] La respuesta del lfs fue: %s",resp.msg.str);
//			ret_val = malloc(resp.msg.tam);
//			strcpy(ret_val,resp.msg.str);
		}
//		borrar_respuesta(resp);
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[RESOLVIENDO CREATE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
//		borrar_respuesta(resp);
//		return NULL;
	}
	return resp;
}

resp_com_t resolver_journal(int socket_lfs,request_t req)
{

//	//Consigo toda la info modificada de memoria
//	datosJournal *datos_modificados = obtener_todos_journal();
//	datosJournal *aux = datos_modificados;
//
//	imprimirMensaje(log_memoria, "[RESOLVIENDO JOURNAL] DATOS MODIFICADOS: ");
//	while(aux!=NULL){
//		imprimirAviso3(log_memoria, "%s %d %s", aux->nombreTabla, aux->key, aux->value);
//		aux = aux->sig;
//	}
	char *msg_resp = malloc(100);
	resp_com_t resp;
	int cant_pasados = procesoJournal(socket_lfs);

	if(cant_pasados != -1){
		snprintf(msg_resp, 99, "Journal hecho. %d registros recibidos OK",cant_pasados);
		resp = armar_respuesta(RESP_OK, msg_resp);
	}
	else{
		snprintf(msg_resp, 99, "El journal no fue realizado correctamente");
		resp = armar_respuesta(RESP_ERROR_COMUNICACION, msg_resp);
	}
	free(msg_resp);

	return resp;
}

resp_com_t resolver_insert(request_t req, int modif)
{
	imprimirMensaje(log_memoria,"[RESOLVIENDO INSERT] Voy a resolver INSERT");
	timestamp_mem_t timestamp_val;
	if(req.cant_args == 3)
		 timestamp_val = 0;
	else if(req.cant_args == 4)
		timestamp_val = atoi(req.args[3]);
	else{
		imprimirError(log_memoria,"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	char *valor = req.args[2];
	retardo_memoria();
	imprimirMensaje3(log_memoria,"[RESOLVIENDO INSERT] Voy a agregar %s en la key %d de la tabla %s",valor,key,nombre_tabla);
	if(funcionInsert(nombre_tabla, key, valor, modif, timestamp_val)== -1){
		imprimirError(log_memoria, "[RESOLVIENDO INSERT]ERROR: Mayor al pasar max value");
		return armar_respuesta(RESP_ERROR_MAYOR_MAX_VALUE,NULL);
	}
	return armar_respuesta(RESP_OK,NULL);
}

char* select_memoria(char *nombre_tabla, uint16_t key)
{
	pagina_a_devolver* pagina = malloc(sizeof(pagina_a_devolver));

	imprimirMensaje2(log_memoria,"[WRAPPER DE SELECT] Quiero obtener la key %d de la tabla %s",key,nombre_tabla);
	segmento *seg;
	int pag;
	bool encontrada = false;
	char* valorAux = malloc(max_valor_key);
	void* informacion = malloc(sizeof(pagina)+max_valor_key);
	pagina->value = malloc(max_valor_key);

	if(funcionSelect(nombre_tabla, key, &pagina, &valorAux)!=0){
		pag =
			buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombre_tabla, key, &seg);
		free(pagina->value);
		free(pagina);
		pagina = selectPaginaPorPosicion(pag,informacion);
		imprimirMensaje1(log_memoria,"[WRAPPER DE SELECT] Valor encontrado: %s",pagina->value);
		//printf("\nSEGMENTO <%s>\nKEY<%d>: VALUE: %s\n", nombre_tabla, pagina->key,pagina->value);
		encontrada = true;
//		imprimirMensaje(log_memoria,"[WRAPPER DE SELECT] POR AQUIIIIII");
	} else {
		imprimirAviso(log_memoria,"[WRAPPER DE SELECT] Valor no encontrado");
		//printf("\nERROR <%s><%d>\n", nombre_tabla, key);
	}
	free(informacion);
	if(encontrada){

		char* valorADevolver = malloc(strlen(pagina->value));
		memcpy(valorADevolver, pagina->value, strlen(pagina->value)+1);
		imprimirMensaje1(log_memoria, "[WRAPPER DE SELECT] Se encontro lo buscado %s", valorADevolver);
		free(pagina->value);
		free(pagina);
		free(valorAux);
		return valorADevolver;
	}
	imprimirAviso(log_memoria, "[WRAPPER DE SELECT] NO Se encontro lo buscado");
	free(valorAux);
	free(pagina->value);
	free(pagina);
	return NULL;
}

resp_com_t resolver_select(int socket_lfs,request_t req)
{
	imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Voy a resolver SELECT");
	if(req.cant_args != 2)
	{
		imprimirError(log_memoria,"[RESOLVIENDO SELECT] Cantidad de argumentos incorrecta. Reintente");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	resp_com_t retval;
	char *valor;
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	retardo_memoria();
	valor = select_memoria(nombre_tabla,key);
	if(valor == NULL){
		imprimirAviso(log_memoria,"[RESOLVIENDO SELECT] En memoria no existe la tabla o no existe la key en la misma");
		//printf("\nEn memoria no existe la tabla o no existe la key en la misma\n");
		if(socket_lfs != -1){
			//Tengo que pedirlo al filesystem
			req_com_t enviar;
			enviar.tam = strlen(req.request_str)+1;
			enviar.str = malloc(enviar.tam);
			strcpy(enviar.str,req.request_str);
			retardo_fs();
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Voy a mandar select al lfs\n");
			if(enviar_request(socket_lfs,enviar) == -1){
				imprimirError(log_memoria,"[RESOLVIENDO SELECT] ERROR al conectarse con lfs para enviar select\n");
				return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
			}
			borrar_request_com(enviar);
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Espero respuesta del lfs");

			//Espero la respuesta
			msg_com_t msg;
			resp_com_t resp;
			request_t req_lfs;
			char *aux;

			msg = recibir_mensaje(socket_lfs);
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Recibi respuesta del lfs");
			if(msg.tipo == RESPUESTA){
				resp = procesar_respuesta(msg);
				//EL MENSAJE DE RESPUESTA PARA UN SELECT DEBE SER VALOR|TIMESTAMP
				imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] El lfs contesto");
				if(resp.tipo == RESP_OK){
					imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] El lfs pudo resolver el select con exito");
					imprimirMensaje1(log_memoria,"[RESOLVIENDO SELECT] REPUESTA (VALOR|TIMESTAMP): %s",resp.msg.str);

					aux = armar_insert(resp.msg.str,nombre_tabla,key);
					borrar_respuesta(resp);
					imprimirMensaje1(log_memoria,"[RESOLVIENDO SELECT] REPUESTA TRANSFORMADA A: %s",aux);
					//req_lfs = parser(resp.msg.str);
					if(aux != NULL){
						req_lfs = parser(aux);
						free(aux);
						if(req_lfs.command != INSERT){
							imprimirAviso(log_memoria,"[RESOLVIENDO SELECT] No reconozco lo que recibi del lfs");
							retval = armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
						}
						else{
							resolver_insert(req_lfs,false);
							imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Agregué el valor a memoria");
							if(req_lfs.cant_args >= 3){
								retval = armar_respuesta(RESP_OK, req_lfs.args[2]);
							}
						}
						borrar_request(req_lfs);
					}
					else{
						imprimirAviso(log_memoria, "[RESOLVIENDO SELECT] No obtengo la respuesta que espero");
						retval = armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
					}
				}
				else{
					imprimirAviso(log_memoria,"[RESOLVIENDO SELECT] El lfs no pudo resolver el select");
					retval = resp; //Copio la respuesta que recibí del LFS
				}
			}
			else{
				borrar_mensaje(msg);
				retval = armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
			}
		}
		else{ //Si no tengo socket para comunicarme
			retval = armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
		}
	}
	else{//Resolví en memoria, sin LFS
		retval = armar_respuesta(RESP_OK, valor);
		free(valor);
	}

	if(retval.msg.str != NULL){
		printf("\nValor obtenido: %s",retval.msg.str);
	}
	return retval;
}

char *armar_insert(char *respuesta_select, char *tab, int key)
{
	char *retval, **aux;
	int cant = 0;
	//VALOR|TIMESTAMP
	aux = string_split(respuesta_select,"|");
	while(aux[cant]){
		cant++;
	}
	if(cant!=2){
		return NULL;
	}
	retval = malloc(MAX_LONG_INSERT);
	snprintf(retval,MAX_LONG_INSERT-1,"INSERT %s %d %s %s", tab,key, aux[0], aux[1]);
	for(int j=0; j<cant ; j++){
		free(aux[j]);
	}
	free(aux);
	return retval;
}

resp_com_t resolver_describe(int socket_lfs, request_t req)
{
	char *ret_val;
	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Entro a función");
	req_com_t a_enviar;

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);
	retardo_fs();
	imprimirMensaje1(log_memoria, "[RESOLVIENDO DESCRIBE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[RESOLVIENDO DESCRIBE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[RESOLVIENDO DESCRIBE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}

	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] La respuesta vino vacía");
		}
		else{
			imprimirMensaje1(log_memoria, "[RESOLVIENDO DESCRIBE] La respuesta del lfs fue: %s",resp.msg.str);
			ret_val = malloc(resp.msg.tam);
			strcpy(ret_val,resp.msg.str);
		}
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[RESOLVIENDO DESCRIBE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
	}
	return resp;
}

int inicializar_gossiping_memoria(void)
{
	imprimirMensaje(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] Entrando a función");
	if(arc_config == NULL || log_memoria == NULL){
		imprimirAviso(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] Es necesario tener cargado config y log");
		return -1;
	}
	char puerto[LARGO_PUERTO], retardo_str[20];
	snprintf(puerto,LARGO_PUERTO-1,"%d",arc_config->puerto);
	snprintf(retardo_str,19,"%d",arc_config->retardo_gossiping);
	imprimirMensaje1(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] El gossiping se hará cada %s milisegundos", retardo_str);
	inicializar_estructuras_gossiping(log_memoria,arc_config->retardo_gossiping);
	agregar_seed(arc_config->memory_number,arc_config->ip,puerto);
	if(arc_config->ip_seeds != NULL && arc_config->puerto_seeds != NULL){
		int i = 0;
		char *aux_ip = arc_config->ip_seeds[0];
		char *aux_puerto = arc_config->puerto_seeds[0];
		while(aux_ip != NULL && aux_puerto != NULL){
			imprimirMensaje2(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] Agregando seed %s %s",aux_ip,aux_puerto);
			agregar_seed(-1,aux_ip,aux_puerto);
			i++;
			aux_ip = arc_config->ip_seeds[i];
			aux_puerto = arc_config->puerto_seeds[i];
		}
	}
	imprimirMensaje(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] Todas las memorias agregadas");
	return 1;
}
#endif
