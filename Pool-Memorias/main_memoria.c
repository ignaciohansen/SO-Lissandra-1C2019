/*
 * main_memoria.c
 *
 *  Created on: 15 jun. 2019
 *      Author: martin
 */

#define COMPILAR_MAIN_MEMORIA
#ifdef COMPILAR_MAIN_MEMORIA

//#define EJECUTAR_GOSSIPING


#include "parser.h"
#include "memoria.h"
#include "gestionMemoria.h"

int conectar_a_lfs(void);
int levantar_servidor_memoria(void);
void* hilo_consola(int * socket_p);
char* resolver_pedido(request_t req, int socket_lfs);
char *resolver_select(int socket_lfs,request_t req);
int resolver_insert(request_t req, int modif);
char *resolver_describe(int socket_lfs,request_t req);
char *resolver_create(int socket_lfs,request_t req);
int resolver_drop(int socket_lfs,request_t req);
int resolver_journal(int socket_lfs,request_t req);
void* hilo_servidor(int * socket_p);
void * hilo_cliente(int * socket_p);
int dar_bienvenida_cliente(int socket);
int rechazar_cliente(int socket);
int responder_request(int socket,char *msg, resp_tipo_com_t tipo_resp);
int inicializar_gossiping_memoria(void);
int responder_gossiping(gos_com_t recibido,id_com_t id_proceso,int socket);

//#define AUTOLANZAR_SERVER
//#define AUTOLANZAR_CLIENTE

int main(void)
{
	// LOGGING
	printf("INICIANDO EL MODULO MEMORIA \n COMINEZA EL TP PIBE\n\n");
	inicioLogYConfig();

#ifdef AUTOLANZAR_SERVER
	char cmd_server[100];
	snprintf(cmd_server,99,"gnome-terminal -- ./servidor %s %d",arc_config->ip_fs,arc_config->puerto_fs);
	system(cmd_server);
#endif

	max_valor_key = arc_config->max_value_key;
	armarMemoriaPrincipal();

	iniciarSemaforosYMutex();

	int socket_lfs = conectar_a_lfs();
	//hilo_consola(&socket_lfs);

	int socket_servidor = levantar_servidor_memoria();
	//hilo_servidor(&socket_servidor);

#ifdef AUTOLANZAR_CLIENTE
	char cmd_cliente[100];
	snprintf(cmd_cliente,99,"gnome-terminal -- ./cliente %s %d",arc_config->ip,arc_config->puerto);
	system(cmd_cliente);
#endif

	pthread_t servidor_h, consola_h, gossiping_h;
	id_com_t mi_id = MEMORIA;

#ifdef EJECUTAR_GOSSIPING
	inicializar_gossiping_memoria();
	iniciar_hilo_gossiping(&mi_id,&gossiping_h);
#endif

	pthread_create(&servidor_h,NULL,(void *)hilo_servidor,&socket_servidor);
	pthread_detach(servidor_h);




	pthread_create(&consola_h,NULL,(void *)hilo_consola,&socket_lfs);
//pthread_detach(journalHilo);
	pthread_create(&journalHilo, NULL, retardo_journal, arc_config->retardo_journal);
		pthread_detach(journalHilo);
	pthread_join(consola_h,NULL);

	pthread_kill(servidor_h, SIGKILL);
	pthread_kill(gossiping_h, SIGKILL);
	pthread_kill(journalHilo, SIGKILL);

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
	imprimirAviso2(log_memoria,"[LEVANTANDO SERVIDOR] Me voy a intentar conectar en ip: <%s> puerto: <%s>", arc_config->ip, puerto);

	int socket = iniciar_servidor(arc_config->ip,puerto);

	if(socket == -1){
		imprimirError(log_memoria,"[LEVANTANDO SERVIDOR] Error al levantar el servidor. Por favor, reintente");
	}
	else
		imprimirAviso(log_memoria,"[LEVANTANDO SERVIDOR] Servidor conectado");
	return socket;
}

int conectar_a_lfs(void)
{
	id_com_t memoria = MEMORIA;
	char puerto_fs[20];
	snprintf(puerto_fs,19,"%d",arc_config->puerto_fs);
	imprimirAviso2(log_memoria,"\n[CONECTANDO A LFS] Me voy a intentar conectar a ip: <%s> puerto: <%s>", arc_config->ip_fs, puerto_fs);
	int socket = conectar_a_servidor(arc_config->ip_fs,puerto_fs,memoria);
	if(socket == -1){
		imprimirError(log_memoria,"[CONECTANDO A LFS] No fue posible conectarse con lissandra. TERMINANDO\n");
		return -1;
	}

	imprimirAviso(log_memoria,"[CONECTANDO A LFS] Me conecté con éxito al lfs. Espero su hs");
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
		if(hs.msg.tam ==0 )
			imprimirError(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión. TERMINANDO\n");
		else
			imprimirError1(log_memoria,"[CONECTANDO A LFS] Lfs rechazo la conexión [%s]. TERMINANDO\n",hs.msg.str);
		borrar_handshake(hs);
		return -1;
	}
	borrar_handshake(hs);
	imprimirAviso(log_memoria,"[CONECTANDO A LFS] Me conecté con éxito al lfs");

	return socket;
}

void* hilo_consola(int * socket_p){
	request_t req;
	int socket_lfs = *socket_p;
	char *linea_leida, *respuesta;
	int fin = 0;
	imprimirAviso(log_memoria,"[CONSOLA] Entrando a hilo consola");
	using_history();
	imprimirPorPantallaTodosLosComandosDisponibles();

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
				imprimirMensaje1(log_memoria,"\n[CONSOLA] Respuesta obtenida: %s",respuesta);
				free(respuesta);
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
			case MEMORIA:
				dar_bienvenida_cliente(cliente.socket);
				pthread_create(&thread,NULL,(void *)hilo_cliente, &cliente.socket );
				pthread_detach(thread);
				break;
			case KERNEL:
				dar_bienvenida_cliente(cliente.socket);
				pthread_create(&thread,NULL,(void *)hilo_cliente, &cliente.socket );
				pthread_detach(thread);
				break;
			default:
				rechazar_cliente(cliente.socket);
				close(cliente.socket);
				break;
		}
	}
	return NULL;
}

void * hilo_cliente(int * socket_p)
{
	imprimirMensaje(log_memoria,"[CLIENTE] Entrando a hilo de atención a un cliente");
	int socket_cliente = *socket_p;
	int socket_lfs = conectar_a_lfs();
	if(socket_lfs == -1){
		imprimirError(log_memoria,"[CLIENTE] No pude conectarme al lfs");
	}
	else{
		imprimirMensaje(log_memoria,"[CLIENTE] Ya tengo un canal para comunicarme con el lfs");
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
		char *respuesta;
		switch(msg.tipo){
			case REQUEST:
				imprimirMensaje(log_memoria,"[CLIENTE] El mensaje recibido es un request");
				request = procesar_request(msg);
				borrar_mensaje(msg);
				req_parseado = parser(request.str);
				borrar_request_com(request);
				respuesta = resolver_pedido(req_parseado,socket_lfs);
				imprimirMensaje1(log_memoria,"[CLIENTE] La resupuesta obtenida para el pedido es %s",respuesta);
				if(responder_request(socket_cliente,respuesta,RESP_OK) != -1) {
					imprimirMensaje(log_memoria,"[CLIENTE] La resupuesta fue enviada con éxito al cliente");
				}
				else {
					imprimirError(log_memoria,"[CLIENTE] La resupuesta no pudo ser enviada al cliente");
				}
				free(respuesta);
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

int responder_request(int socket,char *msg, resp_tipo_com_t tipo_resp)
{
	resp_com_t resp;
	resp.tipo = tipo_resp;
	if(msg != NULL){
		resp.msg.tam = strlen(msg)+1;
		resp.msg.str = malloc(resp.msg.tam);
		strcpy(resp.msg.str,msg);
	}
	else{
		resp.msg.tam = 0;
		resp.msg.str = NULL;
	}
	if(enviar_respuesta(socket,resp)==-1){
		borrar_respuesta(resp);
		return -1;
	}
	borrar_respuesta(resp);
	return 1;
}

int dar_bienvenida_cliente(int socket)
{
	handshake_com_t hs;
	hs.id = MEMORIA;
	hs.msg.tam = 40;
	hs.msg.str = malloc(hs.msg.tam);
	strcpy(hs.msg.str,"Bienvenido");
	enviar_handshake(socket,hs);
	borrar_handshake(hs);
	return 1;
}

int rechazar_cliente(int socket)
{
	handshake_com_t hs;
	hs.id = RECHAZADO;
	hs.msg.tam = 0;
	hs.msg.str = NULL;
	enviar_handshake(socket,hs);
	borrar_handshake(hs);
	return 1;
}

char* resolver_pedido(request_t req, int socket_lfs)
{
	char *ret_val=NULL;
	char *ret_ok_generico = malloc(3);
	strcpy(ret_ok_generico,"OK");
	switch(req.command){
		case INSERT:
			imprimirMensaje(log_memoria,"\n\n[RESOLVIENDO PEDIDO] Voy a resolver INSERT\n\n");
			if(resolver_insert(req,true) != -1){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
				ret_val = ret_ok_generico;
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
			}
			break;
		case SELECT:
			imprimirMensaje(log_memoria,"\n\n[RESOLVIENDO PEDIDO] Voy a resolver SELECT\n\n");
			ret_val = resolver_select(socket_lfs,req);
			if(ret_val != NULL){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",ret_val);
			}
			else{
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
			}
			break;
		case DESCRIBE:
			imprimirAviso(log_memoria,"\n\n[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE\n\n");
			ret_val = resolver_describe(socket_lfs,req);
			if(ret_val != NULL){
				imprimirMensaje1(log_memoria,"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",ret_val);
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
			}
			break;
		case DROP:
			imprimirMensaje(log_memoria,"\n\n[RESOLVIENDO PEDIDO] Voy a resolver DROP\n\n");
			if(resolver_drop(socket_lfs,req) != -1){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
				ret_val = ret_ok_generico;
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
			}
			break;
		case CREATE:
			imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] Voy a resolver CREATE\n\n");
			if(resolver_create(socket_lfs,req) != NULL){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
				ret_val = ret_ok_generico;
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");
			}
			break;
		case JOURNALCOMANDO:
			imprimirMensaje(log_memoria,"\n\n[RESOLVIENDO PEDIDO] Voy a resolver JOURNAL\n\n");
			if(resolver_journal(socket_lfs,req) != -1){
				imprimirMensaje(log_memoria,"[RESOLVIENDO PEDIDO] JOURNAL hecho correctamente");
				ret_val = ret_ok_generico;
			}
			else{
				imprimirError(log_memoria,"[RESOLVIENDO PEDIDO] El JOURNAL no pudo realizarse");
			}
			break;
		default:
			break;
	}
	if(ret_val != ret_ok_generico){
		free(ret_ok_generico);
	}
	fprintf(tablas_fp,"\nEjecutado comando %s",req.request_str);
	loggearEstadoActual(tablas_fp);
	return ret_val;
}


int resolver_drop(int socket_lfs,request_t req)
{
	imprimirMensaje(log_memoria,"[RESOLVIENDO DROP] Voy a resolver DROP");
	if(req.cant_args != 1){
		imprimirError(log_memoria,"[RESOLVIENDO DROP] Cantidad incorrecta de parámetros");
		return -1;
	}
	char *nombre_tabla = malloc(strlen(req.args[0])+1);
	strcpy(nombre_tabla,req.args[0]);
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
	if(enviar_request(socket_lfs,enviar) == -1){
		imprimirError(log_memoria, "[RESOLVIENDO DROP] No se puedo enviar el drop al filesystem");
	}
	borrar_request_com(enviar);
	//Espero su respuesta
	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo == RESPUESTA){
		resp_com_t recibido = procesar_respuesta(msg);
		borrar_mensaje(msg);
		if(recibido.tipo == RESP_OK){
			imprimirMensaje(log_memoria, "[RESOLVIENDO DROP] El filesystem realizó el DROP con éxito");
		}
		else{
			imprimirError(log_memoria, "[RESOLVIENDO DROP] El filesystem no pudo realizar el DROP con éxito.");
			borrar_respuesta(recibido);
			return -1;
		}
		if(recibido.msg.tam >0)
			imprimirMensaje1(log_memoria,"[RESOLVIENDO DROP] El filesystem contestó al DROP con %s",recibido.msg.str);
		borrar_respuesta(recibido);
	}
	else{
		borrar_mensaje(msg);
	}
	return 1;
}

char *resolver_create(int socket_lfs,request_t req)
{
	char *ret_val;
	if(req.cant_args < 1){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] Cantidad incorrecta de parámetros");
		return NULL;
	}

	//Le envio el CREATE al filesystem
	req_com_t a_enviar;

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);

	imprimirMensaje1(log_memoria, "[RESOLVIENDO CREATE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return NULL;
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[RESOLVIENDO CREATE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return NULL;
	}

	imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[RESOLVIENDO CREATE] La respuesta vino vacía");
			ret_val = NULL;
		}
		else{
			imprimirMensaje1(log_memoria, "[RESOLVIENDO CREATE] La respuesta del lfs fue: %s",resp.msg.str);
			ret_val = malloc(resp.msg.tam);
			strcpy(ret_val,resp.msg.str);
		}
		borrar_respuesta(resp);
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[RESOLVIENDO CREATE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
		borrar_respuesta(resp);
		return NULL;
	}
	return ret_val;
}

int resolver_journal(int socket_lfs,request_t req)
{

	//Consigo toda la info modificada de memoria
	datosJournal *datos_modificados = obtener_todos_journal();
	datosJournal *aux = datos_modificados;

	imprimirMensaje(log_memoria, "[RESOLVIENDO JOURNAL] DATOS MODIFICADOS: ");
	while(aux!=NULL){
		imprimirAviso3(log_memoria, "%s %d %s", aux->nombreTabla, aux->key, aux->value);
		aux = aux->sig;
	}

	//Ya puedo liberar la memoria y habilitar a que lleguen más pedidos

	//Tengo que enviar al filesystem

	//Espero respuesta de qué pudo hacer y qué no

	//Respondo al que pidió el journal que ya está ok e informo posibles errores

	liberarDatosJournal(datos_modificados);

/*
	//Le envio el DROP al filesystem
	req_com_t enviar;
	enviar.tam = strlen("DROP ") + strlen(nombre_tabla) + 1;
	enviar.str = malloc(enviar.tam);
	strcpy(enviar.str,"DROP ");
	strcat(enviar.str,nombre_tabla);
	if(enviar_request(socket_lfs,enviar) == -1){
		imprimirError(log_memoria, "[RESOLVIENDO DROP] No se puedo enviar el drop al filesystem");
	}
	borrar_request_com(enviar);
	//Espero su respuesta
	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo == RESPUESTA){
		resp_com_t recibido = procesar_respuesta(msg);
		borrar_mensaje(msg);
		if(recibido.tipo == RESP_OK){
			imprimirAviso(log_memoria, "[RESOLVIENDO DROP] El filesystem realizó el DROP con éxito");
		}
		else{
			imprimirError(log_memoria, "[RESOLVIENDO DROP] El filesystem no pudo realizar el DROP con éxito.");
			borrar_respuesta(recibido);
			return -1;
		}
		if(recibido.msg.tam >0)
			imprimirAviso1(log_memoria,"[RESOLVIENDO DROP] El filesystem contestó al DROP con %s",recibido.msg.str);
		borrar_respuesta(recibido);
	}
	else{
		borrar_mensaje(msg);
	}
	*/
	liberar_todo_segmento();
	return 1;
}

int resolver_insert(request_t req, int modif)
{
	imprimirMensaje(log_memoria,"[RESOLVIENDO INSERT] Voy a resolver INSERT");
	double timestamp_val;
	if(req.cant_args == 3)
		 timestamp_val = -1;
	else if(req.cant_args == 4)
		timestamp_val = atof(req.args[3]);
	else{
		imprimirError(log_memoria,"[RESOLVIENDO INSERT] Cantidad incorrecta de parámetros");
		return -1;
	}
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
	char *valor = req.args[2];
	imprimirMensaje3(log_memoria,"[RESOLVIENDO INSERT] Voy a agregar %s en la key %d de la tabla %s",valor,key,nombre_tabla);
	if(funcionInsert(nombre_tabla, key, valor, modif, timestamp_val)== -1){
		imprimirError(log_memoria, "[RESOLVIENDO INSERT]ERROR: Mayor al pasar max value");
		return -1;
	}
	return 1;
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

char *resolver_select(int socket_lfs,request_t req)
{
	imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Voy a resolver SELECT");
	if(req.cant_args != 2)
	{
		imprimirError(log_memoria,"[RESOLVIENDO SELECT] Cantidad de argumentos incorrecta. Reintente");
		return NULL;
	}
	char *valor;
	char *nombre_tabla = req.args[0];
	uint16_t key = atoi(req.args[1]);
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
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Voy a mandar select al lfs\n");
			if(enviar_request(socket_lfs,enviar) == -1){
				imprimirError(log_memoria,"[RESOLVIENDO SELECT] ERROR al conectarse con lfs para enviar select\n");
				return NULL;
			}
			borrar_request_com(enviar);
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Espero respuesta del lfs");

			//Espero la respuesta
			msg_com_t msg;
			resp_com_t resp;
			request_t req_lfs;

			msg = recibir_mensaje(socket_lfs);
			imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Recibi respuesta del lfs");
			if(msg.tipo == RESPUESTA){
				resp = procesar_respuesta(msg);
				//ASUMO QUE ME VA A LLEGAR ALGO DEL TIPO: INSERT <TABLA> <KEY> <VALOR> <TIMESTAMP>
				imprimirMensaje(log_memoria,"\nAVISO, el lfs contesto\n");
				if(resp.tipo == RESP_OK){
					imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] El lfs pudo resolver el select con exito");
					imprimirMensaje1(log_memoria,"[RESOLVIENDO SELECT] REPUESTA: %s",resp.msg.str);
					req_lfs = parser(resp.msg.str);
					borrar_respuesta(resp);
					if(req_lfs.command != INSERT){
						imprimirAviso(log_memoria,"[RESOLVIENDO SELECT] No reconozco lo que recibi del lfs");
					}
					else{
						resolver_insert(req_lfs,false);
						imprimirMensaje(log_memoria,"[RESOLVIENDO SELECT] Agregué el valor a memoria");
						if(req_lfs.cant_args >= 3){
							valor = malloc(strlen(req_lfs.args[2])+1);
							strcpy(valor,req_lfs.args[2]);
						}
					}
					borrar_request(req_lfs);
				}
				else{
					imprimirAviso(log_memoria,"[RESOLVIENDO SELECT] El lfs no pudo resolver el select");
					borrar_respuesta(resp);
				}
			}
			else{
				borrar_mensaje(msg);
			}
		}
	}
	if(valor != NULL){
		printf("\nValor obtenido: %s",valor);
	}
	return valor;
}

char *resolver_describe(int socket_lfs, request_t req)
{
	char *ret_val;
	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Entro a función");
	req_com_t a_enviar;

	a_enviar.tam = strlen(req.request_str)+1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str,req.request_str);

	imprimirMensaje1(log_memoria, "[RESOLVIENDO DESCRIBE] Voy a enviar request a lfs: %s",a_enviar.str);
	if(enviar_request(socket_lfs,a_enviar) == -1){
		imprimirError(log_memoria,"[RESOLVIENDO DESCRIBE] Error al enviar el request");
		borrar_request_com(a_enviar);
		return NULL;
	}
	borrar_request_com(a_enviar);

	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Request enviado. Esperando respuesta");

	msg_com_t msg = recibir_mensaje(socket_lfs);
	if(msg.tipo != RESPUESTA){
		imprimirError(log_memoria,"[RESOLVIENDO DESCRIBE] El lfs no responde como se espera");
		borrar_mensaje(msg);
		return NULL;
	}

	imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] Respuesta recibida");
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);
	if(resp.tipo == RESP_OK){
		imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] El lfs pudo completar el pedido OK");
		if(resp.msg.tam == 0){
			imprimirMensaje(log_memoria, "[RESOLVIENDO DESCRIBE] La respuesta vino vacía");
			ret_val = NULL;
		}
		else{
			imprimirMensaje1(log_memoria, "[RESOLVIENDO DESCRIBE] La respuesta del lfs fue: %s",resp.msg.str);
			ret_val = malloc(resp.msg.tam);
			strcpy(ret_val,resp.msg.str);
		}
		borrar_respuesta(resp);
	}
	else{
		char tipo_error[10];
		snprintf(tipo_error,9,"%d",resp.tipo);
		imprimirError1(log_memoria, "[RESOLVIENDO DESCRIBE] El lfs NO pudo completar el pedido. Error: %s",tipo_error);
		borrar_respuesta(resp);
		return NULL;
	}
	return ret_val;
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
