/*
 * main_memoria.c
 *
 *  Created on: 15 jun. 2019
 *      Author: martin
 */

#define COMPILAR_MAIN_MEMORIA
#ifdef COMPILAR_MAIN_MEMORIA

#include "main_memoria.h"

#define TESTEAR_GOSSIPING

t_list *clientes_activos;
pthread_mutex_t mutex_clientes_activos = PTHREAD_MUTEX_INITIALIZER;

int g_socket_memoria = -1;

void actualizarMemoriasDisponibles(void)
{
	t_list *caidas = hayMemoriasCaidas();
	seed_com_t *aux;

	if(list_size(caidas) == 0)
		log_info(log_memoria,"No hay memorias caidas");
	else{
		for(int i=0;i<list_size(caidas);i++){
			aux = list_get(caidas,i);
			log_info(log_memoria,"Memoria caida (%d): %d-%s-%s",i,aux->numMemoria,aux->ip,aux->puerto);
		}
	}

	if(huboCambios()){
		log_info(log_memoria,"Hubo cambios en las memorias conocidas");
	}
}

int main(int argc, char **argv)
{
	// LOGGING
	printf("\n***INICIANDO EL PROCESO MEMORIA***");

/*#ifdef TESTEAR_GOSSIPING
	if(argc>1)
		inicioLogYConfig(argv[1]);
	else
		inicioLogYConfig(PATH_MEMORIA_CONFIG);
#else
	inicioLogYConfig(PATH_MEMORIA_CONFIG);
//#endif*/

	bool loggerEnConsola = true;
	int numMemoria = -1;
	if(argc < 2){
		printf("\n*Se debe indicar el numero de memoria como primer argumento del main*\n\n");
		return 1;
	}

	numMemoria = strtol(argv[1],NULL,10);
	if(numMemoria <= 0){
		printf("\n*El numero de memoria <%s> no es valido*\n\n",argv[1]);
		return 1;
	}
	printf("\n*El numero de memoria es <%d>*\n\n",numMemoria);

	//Si quiero que no logge en consola ejecuto poniendo "./PoolMemorias <num> -cl". En cualquier otro caso loggea en consola
	if(argc >= 3){
		if(!strcmp(argv[2],"-cl")){
			loggerEnConsola = false;
		}
	}

	if(loggerEnConsola){
		printf("\n*Se loggea en consola. Para desactivarlo ejecutar el proceso con -cl*\n\n\n");
	}
	else{
		printf("\n*Opción -cl. No se loggea en consola*");
	}
	inicioLogYConfig(numMemoria,loggerEnConsola);

//	printf("\n*Log creado en %s", LOG_PATH);
//	printf("\n*Archivo de configuración cargado*");

	int socket_lfs = conectar_a_lfs(true,&max_valor_key);

	if(socket_lfs == -1 || max_valor_key <= 0){
		printf("\n\n***NO SE PUDO ESTABLECER CONEXIÓN CON LFS. ABORTANDO...***\n");
		imprimirError(log_memoria,"[MAIN] Error al conectar al LFS, terminando!\n");
		return 1;
	}

	//Ya no necesito el socket de LFS
	close(socket_lfs);
	socket_lfs = -1;

	printf("\n*Conectado a LFS");
	arc_config->max_value_key = max_valor_key; //agrego +1 para contemplar el '/0'
	imprimirMensaje1(log_memoria,"[MAIN] Iniciando con tamaño máximo de valor %d",arc_config->max_value_key);

	armarMemoriaPrincipal();
	printf("\n*Memoria principal inicializada");

	iniciarSemaforosYMutex();

	int socket_servidor = levantar_servidor_memoria();

	if(socket_servidor == -1){
		printf("\n\n***NO SE PUDO LEVANTAR EL SERVIDOR DE MEMORIA. ABORTANDO...***\n");
		imprimirError(log_memoria, "[MAIN] No se pudo levantar el servidor de escucha. Finalizando...\n");
		return 1;
	}
	printf("\n*El servidor de memoria está inicializado y listo para recibir clientes");

	g_socket_memoria = socket_servidor;

	signal(SIGINT, INThandler);

	pthread_t servidor_h, consola_h, gossiping_h, inotify_c;

	id_com_t mi_id = MEMORIA;
	inicializar_gossiping_memoria();
	iniciar_hilo_gossiping(&mi_id,&gossiping_h,actualizarMemoriasDisponibles);
	printf("\n*Gossiping corriendo");
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&servidor_h, &attr,(void *)hilo_servidor,&socket_servidor);
	pthread_join(servidor_h, NULL);
	printf("\n*Servidor corriendo");

	char* path_de_memoria = malloc(strlen(PATH_MEMORIA_CONFIG)+1);
	strcpy(path_de_memoria, PATH_MEMORIA_CONFIG);

	pthread_create(&inotify_c,&attr, (void *)inotifyAutomatico,path_de_memoria);
	pthread_join(inotify_c, NULL);
	printf("\n*Hilo de actualización de retardos corriendo");

	printf("\n\n***PROCESO MEMORIA CARGADO COMPLETAMENTE***\n\n");

	pthread_create(&consola_h,NULL,(void *)hilo_consola,&socket_lfs);

//pthread_detach(journalHilo);
	pthread_join(consola_h,NULL);
	pthread_cancel(consola_h);
	pthread_cancel(inotify_c);
	pthread_cancel(servidor_h);

	free(path_de_memoria);

	printf("\n\n***FINALIZANDO PROCESO MEMORIA***\n\n");

	//ESTO ESTA MAL, PERO QUIERO VER SI FUNCA LO MIO
//	pthread_cancel(servidor_h, SIGKILL);
//	pthread_cancel(gossiping_h, SIGKILL);
	log_info(log_memoria, "[Liberando] Liberando memoria Gossiping");
	liberar_memoria_gossiping();
	printf("\nSe libero la memoria gossiping");
	log_info(log_memoria, "[Liberando] Cerrando clientes");
	cerrar_todos_clientes();
	printf("\nSe liberaron los clientes\n\n");
	liberar_todo_por_cierre_de_modulo();


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
	else{
		imprimirMensaje(log_memoria,"[LEVANTANDO SERVIDOR] Servidor conectado");
		clientes_activos = list_create();
	}
	return socket;
}

int conectar_a_lfs(bool inicializando, int *tam_valor)
{
	id_com_t memoria = MEMORIA;
	char puerto_fs[20];
	snprintf(puerto_fs,19,"%d",arc_config->puerto_fs);
//	imprimirMensaje2(log_memoria,"[CONECTANDO A LFS] Me voy a intentar conectar a ip: <%s> puerto: <%s>", arc_config->ip_fs, puerto_fs);
	int socket = conectar_a_servidor(arc_config->ip_fs,puerto_fs,memoria);
//	printf("\nOBTUVE RESPUESTA\n\n\n");
	if(socket == -1){
		imprimirError(log_memoria,"[CONECTANDO A LFS] No fue posible conectarse con lissandra.\n");
		return -1;
	}

//	imprimirMensaje(log_memoria,"[CONECTANDO A LFS] Me conecté con éxito al lfs. Espero su hs");
	//Si me conecté, espero su msg de bienvenida

	msg_com_t msg = recibir_mensaje(socket);

	if(msg.tipo != HANDSHAKECOMANDO){
		borrar_mensaje(msg);
		imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs no responde el hs.\n");
		return -1;
	}

	handshake_com_t hs = procesar_handshake(msg);
	borrar_mensaje(msg);

	if(hs.id == RECHAZADO){
		if(hs.msg.tam == 0 )
			imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión.\n");
		else
			imprimirError1(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión [%s].\n",hs.msg.str);
		borrar_handshake(hs);
		close(socket);
		return -1;
	}
	if(inicializando){
		if(hs.msg.tam == 0){
			imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs no mandó el tamaño de los valores.\n");
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
	pthread_create(&journalHilo, NULL, (void*)retardo_journal, NULL);
	pthread_detach(journalHilo);
	while(!fin){
		linea_leida=readline("\n\n>");
		if(linea_leida)
			add_history(linea_leida);
		req = parser(linea_leida);
		log_info(log_memoria,"[CONSOLA] Request: %s",linea_leida);
		free(linea_leida);
		switch(req.command){
			case SALIR:
				fin = 1;
				respuesta = armar_respuesta(RESP_OK,"SALIENDO");
				break;
			case SELECT:
			case INSERT:
			case DESCRIBE:
			case CREATE:
			case DROP:
			case JOURNALCOMANDO:
				retardo_memoria();	//SIEMPRE TENDRAN 1 RETARNO AL ENTRAR UN NUEVO PEDIDO
				respuesta = resolver_pedido(req,socket_lfs);
				if(respuesta.tipo == RESP_OK){
					if(respuesta.msg.tam > 0 && respuesta.msg.str != NULL)
						imprimirMensaje1(log_memoria,"[CONSOLA] Resuelto OK. Respuesta obtenida: %s",respuesta.msg.str);
					else
						imprimirMensaje(log_memoria,"[CONSOLA] Resuelto OK. No se especifica respuesta");
				}
				else{
					if(respuesta.msg.tam > 0 && respuesta.msg.str != NULL)
						imprimirAviso1(log_memoria,"[CONSOLA] Error al resolver pedido. Respuesta %s",respuesta.msg.str);
					else
						imprimirAviso(log_memoria,"[CONSOLA] Error al resolver pedido. No hay mensaje de error");
				}
//				borrar_respuesta(respuesta);
				break;
			default:
				respuesta = armar_respuesta(RESP_ERROR_PEDIDO_DESCONOCIDO, "Pedido desconocido");
				break;
		}
		borrar_request(req);
		if(respuesta.tipo == RESP_OK){
			if(respuesta.msg.tam > 0 && respuesta.msg.str != NULL)
				printf("-> [RESUELTO]: %s",respuesta.msg.str);
			else
				printf("-> [RESUELTO]");
		}
		else{
			printf("***El pedido no se pudo resolver. Error <%d>.",respuesta.tipo);
			if(respuesta.msg.str != NULL){
				printf(" Mensaje de error: \"%s\".", respuesta.msg.str);
			}
		}
		borrar_respuesta(respuesta);
	}
	clear_history();
	printf("\nSaliendo de hilo consola\n");
	return NULL;
}

void cliente_dar_de_alta(int socket)
{
//	cliente_t *cliente = malloc(sizeof(cliente_t));
	int *copia = malloc(sizeof(int));
	memcpy(copia,&socket,sizeof(int));
//	int *copy = malloc(sizeof(int)); //NO HACER UN FREE ACÁ SINO VA A ROMPER, LO HAGO MÁS ADELANTE
	log_info(log_memoria,"[CLIENTE] Dando de alta cliente en socket %d",socket);
//	memcpy(copy,&socket,sizeof(int));
	pthread_mutex_lock(&mutex_clientes_activos);
	list_add(clientes_activos,copia);
	int activos = list_size(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
//	log_info(log_memoria,"[CLIENTE] Socket cliente %d agregado a la lista de activos",socket);
	log_info(log_memoria,"[CLIENTE] Hay %d clientes activos",activos);
}

void cliente_dar_de_baja(int socket)
{
	int *aux;
	bool encontrado = false;
//	log_info(log_memoria,"[CLIENTE] Voy a dar de baja socket %d",socket);
	pthread_mutex_lock(&mutex_clientes_activos);
	for(int i=0;i<list_size(clientes_activos);i++){
		aux = list_get(clientes_activos,i);
		if(*aux == socket){
			list_remove(clientes_activos,i);
			free(aux);
			log_info(log_memoria,"[CLIENTE] Socket cliente %d sacado de la lista de activos",socket);
			encontrado = true;
			break;
		}
	}
	int activos = list_size(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
//	if(encontrado == false)
//		log_info(log_memoria,"[CLIENTE] Socket cliente %d no se dió de baja porque no se encontró en la lista");
	log_info(log_memoria,"[CLIENTE] Hay %d clientes activos",activos);
}

void cerrar_todos_clientes(void)
{
	pthread_mutex_lock(&mutex_clientes_activos);
	log_info(log_memoria,"[CLIENTE] Cerrando sockets de clientes activos. Hay %d",list_size(clientes_activos));
//	int *aux;
	int *aux;
	for(int i=0;i<list_size(clientes_activos);i++){
		aux = list_get(clientes_activos, i);
		if(*aux != -1){
			log_info(log_memoria,"[CLIENTE] Cerrando socket %d",*aux);
//			pthread_cancel(*(aux->hilo));
			close(*aux);//@martin : va con *?
		}
		free(aux);
	}
	list_destroy(clientes_activos);
	pthread_mutex_unlock(&mutex_clientes_activos);
	log_info(log_memoria,"[CLIENTE] Todos los sockets de clientes fueron cerrados");
}

void  INThandler(int sig)
{
	signal(sig, SIG_IGN);
	log_info(log_memoria,"[CATCHING SIGNAL] Cierro sockets clientes antes de salir");
	cerrar_todos_clientes();

	log_info(log_memoria,"[CATCHING SIGNAL] Cierro socket memoria");
	if(g_socket_memoria != -1)
		close(g_socket_memoria);

	log_info(log_memoria,"[CATCHING SIGNAL] Libero memoria");
//	liberar_todo_por_cierre_de_modulo();
	liberar_memoria_gossiping();

	log_info(log_memoria,"[CATCHING SIGNAL] Saliendo");
	exit(0);
}

void* hilo_servidor(int * socket_p){
	int socket = *socket_p;
	cliente_com_t cliente;
	pthread_t thread;
	imprimirMensaje(log_memoria,"[SERVIDOR] Entrando a hilo de escucha del servidor de la memoria");
	hilo_cliente_args_t *args;
	while(1){
//		imprimirMensaje(log_memoria,"[SERVIDOR] Esperando recibir un cliente");
		cliente = esperar_cliente(socket);
//		imprimirMensaje(log_memoria,"[SERVIDOR] Cliente intentando conectarse");
//		pthread_attr_t attr;
//		pthread_attr_init(&attr);
//		//EL PTHREAD_CREATE_DETACHED LO HACE ACTUAR COMO SI FUERA UN DETACH PERO AL FINALIZAR SE LIBERA
//		//DETACH POR OTRA PARTE SOLO TIENE EL PROBLEMA QUE NO SE LIBERAL AL FINAL Y SE VAN ACUMULANDO
//		//EN POSSYBLE LOST.
//		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_t aux;
		switch(cliente.id){
			case MEMORIA:
				args = malloc(sizeof(hilo_cliente_args_t));
				args->socket_cliente = cliente.socket;
				args->requiere_lfs = false;
				dar_bienvenida_cliente(cliente.socket, MEMORIA, "Bienvenido!");
				pthread_create(&thread, NULL,(void *)hilo_cliente, args );
				cliente_dar_de_alta(cliente.socket);
//				pthread_join(thread, NULL);
				pthread_detach(thread);
				break;
			case KERNEL:
				args = malloc(sizeof(hilo_cliente_args_t));
				args->socket_cliente = cliente.socket;
				args->requiere_lfs = false;
				dar_bienvenida_cliente(cliente.socket, MEMORIA, "Bienvenido!");
				pthread_create(&thread, NULL,(void *)hilo_cliente, args );
				cliente_dar_de_alta(cliente.socket);
//				pthread_join(thread, NULL);
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
	/*if(args->requiere_lfs){
		socket_lfs = conectar_a_lfs(false,NULL);
		if(socket_lfs == -1){
			imprimirError(log_memoria,"[CLIENTE] No pude conectarme al lfs");
		}
		else{
			imprimirMensaje(log_memoria,"[CLIENTE] Ya tengo un canal para comunicarme con el lfs");
		}
	}*/
//	imprimirMensaje(log_memoria,"[CLIENTE] Empiezo a esperar mensajes");
	msg_com_t msg;
	bool fin = false;
	while(fin == false){
		msg = recibir_mensaje(socket_cliente);
//		imprimirMensaje(log_memoria,"[CLIENTE] Recibí un mensaje");
		retardo_memoria();	//SIEMPRE TENDRAN 1 RETARNO AL ENTRAR UN NUEVO PEDIDO
		req_com_t request;
		request_t req_parseado;
		gos_com_t gossip;
		resp_com_t respuesta;
		switch(msg.tipo){
			case REQUEST:
				request = procesar_request(msg);
				borrar_mensaje(msg);
				req_parseado = parser(request.str);
				borrar_request_com(request);
				log_info(log_memoria,"[CLIENTE] El cliente envio: %s",req_parseado.request_str);
				respuesta = resolver_pedido(req_parseado,socket_lfs);

				if(respuesta.tipo == RESP_OK)
					imprimirMensaje1(log_memoria,"[CLIENTE] Pedido resuelto OK. "
							"La resupuesta obtenida para el pedido es %s",respuesta.msg.str);
				else
					imprimirMensaje1(log_memoria,"[CLIENTE] Pedido no pudo ser resuelto. "
							"La resupuesta obtenida para el pedido es %s",respuesta.msg.str);
				if(enviar_respuesta(socket_cliente,respuesta) != -1) {
					imprimirMensaje(log_memoria,"[CLIENTE] La resupuesta fue enviada "
							"con éxito al cliente");
				}
				else {
					imprimirError(log_memoria,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				borrar_request(req_parseado);
				borrar_respuesta(respuesta);
				break;
			case GOSSIPING:
				imprimirMensaje(log_memoria,"[CLIENTE] El cliente hizo un pedido de gossiping");
				gossip = procesar_gossiping(msg);
				borrar_mensaje(msg);
				if(responder_gossiping(gossip,MEMORIA,socket_cliente) != -1) {
					imprimirMensaje(log_memoria,"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
				}
				else {
					imprimirError(log_memoria,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				borrar_gossiping(gossip);
				break;
			case DESCONECTADO:
				imprimirMensaje(log_memoria,"[CLIENTE] El cliente se desconectó");
				borrar_mensaje(msg);
				//close(socket_cliente);
				if(socket_lfs != -1)
					close(socket_lfs);
				fin = true;
				cliente_dar_de_baja(socket_cliente);
				if(socket_cliente != -1)
					close(socket_cliente);
				break;
			default:
				imprimirAviso(log_memoria,"[CLIENTE] El tipo de mensaje no está permitido para este cliente");
				borrar_mensaje(msg);
				break;
		}
	}
	free(args);
//	imprimirMensaje(log_memoria,"[CLIENTE] Finalizando el hilo");
//	pthread_cancel(pthread_self());
	return NULL;
}

int responder_gossiping(gos_com_t recibido,id_com_t id_proceso,int socket)
{
	gos_com_t conocidas = armar_vector_seeds(id_proceso);
	incorporar_seeds_gossiping(recibido);
	if(enviar_gossiping(socket,conocidas) == -1){
		borrar_gossiping(conocidas);
		return -1;
	}
	borrar_gossiping(conocidas);
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
			//imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver INSERT");
			respuesta = resolver_insert(req,true);
			if( respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
			}
			break;
		case SELECT:
			//imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver SELECT");
			respuesta = resolver_select(socket_lfs,req);
			if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",respuesta.msg.str);
			}
			else{
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
			}
			break;
		case DESCRIBE:
			//imprimirAviso(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE");
			respuesta = resolver_describe(socket_lfs,req);
			if(respuesta.tipo == RESP_OK && respuesta.msg.tam > 0){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",respuesta.msg.str);
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
			}
			break;
		case DROP:
			//imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver DROP");
			respuesta = resolver_drop(socket_lfs,req);
			if(respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
			}
			break;
		case CREATE:
			//imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver CREATE");
			respuesta = resolver_create(socket_lfs,req);
			if(respuesta.tipo == RESP_OK){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");
			}
			break;
		case JOURNALCOMANDO:
			//imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver JOURNAL");
			respuesta = resolver_journal(socket_lfs,req);
			if(respuesta.tipo == RESP_OK ){
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
	imprimirMensaje(log_memoria,"[DROP] Voy a resolver DROP");
	if(req.cant_args != 1){
		imprimirError(log_memoria,"[DROP] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	char *nombre_tabla = malloc(strlen(req.args[0])+1);
	strcpy(nombre_tabla,req.args[0]);
	retardo_memoria();
	if(funcionDrop(nombre_tabla)==-1){
		imprimirAviso1(log_memoria, "[DROP] La tabla ya fue eliminada o no existe en memoria: <%s>", nombre_tabla);
	}

	socket_lfs = -1;
	socket_lfs = conectar_a_lfs(false,NULL);
	if(socket_lfs == -1){
		imprimirError(log_memoria,"[DROP] No pude conectarme al lfs");
	}
	else{
//		imprimirMensaje(log_memoria,"[DROP] Ya tengo un canal para comunicarme con el lfs");
	}

	//Le envio el DROP al filesystem
	req_com_t enviar;
	enviar.tam = strlen("DROP ") + strlen(nombre_tabla) + 1;
	enviar.str = malloc(enviar.tam);
	strcpy(enviar.str,"DROP ");
	strcat(enviar.str,nombre_tabla);
	free(nombre_tabla);
	retardo_fs();
	imprimirMensaje(log_memoria, "[DROP] Voy a enviar el drop al filesystem");
	if(enviar_request(socket_lfs,enviar) == -1){
		imprimirError(log_memoria, "[DROP] No se puedo enviar el drop al filesystem");
		borrar_request_com(enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(enviar);
	//Espero su respuesta
	msg_com_t msg = recibir_mensaje(socket_lfs);
//	retardo_fs();
	if(msg.tipo == RESPUESTA){
		respuesta = procesar_respuesta(msg);
		borrar_mensaje(msg);
		if(respuesta.tipo == RESP_OK){
			imprimirMensaje(log_memoria, "[DROP] El filesystem realizó el DROP con éxito");
		}
		else{
			imprimirError(log_memoria, "[DROP] El filesystem no pudo realizar el DROP con éxito.");
//			borrar_respuesta(recibido);
//			return -1;
		}
		if(respuesta.msg.tam >0 && respuesta.msg.str != NULL)
			imprimirMensaje1(log_memoria,"[DROP] El filesystem contestó al DROP con %s",respuesta.msg.str);
//		borrar_respuesta(recibido);
	}
	else{
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	if(socket_lfs != -1)
		close(socket_lfs);
	return respuesta;
}

resp_com_t resolver_create(int socket_lfs,request_t req)
{
	if(req.cant_args < 1){
		imprimirError(log_memoria,"[CREATE] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}

	socket_lfs = -1;
	socket_lfs = conectar_a_lfs(false,NULL);
	if(socket_lfs == -1){
		imprimirError(log_memoria,"[CREATE] No pude conectarme al lfs");
	}
	else{
//		imprimirMensaje(log_memoria,"[CREATE] Ya tengo un canal para comunicarme con el lfs");
	}

	//Le envio el CREATE al filesystem
	req_com_t a_enviar;

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);
	retardo_fs();
	imprimirMensaje1(log_memoria, "[CREATE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[CREATE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[CREATE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[CREATE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}

	imprimirMensaje(log_memoria, "[CREATE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[CREATE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[CREATE] La respuesta vino vacía");
//			ret_val = NULL;
		}
		else{
			imprimirMensaje1(log_memoria, "[CREATE] La respuesta del lfs fue: %s",resp.msg.str);
//			ret_val = malloc(resp.msg.tam);
//			strcpy(ret_val,resp.msg.str);
		}
//		borrar_respuesta(resp);
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[CREATE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
//		borrar_respuesta(resp);
//		return NULL;
	}
	if(socket_lfs != -1)
		close(socket_lfs);
	return resp;
}

resp_com_t resolver_journal(int socket_lfs,request_t req)
{
	imprimirMensaje(log_memoria,"[JOURNAL] Voy a resolver JOURNAL");
//	//Consigo toda la info modificada de memoria
//	datosJournal *datos_modificados = obtener_todos_journal();
//	datosJournal *aux = datos_modificados;
//
//	imprimirMensaje(log_memoria, "[RESOLVIENDO JOURNAL] DATOS MODIFICADOS: ");
//	while(aux!=NULL){
//		imprimirAviso3(log_memoria, "%s %d %s", aux->nombreTabla, aux->key, aux->value);
//		aux = aux->sig;
//	}
	socket_lfs = -1;
	socket_lfs = conectar_a_lfs(false,NULL);
	if(socket_lfs == -1){
		imprimirError(log_memoria,"[JOURNAL] No pude conectarme al lfs");
	}
	else{
//		imprimirMensaje(log_memoria,"[JOURNAL] Ya tengo un canal para comunicarme con el lfs");
	}
	char *msg_resp = malloc(100);
	resp_com_t resp;

	rwLockEscribir(&sem_insert_select);
	int cant_pasados = procesoJournal(socket_lfs);
	rwLockDesbloquear(&sem_insert_select);

	if(cant_pasados != -1){
		snprintf(msg_resp, 99, "Journal hecho.");
		resp = armar_respuesta(RESP_OK, msg_resp);
	}
	else{
		snprintf(msg_resp, 99, "El journal no fue realizado correctamente");
		resp = armar_respuesta(RESP_ERROR_COMUNICACION, msg_resp);
	}
	free(msg_resp);
	if(socket_lfs != -1)
		close(socket_lfs);
	return resp;
}

resp_com_t resolver_insert(request_t req, int modif)
{
	imprimirMensaje(log_memoria,"[INSERT] Voy a resolver INSERT");
	timestamp_mem_t timestamp_val;
	if(req.cant_args == 3)
		 timestamp_val = 0;
	else if(req.cant_args == 4)
		timestamp_val = strtoull(req.args[3],NULL,10);
	else{
		imprimirError(log_memoria,"[INSERT] Cantidad incorrecta de parámetros");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	char *valor = req.args[2];
	retardo_memoria();
//	imprimirMensaje3(log_memoria,"[INSERT] Voy a agregar %s en la key %d de la tabla %s",valor,key,nombre_tabla);
	if(funcionInsert(nombre_tabla, key, valor, modif, timestamp_val)== -1){
//		imprimirError(log_memoria, "[INSERT] ERROR: Mayor al pasar max value");
		return armar_respuesta(RESP_ERROR_MAYOR_MAX_VALUE,NULL);
	}
	return armar_respuesta(RESP_OK,NULL);
}

char* select_memoria(char *nombre_tabla, uint16_t key)
{
	pagina_a_devolver* pagina = malloc(sizeof(pagina_a_devolver));

//	imprimirMensaje2(log_memoria,"[WRAPPER DE SELECT] Quiero obtener la key %d de la tabla %s",key,nombre_tabla);
	segmento *seg;
	int pag;
	bool encontrada = false;
	char* valorAux = malloc(max_valor_key);
	//void* informacion = malloc(sizeof(pagina)+max_valor_key);
	pagina->value = malloc(max_valor_key);
	rwLockLeer(&sem_insert_select);
	if(funcionSelect(nombre_tabla, key, &pagina, &valorAux)!=0){
		pag = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombre_tabla, key, &seg);
		free(pagina->value);
		free(pagina);
		//		imprimirMensaje(log_memoria,"[WRAPPER DE SELECT] POR AQUIIIIII NO");
		pagina = selectPaginaPorPosicion(pag,true);
//		imprimirMensaje1(log_memoria,"[WRAPPER DE SELECT] Valor encontrado: %s",pagina->value);
		encontrada = true;

	} else {
//		imprimirAviso(log_memoria,"[WRAPPER DE SELECT] Valor no encontrado");

	}
	rwLockDesbloquear(&sem_insert_select);

	//free(informacion);
	if(encontrada){

		char* valorADevolver = malloc(strlen(pagina->value)+1);
		strcpy(valorADevolver, pagina->value);
//		imprimirMensaje1(log_memoria, "[WRAPPER DE SELECT] Se encontro lo buscado %s", valorADevolver);
		free(pagina->value);
		free(pagina);
		free(valorAux);
		return valorADevolver;
	}
	free(valorAux);
	free(pagina->value);
	free(pagina);
	return NULL;
}

resp_com_t resolver_select(int socket_lfs,request_t req)
{
	imprimirMensaje(log_memoria,"[SELECT] Voy a resolver SELECT");
	if(req.cant_args != 2)
	{
		imprimirError(log_memoria,"[SELECT] Cantidad de argumentos incorrecta. Reintente");
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS,NULL);
	}
	resp_com_t retval;
	char *valor;
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	retardo_memoria();
	valor = select_memoria(nombre_tabla,key);
	if(valor == NULL){
		imprimirAviso(log_memoria,"[SELECT] En memoria no existe la tabla o no existe la key en la misma");
		//printf("\nEn memoria no existe la tabla o no existe la key en la misma\n");
		socket_lfs = -1;
		socket_lfs = conectar_a_lfs(false,NULL);
		if(socket_lfs == -1){
			imprimirError(log_memoria,"[SELECT] No pude conectarme al lfs");
		}
		else{
//			imprimirMensaje(log_memoria,"[SELECT] Ya tengo un canal para comunicarme con el lfs");
		}

		if(socket_lfs != -1){
			//Tengo que pedirlo al filesystem
			req_com_t enviar;
			enviar.tam = strlen(req.request_str)+1;
			enviar.str = malloc(enviar.tam);
			strcpy(enviar.str,req.request_str);
			retardo_fs();
			imprimirMensaje(log_memoria,"[SELECT] Voy a mandar select al lfs");
			if(enviar_request(socket_lfs,enviar) == -1){
				imprimirError(log_memoria,"[SELECT] ERROR al conectarse con lfs para enviar select");
				return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
			}
			borrar_request_com(enviar);
			imprimirMensaje(log_memoria,"[SELECT] Espero respuesta del lfs");

			//Espero la respuesta
			msg_com_t msg;
			resp_com_t resp;
			request_t req_lfs;
			char *aux;

			msg = recibir_mensaje(socket_lfs);

//			retardo_fs();

			imprimirMensaje(log_memoria,"[SELECT] Recibi respuesta del lfs");
			if(msg.tipo == RESPUESTA){
				resp = procesar_respuesta(msg);
				borrar_mensaje(msg);
				//EL MENSAJE DE RESPUESTA PARA UN SELECT DEBE SER VALOR|TIMESTAMP
				imprimirMensaje(log_memoria,"[SELECT] El lfs contesto");
				if(resp.tipo == RESP_OK){
					imprimirMensaje(log_memoria,"[SELECT] El lfs pudo resolver el select con exito");
					imprimirMensaje1(log_memoria,"[SELECT] REPUESTA (VALOR|TIMESTAMP): %s",resp.msg.str);

					aux = armar_insert(resp.msg.str,nombre_tabla,key);
					borrar_respuesta(resp);
					imprimirMensaje1(log_memoria,"[SELECT] REPUESTA TRANSFORMADA A: %s",aux);
					//req_lfs = parser(resp.msg.str);
					if(aux != NULL){
						req_lfs = parser(aux);
						free(aux);
						if(req_lfs.command != INSERT){
							imprimirAviso(log_memoria,"[SELECT] No reconozco lo que recibi del lfs");
							retval = armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
						}
						else{
							resolver_insert(req_lfs,false);
							imprimirMensaje(log_memoria,"[SELECT] Agregué el valor a memoria");
							if(req_lfs.cant_args >= 3){
								retval = armar_respuesta(RESP_OK, req_lfs.args[2]);
							}
						}
						borrar_request(req_lfs);
					}
					else{
						imprimirAviso(log_memoria, "[SELECT] No obtengo la respuesta que espero");
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
		if(socket_lfs != -1)
			close(socket_lfs);
	}
	else{//Resolví en memoria, sin LFS
		retval = armar_respuesta(RESP_OK, valor);
		free(valor);
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
	snprintf(retval,MAX_LONG_INSERT-1,"INSERT %s %d \"%s\" %s", tab,key, aux[0], aux[1]);
	for(int j=0; j<cant ; j++){
		free(aux[j]);
	}
	free(aux);
	return retval;
}

resp_com_t resolver_describe(int socket_lfs, request_t req)
{
	imprimirMensaje(log_memoria,"[DESCRIBE] Voy a resolver DESCRIBE");
	char *ret_val;
	req_com_t a_enviar;

	socket_lfs = -1;
	socket_lfs = conectar_a_lfs(false,NULL);
	if(socket_lfs == -1){
		imprimirError(log_memoria,"[DESCRIBE] No pude conectarme al lfs");
	}
	else{
		imprimirMensaje(log_memoria,"[DESCRIBE] Ya tengo un canal para comunicarme con el lfs");
	}

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);

	imprimirMensaje1(log_memoria, "[DESCRIBE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[DESCRIBE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[DESCRIBE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);

	retardo_fs();

	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[DESCRIBE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION,NULL);
	}

	imprimirMensaje(log_memoria, "[DESCRIBE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[DESCRIBE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[DESCRIBE] La respuesta vino vacía");
		}
		else{
			imprimirMensaje1(log_memoria, "[DESCRIBE] La respuesta del lfs fue: %s",resp.msg.str);
		}
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[DESCRIBE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
	}
	if(socket_lfs != -1)
		close(socket_lfs);
	return resp;
}

int inicializar_gossiping_memoria(void)
{
//	imprimirMensaje(log_memoria,"[INICIALIZANDO GOSSIPING MEMORIA] Entrando a función");
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
