/*
 * Gossiping.c
 *
 *  Created on: 21 jun. 2019
 *      Author: martin
 */
#include "Gossiping.h"

time_gos_t ahora(void);

pthread_mutex_t gossip_mutex = PTHREAD_MUTEX_INITIALIZER; //Para que no se ejecute 2 veces a la vez
pthread_mutex_t gossip_table_mutex = PTHREAD_MUTEX_INITIALIZER; //Para proteger las consultas del vector de seeds
pthread_mutex_t gossip_retardo_mutex = PTHREAD_MUTEX_INITIALIZER; //Para poder actualizar el retardo sin romper nada
pthread_mutex_t gossip_caidas_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gossip_cambios_mutex = PTHREAD_MUTEX_INITIALIZER;

t_list *g_lista_seeds;
t_list *g_memorias_caidas;
t_log *logger_gossiping = NULL;
bool gossiping_inicializado = false;
time_gos_t g_retardo_gos;
bool g_hubo_cambios = false;

time_gos_t proxima_ejecucion_gossiping(time_gos_t ultimo);


void inicializar_estructuras_gossiping(t_log *logger, time_gos_t retardo)
{
	if(gossiping_inicializado == false){
		logger_gossiping = logger;
		g_lista_seeds = list_create();
		g_memorias_caidas = list_create();
		g_retardo_gos = retardo;
		gossiping_inicializado = true;
	}
}

void actualizar_retardo_gossiping(time_gos_t retardo)
{
	imprimirMensaje(logger_gossiping, "[ACTUALIZANDO RETARDO GOSSIPING] Voy a actualizar retardo");
	pthread_mutex_lock(&gossip_retardo_mutex);
	g_retardo_gos = retardo;
	pthread_mutex_unlock(&gossip_retardo_mutex);
	imprimirMensaje1(logger_gossiping, "[ACTUALIZANDO RETARDO GOSSIPING] Retardo actualizado a %d milisegundos", retardo);
}

time_gos_t proxima_ejecucion_gossiping(time_gos_t ultimo)
{
	time_gos_t proximo;
	time_gos_t actual = ahora();
	pthread_mutex_lock(&gossip_retardo_mutex);
	proximo = ultimo + g_retardo_gos;
	if(proximo < actual)
		proximo = actual;
	pthread_mutex_unlock(&gossip_retardo_mutex);
	return proximo;
}

void liberar_memoria_gossiping(void)
{
	list_clean_and_destroy_elements(g_lista_seeds,(void*)free);
	gossiping_inicializado = false;
	list_clean_and_destroy_elements(g_memorias_caidas,(void*)free);
}

void agregar_seed(int nro_mem, char* ip, char *puerto)
{
	seed_com_t *aux = malloc(sizeof(seed_com_t));
	aux->numMemoria = nro_mem;
	strcpy(aux->ip,ip);
	strcpy(aux->puerto,puerto);
	list_add(g_lista_seeds,aux);
}

void incorporar_seeds_gossiping(gos_com_t nuevas)
{
	seed_com_t *nueva_aux;
	int i_mem;
	for(int i=0; i<nuevas.cant; i++){
		i_mem = conozco_memoria(nuevas.seeds[i]);
		if(i_mem == -1){
			nueva_aux = malloc(sizeof(seed_com_t));
			if(i==0){ //Solo doy de alta a la memoria con la que me estoy conectando
				nueva_aux->numMemoria = nuevas.seeds[i].numMemoria;
				pthread_mutex_lock(&gossip_cambios_mutex);
				g_hubo_cambios = true;
				pthread_mutex_unlock(&gossip_cambios_mutex);
			}
			else
				nueva_aux->numMemoria = -1;
			strcpy(nueva_aux->ip,nuevas.seeds[i].ip);
			strcpy(nueva_aux->puerto,nuevas.seeds[i].puerto);
			pthread_mutex_lock(&gossip_table_mutex);
			list_add(g_lista_seeds,nueva_aux);
			pthread_mutex_unlock(&gossip_table_mutex);
			log_info(logger_gossiping,"[INCORPORAR SEEDS] Nueva memoria conocida. %d-%s-%s",nuevas.seeds[i].numMemoria,nuevas.seeds[i].ip,nuevas.seeds[i].puerto);
		}
		else if(i==0){ //Solo doy de alta a la memoria con la que me estoy conectando
			pthread_mutex_lock(&gossip_table_mutex);
			seed_com_t *aux = list_get(g_lista_seeds,i_mem);
			if(aux->numMemoria == -1 && nuevas.seeds[i].numMemoria != -1){
				aux->numMemoria = nuevas.seeds[i].numMemoria;
				log_info(logger_gossiping,"[INCORPORAR SEEDS] Memoria de alta en el pool. %d-%s-%s\n",nuevas.seeds[i].numMemoria,nuevas.seeds[i].ip,nuevas.seeds[i].puerto);
				pthread_mutex_lock(&gossip_cambios_mutex);
				g_hubo_cambios = true;
				pthread_mutex_unlock(&gossip_cambios_mutex);
			}
			else{
				log_info(logger_gossiping,"[INCORPORAR SEEDS] Memoria ya conocida. %d-%s-%s\n",nuevas.seeds[i].numMemoria,nuevas.seeds[i].ip,nuevas.seeds[i].puerto);
			}
			pthread_mutex_unlock(&gossip_table_mutex);
		}
	}
}

/*
gos_com_t memoriasDispoinbles;

actualizar()
{
	gos_com_t caidas = obtenerCaidas();
	if(caidas.cant > 0){

	}

	if(huboCambios()){
		memoriasDispoinbles = armar_vector_seeds(KERNEL);
	}
}*/

gos_com_t armar_vector_seeds(id_com_t id_proceso)
{
	pthread_mutex_lock(&gossip_table_mutex);
	gos_com_t gos_tabla;
	seed_com_t *aux;
	gos_tabla.cant = list_size(g_lista_seeds);
	gos_tabla.seeds = malloc(sizeof(seed_com_t)*gos_tabla.cant);
	int activas = 0;
	for(int i=0; i<gos_tabla.cant;i++)
	{
		aux = list_get(g_lista_seeds,i);
		if(id_proceso == KERNEL && aux->numMemoria == -1)
			continue;
		gos_tabla.seeds[activas] = *aux;
		activas++;
	}
	pthread_mutex_unlock(&gossip_table_mutex);
	if(activas < gos_tabla.cant){
		gos_tabla.cant = activas;
		gos_tabla.seeds = realloc(gos_tabla.seeds,sizeof(seed_com_t)*gos_tabla.cant);
	}
	return gos_tabla;
}

int conozco_memoria(seed_com_t memoria)
{
	seed_com_t *aux;
	int retval = -1;
	pthread_mutex_lock(&gossip_table_mutex);
	for(int i=0; i<list_size(g_lista_seeds);i++){
		aux = list_get(g_lista_seeds,i);
		if(strcmp(aux->ip,memoria.ip)==0 && strcmp(aux->puerto,memoria.puerto)==0){
			retval = i;
			break;
		}
	}
	pthread_mutex_unlock(&gossip_table_mutex);
//	printf("No la conozco\n");
	return retval;
}

t_list* lista_seeds(void){
	t_list* aux;
	pthread_mutex_lock(&gossip_table_mutex);
	aux = list_duplicate(g_lista_seeds);
	pthread_mutex_unlock(&gossip_table_mutex);
	return aux;
}

void borrar_seed(seed_com_t *memoria)
{
	seed_com_t *aux;
	for(int i=0; i<list_size(g_lista_seeds);i++){
		aux = list_get(g_lista_seeds,i);
		if(strcmp(aux->ip,memoria->ip)!=0 && strcmp(aux->puerto,memoria->puerto)!=0){
			log_info(logger_gossiping,"Seed %d-%s-%s borrado",memoria->numMemoria,memoria->ip,memoria->puerto);
			list_remove(g_lista_seeds,i);
			break;
		}
	}
}

void registrar_memoria_caida(int i_mem)
{
	pthread_mutex_lock(&gossip_table_mutex);
	seed_com_t *aux = list_get(g_lista_seeds,i_mem);

	if(aux->numMemoria != -1){
		seed_com_t *caida = malloc(sizeof(seed_com_t));
		memcpy(caida,aux,sizeof(seed_com_t));
		pthread_mutex_lock(&gossip_caidas_mutex);
		list_add(g_memorias_caidas,caida);
		pthread_mutex_unlock(&gossip_caidas_mutex);

		pthread_mutex_lock(&gossip_cambios_mutex);
		g_hubo_cambios = true;
		pthread_mutex_unlock(&gossip_cambios_mutex);
	}
	aux->numMemoria = -1;
	pthread_mutex_unlock(&gossip_table_mutex);
}

void correr_gossiping(id_com_t id_proceso)
{
	imprimirMensaje(logger_gossiping,"[CORRIENDO GOSSIPING] Iniciando proceso de gossiping");

	handshake_com_t hs;
	msg_com_t msg;
	gos_com_t nuevas;

//	pthread_mutex_lock(&gossip_mutex);

	pthread_mutex_lock(&gossip_table_mutex);
	//Me guardo las memorias conocidas hasta el momento. Son con las que haré gossiping ahora
	t_list *copia_seeds = list_duplicate(g_lista_seeds);
	pthread_mutex_unlock(&gossip_table_mutex);

	gos_com_t conocidas;

	//Me armo un vector con las memorias conocidas que presentaré a las demás memorias
	if(id_proceso == MEMORIA)
		conocidas = armar_vector_seeds(id_proceso);
	else{
		//El kernel no le presenta a las memorias las que él conoce
		conocidas.seeds = NULL;
		conocidas.cant = 0;
	}
	imprimirMensaje(logger_gossiping,"[CORRIENDO GOSSIPING] Empiezo a conectarme con las memorias que conozco");
	int i;
	if(id_proceso == KERNEL)
		i=0;
	else
		i=1;//La primera memoria siempre soy yo misma
	for(; i<list_size(copia_seeds); i++){
		seed_com_t *memoria = list_get(copia_seeds,i);

		//Me conecto a la memoria
		imprimirMensaje2(logger_gossiping,"[CORRIENDO GOSSIPING] Voy a conectarme a <%s>-<%s>",memoria->ip,memoria->puerto);
		int conexion = conectar_a_servidor(memoria->ip,memoria->puerto,id_proceso);
		if(conexion==-1){
			log_info(logger_gossiping,"[CORRIENDO GOSSIPING] La memoria <%d>-<%s>-<%s> está caida",memoria->numMemoria,memoria->ip,memoria->puerto);
			//Tengo que borrarla de la lista
			//borrar_seed(memoria);
			registrar_memoria_caida(i);
			continue;
		}

		//Recibo el hs de la memoria para ver si me acepta
		msg = recibir_mensaje(conexion);
		if(msg.tipo != HANDSHAKECOMANDO){
			log_info(logger_gossiping,"[CORRIENDO GOSSIPING] La memoria no responde como se espera");
			close(conexion);
			continue;
		}

		hs = procesar_handshake(msg);
		borrar_mensaje(msg);
		if(hs.id == RECHAZADO){
			log_info(logger_gossiping,"[CORRIENDO GOSSIPING] La memoria rechazo la conexion");
			close(conexion);
			continue;
		}
		borrar_handshake(hs);
		log_info(logger_gossiping,"[CORRIENDO GOSSIPING] La memoria acepto la conexion");

		//Le envío las memorias que conozco
		enviar_gossiping(conexion,conocidas);

		//Espero su respuesta
		msg = recibir_mensaje(conexion);
		if(msg.tipo != GOSSIPING){
			log_info(logger_gossiping,"[CORRIENDO GOSSIPING] La memoria no responde como se espera");
			close(conexion);
			continue;
		}

		nuevas = procesar_gossiping(msg);
		borrar_mensaje(msg);

		incorporar_seeds_gossiping(nuevas);
		log_info(logger_gossiping,"[CORRIENDO GOSSIPING] Se agregaron memorias recibidas");
		borrar_gossiping(nuevas);

		close(conexion);
	}
	borrar_gossiping(conocidas);
//	pthread_mutex_unlock(&gossip_mutex);
	list_destroy(copia_seeds);
}

int iniciar_hilo_gossiping(id_com_t *id_proceso, pthread_t *thread, void (*funcion_actualizacion) (void))
{
	thread_gos_args_t *args = malloc(sizeof(thread_gos_args_t));
	args->id_proceso = *id_proceso;
	args->funcion = funcion_actualizacion;
	if(gossiping_inicializado == false){
		imprimirAviso(logger_gossiping,"[INICIANDO HILO GOSSIPING] Debe inicializar las variables de gossiping. No se lanzará el hilo");
		return -1;
	}
	imprimirMensaje(logger_gossiping,"[INICIANDO HILO GOSSIPING] Voy a crear hilo");
	pthread_create(thread,NULL,(void*)hilo_gossiping,args);
	pthread_detach(*thread);
	imprimirMensaje(logger_gossiping,"[INICIANDO HILO GOSSIPING] Hilo creado");
	return 1;
}

void *hilo_gossiping(thread_gos_args_t *args_p)
{
	thread_gos_args_t args = *args_p;
	id_com_t *id_proceso = &(args.id_proceso);
	free(args_p);
	imprimirMensaje(logger_gossiping,"[HILO GOSSIPING] Entrando a hilo");
//	log_info(logger_gossiping,"[HILO GOSSIPING] Soy proceso %d",*id_proceso);
	//time_gos_t t0 = ahora(), t1 = 0;
	time_gos_t proximo, ultimo = 0;
	proximo = proxima_ejecucion_gossiping(ultimo);
	while(1){
		imprimirMensaje(logger_gossiping,"[HILO GOSSIPING] Voy a correr gossiping");
		correr_gossiping(*id_proceso);
		ultimo = ahora();
		imprimirMensaje(logger_gossiping,"[HILO GOSSIPING] Terminé de ejecutar gossiping.");

		imprimirMensaje(logger_gossiping,"[HILO GOSSIPING] Voy a correr función de actualización");
		args.funcion();
		imprimirMensaje(logger_gossiping,"[HILO GOSSIPING] Corrí función de actualización");
		proximo = proxima_ejecucion_gossiping(ultimo);
		do{
			imprimirMensaje1(logger_gossiping,"[HILO GOSSIPING] La próxima ejecución será en %d milisegundos",proximo-ahora());
			usleep((proximo-ahora())*1000);
			proximo = proxima_ejecucion_gossiping(ultimo);
		}while(proximo>ahora());
	}
	return NULL;
}

t_list *hayMemoriasCaidas(void)
{
	t_list *retval;
	pthread_mutex_lock(&gossip_caidas_mutex);
	retval = list_duplicate(g_memorias_caidas);
	list_clean(g_memorias_caidas);
	pthread_mutex_unlock(&gossip_caidas_mutex);
	return retval;
}

bool huboCambios(void)
{
	bool retval;
	pthread_mutex_lock(&gossip_cambios_mutex);
	retval = g_hubo_cambios;
	g_hubo_cambios = false;
	pthread_mutex_unlock(&gossip_cambios_mutex);
	return retval;
}

time_gos_t ahora(void)
{
	time_gos_t retval;
	struct timeval tv;
	if(gettimeofday(&tv,NULL) == -1)
		printf("Error\n");
	retval = tv.tv_sec*1000 + (time_gos_t) tv.tv_usec/1000;
	return retval;
}
