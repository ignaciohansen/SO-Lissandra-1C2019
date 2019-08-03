/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"
#include "kernel_aux.h"

pthread_mutex_t lista_tablas_mutex = PTHREAD_MUTEX_INITIALIZER;
t_list *g_lista_tablas = NULL;

pthread_mutex_t lista_memorias_asociadas_mutex = PTHREAD_MUTEX_INITIALIZER;
t_list *g_lista_memorias_asociadas = NULL;

pthread_mutex_t memorias_conocidas_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *fp_trace_ejecucion;

//Agregue esto (lorenzo)
pthread_mutex_t lista_memorias_criterio_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {

	fp_trace_ejecucion = fopen("../Log/traceEjecucion.txt", "w"); //Para revisar si estamos haciendo bien la planificacion

	log_kernel = archivoLogCrear(LOG_PATH, "[MAIN]- Proceso Kernel.");

	log_info(log_kernel,"[MAIN]- Por cargar configuracion desde archivo.");

	cargarConfiguracion();

	log_info(log_kernel, "[MAIN]- La carga de archivo de configuracion finalizo.");

	iniciarSemaforos();

	inicializarListasPlanificador();
	lista_memorias = list_create();

	srand(timestamp()); //Para elegir aleatoriamente las memorias
	g_lista_memorias_asociadas = list_create();
	criterioSC.listMemorias = list_create();
	criterioSHC.listMemorias = list_create();
	criterioEC.listMemorias = list_create();
	memoriasConocidasKernel.cant = 0;
	memoriasConocidasKernel.seeds = NULL;

	mutexIniciar(&countProcess);
	mutexIniciar(&mutexColaNuevos);

	countPID = 0;

	mutexIniciar(&mutex_retardos_kernel);

	char* path_de_kernel = malloc(strlen(PATH_KERNEL_CONFIG) + 1);
	strcpy(path_de_kernel, PATH_KERNEL_CONFIG);
	pthread_t inotify_c;
	pthread_create(&inotify_c, NULL, (void *) inotifyAutomatico,path_de_kernel);
	pthread_detach(inotify_c);
	log_info(log_kernel,"[MAIN]- Hilo de actualización de retardos y Quantum corriendo");

	pthread_t hiloGossiping;
	gossiping_Kernel(&hiloGossiping);
	pthread_detach(hiloGossiping);

	log_info(log_kernel,"[MAIN]- Creamos hilo para actualizar la metadata de las tablas");
	pthread_t hiloMetadataRefresh;
	pthread_create(&hiloMetadataRefresh, NULL, (void*) hilo_metadata_refresh,
	NULL);
	pthread_detach(hiloMetadataRefresh);

	log_info(log_kernel, "[MAIN]- Creamos hilo para Consola.");
	pthread_t* hilosPlanificador = iniciarHilosMultiprocesamiento(arc_config->multiprocesamiento);

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);

	log_info(log_kernel, "[MAIN]- Creamos hilo para Metricas.");
	pthread_t hiloMetricas;
	pthread_create(&hiloMetricas,NULL,correrHiloMetricas,NULL);
	pthread_detach(hiloMetricas);

	pthread_join(hiloConsola, NULL);

	pthread_cancel(hiloMetricas);
	pthread_cancel(hiloMetadataRefresh);
	pthread_cancel(hiloGossiping);
	pthread_cancel(inotify_c);

	for(int i=0;i<arc_config->multiprocesamiento;i++){
		pthread_cancel(hilosPlanificador[i]);
	}
	free(hilosPlanificador);

	fclose(fp_trace_ejecucion);

	free(path_de_kernel);

	liberarTodo();

//	log_info(log_kernel, "Salimoooos, fin del main.");

	return 0;

}

void liberarTodo(void){
	log_info(log_kernel, "[FINALIZANDO] Liberamos memoria");

	log_info(log_kernel, "[FINALIZANDO] Vacio y borro colas");
	vaciarColas();

	log_info(log_kernel, "[FINALIZANDO] Borro memoria utilizada por el gossiping");
	liberar_memoria_gossiping();

	log_info(log_kernel, "[FINALIZANDO] Borro lista de tablas");
	pthread_mutex_lock(&lista_tablas_mutex);
	if (g_lista_tablas != NULL) {
		list_destroy_and_destroy_elements(g_lista_tablas,(void *) borrarEntradaListaTablas);
	}
	pthread_mutex_unlock(&lista_tablas_mutex);

	log_info(log_kernel, "[FINALIZANDO] Borro criterios");
	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
	for (int i = 0; i < list_size(g_lista_memorias_asociadas); i++) {
		seed_com_t *aux = list_get(g_lista_memorias_asociadas, i);
		list_remove(g_lista_memorias_asociadas, i);
		free(aux);
	}
	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);
	list_destroy_and_destroy_elements(criterioEC.listMemorias,free);
	list_destroy_and_destroy_elements(criterioSHC.listMemorias,free);
	list_destroy_and_destroy_elements(criterioSC.listMemorias,free);

	log_info(log_kernel, "[FINALIZANDO] Borro estructura de metricas");
	t_metricas_memoria *aux;
	pthread_mutex_lock(&mutex_metricas);
	for(int i=0; i<list_size(g_metricas.operaciones_memorias);i++){
		aux = list_get(g_metricas.operaciones_memorias,i);
		list_remove(g_metricas.operaciones_memorias,i);
		free(aux);
	}
	pthread_mutex_unlock(&mutex_metricas);

	log_info(log_kernel, "[FINALIZANDO] Borro archivo config");
	free(arc_config->ip_memoria);
	free(arc_config);

	log_info(log_kernel, "[FINALIZANDO] Borro log");
	log_destroy(log_kernel);
}

void vaciarColas(void)
{
	list_destroy_and_destroy_elements(colaEjecucion, (void*)borrarPCB);
	list_destroy_and_destroy_elements(colaExit, (void*)borrarPCB);
	list_destroy_and_destroy_elements(colaListos, (void*)borrarPCB);
	list_destroy_and_destroy_elements(colaNuevos, free);
}

void borrarPCB(t_pcb *pcb)
{
	free(pcb->linea);
	free(pcb);
}

pthread_t* iniciarHilosMultiprocesamiento(int nivel) {

	pthread_t* hilosPlanificador = malloc(sizeof(pthread_t) * nivel);

	for (int i = 0; i < nivel; i++) {

		int *aux_nivel = malloc(sizeof(int));
		memcpy(aux_nivel, &i, sizeof(int));
		pthread_create(&hilosPlanificador[i], NULL,	(void*) nivelMultiprogramacion, aux_nivel);

		pthread_detach(hilosPlanificador[i]);

	}

	return hilosPlanificador;

}

void iniciarSemaforos() {

	semaforoIniciar(&sem_planificador, 0);
}

void cargarConfiguracion() {

	log_info(log_kernel,"[CARGAR CONFIGURACION]-Por reservar memoria para variable de configuracion.");

	arc_config = malloc(sizeof(t_kernel_config));

	t_config* configFile;

	log_info(log_kernel,"[CARGAR CONFIGURACION]-Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_KERNEL_CONFIG);

	if (configFile != NULL) {

		log_info(log_kernel, "[CARGAR CONFIGURACION]- Leyendo Archivo de Configuracion...");

		if (config_has_property(configFile, "PUERTO_MEMORIA")) {

			log_info(log_kernel, "[CARGAR CONFIGURACION]-Almacenando el puerto");

			arc_config->puerto_memoria = config_get_int_value(configFile,"PUERTO_MEMORIA");

			log_info(log_kernel, "[CARGAR CONFIGURACION]- El puerto de la memoria es: %d",arc_config->puerto_memoria);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no contiene el PUERTO de la Memoria");
		}

		if (config_has_property(configFile, "IP_MEMORIA")) {

			log_info(log_kernel, "[CARGAR CONFIGURACION]- Almacenando la IP de la Memoria");

			char *aux = config_get_string_value(configFile,"IP_MEMORIA");

			arc_config->ip_memoria = malloc(strlen(aux)+1);
			strcpy(arc_config->ip_memoria, aux);

			log_info(log_kernel, "[CARGAR CONFIGURACION]- La Ip de la memoria es: %s",arc_config->ip_memoria);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no contiene la IP de la Memoria");

		}

		if (config_has_property(configFile, "QUANTUM")) {

			log_info(log_kernel, "[CARGAR CONFIGURACION]- Almancenando el Quantum del planificador");

			arc_config->quantum = config_get_int_value(configFile, "QUANTUM");

			log_info(log_kernel, "[CARGAR CONFIGURACION]- El Quantum del planificador es: %d", arc_config->quantum);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no contiene el Quantum del planificador");

		}

		if (config_has_property(configFile, "MULTIPROCESAMIENTO")) {

			log_info(log_kernel,"[CARGAR CONFIGURACION]- Almacenando el valor del Multiprocesamiento para el Planificador");

			arc_config->multiprocesamiento = config_get_int_value(configFile,"MULTIPROCESAMIENTO");

			log_info(log_kernel,"[CARGAR CONFIGURACION]- El grado de multiprocesamiento del planificador es: %d",arc_config->multiprocesamiento);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no el grado de multiprocesamiento del planificador");

		}

		if (config_has_property(configFile, "METADATA_REFRESH")) {

			log_info(log_kernel,"[CARGAR CONFIGURACION]- Almacenando el valor del Metadata Refresh para el Kernel");

			arc_config->metadata_refresh = config_get_int_value(configFile,"METADATA_REFRESH");

			log_info(log_kernel, "[CARGAR CONFIGURACION]- El valor del Metadata Refresh es: %d",arc_config->metadata_refresh);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no tiene el valor del Metadata refresh");

		}

		if (config_has_property(configFile, "SLEEP_EJECUCION")) {

			log_info(log_kernel,"[CARGAR CONFIGURACION]- Almacenando el valor del Sleep Ejecucion para el Kernel");

			arc_config->sleep_ejecucion = config_get_int_value(configFile,"SLEEP_EJECUCION");

			log_info(log_kernel, "[CARGAR CONFIGURACION]- El valor del Sleep Ejecucion es: %d",	arc_config->sleep_ejecucion);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no tiene el valor del Sleep Ejecucion");

		}
		if (config_has_property(configFile, "RETARDO_GOSSIPING")) {

			log_info(log_kernel,"[CARGAR CONFIGURACION]- Almacenando el valor del Retardo de Gossiping para el Kernel");

			arc_config->retardo_gossiping = config_get_int_value(configFile,"RETARDO_GOSSIPING");

			log_info(log_kernel, "[CARGAR CONFIGURACION]- El valor del Retardo de Gossiping es: %d",arc_config->retardo_gossiping);

		} else {

			log_error(log_kernel,"[CARGAR CONFIGURACION]- El archivo de configuracion no tiene el valor del Retardo de Gossiping. Se utiliza 20000 milisegundos");

			arc_config->retardo_gossiping = 20000;

		}

	} else {

		log_error(log_kernel,"[CARGAR CONFIGURACION]- No se encontro el archivo de configuracion para cargar la estructura de Kernel");

	}

	log_info(log_kernel,"[CARGAR CONFIGURACION]- Cargamos todo lo que se encontro en el archivo de configuracion. Liberamos la variable config que fue utlizada para navegar el archivo de configuracion");

	/*free(configFile->path);
	int i;
	t_hash_element* nextHash;
	for (i = 0; i < configFile->properties->elements_amount; i++) {
		while (configFile->properties->elements[i] != NULL) {
			nextHash = configFile->properties->elements[i]->next;
			free(configFile->properties->elements[i]->data);
			free(configFile->properties->elements[i]->key);

			configFile->properties->elements[i] = nextHash;
		}
		free(configFile->properties->elements[i]);
	}
	free(configFile->properties);
	free(configFile);*/
	config_destroy(configFile);

}

void consola() {

	log_info(log_kernel, "[CONSOLA]- En el hilo de consola.");

	menu();

	while (1) {

		linea = readline("\n>");

		if (linea) {
			add_history(linea);
			comandoSeparado = string_split(linea, separator);
		}

		if (!strncmp(linea, "SALIR", 4)) {
			log_info(log_kernel, "[CONSOLA]- Viene el comando en la cadena: %s",	comandoSeparado[0]);
			free(linea);
			int i = 0;
			while(comandoSeparado[i]!=NULL){
				free(comandoSeparado[i]);
				i++;
			}
			free(comandoSeparado);
			break;
		}

		strtok(linea, "\n");

		log_info(log_kernel, "[CONSOLA]- Viene el comando en la cadena: %s", comandoSeparado[0]);

		int comando = buscarComando(comandoSeparado[0]);

		log_info(log_kernel, "[CONSOLA]- El Enum correspondiente para el comando es: %d",comando);
		request_t req;

		switch (comando) {
		case ADD:
			req = parser(linea);
			if(req.cant_args != 4){
				printf("Comando mal ingresado. Debe ser: ADD MEMORY <NUM> TO <CRITERIO>\n");
				log_warning(log_kernel,"Comando mal ingresado. Debe ser: ADD MEMORY <NUM> TO <CRITERIO>\n");
			}
			else{
				printf("Vino ADD.\n");
				log_info(log_kernel, "[CONSOLA]- ADD ingresado.");
				comandoAdd(comandoSeparado);
			}
			free(linea);
			borrar_request(req);
			break;
		case JOURNAL:
			printf("Vino journal.\n");
			log_info(log_kernel, "[CONSOLA]- Journal ingresado.");
			//comandoJournal(comandoSeparado);
			request_t request = parser(linea);
			free(linea);
			resp_com_t resp = resolverJournalGlobal(request);
			borrar_respuesta(resp);
			borrar_request(request);
			break;
		case METRICS:
			printf("Vino metrics.\n");
			log_info(log_kernel, "[CONSOLA]- Metrics ingresado.");
			free(linea);
			comandoMetrics();
			break;
		case SALIR:
			printf("Salimos de la consola y el proceso!.\n");
			log_info(log_kernel, "[CONSOLA]- Salir ingresado. Cerramos todo");
			free(linea);
			int i = 0;
			while(comandoSeparado[i]!=NULL){
				free(comandoSeparado[i]);
				i++;
			}
			free(comandoSeparado);
			return;
		case INSERT:
		case SELECT:
		case DESCRIBE:
		case DROP:
		case CREATE:
		case RUN:
			log_info(log_kernel, "[CONSOLA]- Entramos por comandoPlanificable (insert,select,describe,drop,create,run), a Planificar");
			strtok(linea, "\n");
			planificadorLargoPlazo(linea);
			break;
		default:
			printf("[CONSOLA]- Comando mal ingresado.");
			free(linea);
			break;
		}

		int i=0;
		while(comandoSeparado[i]!=NULL){
			free(comandoSeparado[i]);
			i++;
		}
		free(comandoSeparado);

		//free(linea);

	}
}

void menu() {

	printf("Los comandos que se pueden ingresar son: \n"
			"Insert \n"
			"Select \n"
			"Create \n"
			"Describe \n"
			"Drop \n"
			"Journal  \n"
			"add \n"
			"run \n"
			"metrics \n"
			"SALIR \n"
			"\n");
}
int buscarComando(char* comando) {

	if (comando == NULL) {
		log_info(log_kernel, "[buscarComando]- Recibimos el comando: NULL");
		return -1;
	}

	log_info(log_kernel, "[buscarComando]- Recibimos el comando: %s", comando);

	int i = 0;

	for (i; i <= SALIR && strcmp(comandosPermitidos[i], comando); i++) {
	}

	log_info(log_kernel, "[buscarComando]- Se devuelve el valor %d", i);

	return i;

}

void inicializarListasPlanificador(void) {

	colaNuevos = list_create();
	colaListos = list_create();
	colaExit = list_create();
	colaEjecucion = list_create();
	g_lista_tablas = list_create();
}

t_pcb* crearPcb(char* linea) {

	log_info(log_kernel, "[crearPcb]- Creando PCB ==> PID: %d", countPID);

	t_pcb* pcbProceso = malloc(sizeof(t_pcb));

	pcbProceso->linea = linea;

	comandoSeparado = string_split(linea, separator);

	for (int i = 0; comandoSeparado[i] != NULL; i++) {

		log_info(log_kernel, "[crearPcb]- En la posicion %d del array esta el valor %s", i, 	comandoSeparado[i]);

		tamanio = i + 1;
	}

	int auxComandoInt = -1;

	if (strcmp(comandoSeparado[0], "RUN") == 0) {

		auxComandoInt = RUN;

	} else if (strcmp(comandoSeparado[0], "ADD") == 0) {

		auxComandoInt = ADD;

	} else if (strcmp(comandoSeparado[0], "SELECT") == 0) {

		auxComandoInt = SELECT;

	} else if (strcmp(comandoSeparado[0], "INSERT") == 0) {

		auxComandoInt = INSERT;

	} else if (strcmp(comandoSeparado[0], "CREATE") == 0) {

		auxComandoInt = CREATE;
	} else if (strcmp(comandoSeparado[0], "DESCRIBE") == 0) {

		auxComandoInt = DESCRIBE;
	}

	switch (auxComandoInt) {

	case RUN: {

		log_info(log_kernel,"[crearPcb]- Vino Run de comando, calculamos cuantas lineas tiene el archivo para llenar PCB.");
		int aux_rafaga = rafagaComandoRun(comandoSeparado[1]);

		log_info(log_kernel, "[crearPcb]- La rafaga del run es: %d", aux_rafaga);
		pcbProceso->pid = countPID;
		pcbProceso->comando = auxComandoInt;
		pcbProceso->rafaga = aux_rafaga;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estado en la cola new porque recien se crea
		pcbProceso->progamCounter = 0;
		pcbProceso->archivo = fopen(comandoSeparado[1], "r");
		log_info(log_kernel, "[crearPcb]- EN PCB: %s", comandoSeparado[1]);
		log_info(log_kernel, "[crearPcb]- EN PCB: %p", pcbProceso->archivo);
		log_info(log_kernel, "[crearPcb]- Terminamos de completar PCB para RUN.");

	}
		break;
	default: {

		log_info(log_kernel, "[crearPcb]- En la condicion de que no es un comando RUN");
		pcbProceso->pid = countPID;
		pcbProceso->comando = auxComandoInt;
		pcbProceso->rafaga = 1;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estado en la cola new porque recien se crea
		pcbProceso->progamCounter = 0;
		log_info(log_kernel, "[crearPcb]- Terminamos de completar PCB para comando que no es RUN.");

	}
		break;

	}

	log_info(log_kernel, "[crearPcb]- Retornamos direccion del PCB en:  %p.",pcbProceso);

	return pcbProceso;
}

int rafagaComandoRun(char* path) {

	log_info(log_kernel,"[rafagaComandoRun]- Recorremos para calcular la cantidad de lineas que tiene el archivo.");
	int caracter, contador;

	contador = 0;

	FILE* fd;

	fd = fopen(path, "r");

	if (fd == NULL) {

		log_info(log_kernel, "[rafagaComandoRun]- El archivo pasado por path no se encontró");
		printf("El archivo %s No fue encontrado!\n", path);

		//free(path);
		return -1;
	} else {

		log_info(log_kernel,"[rafagaComandoRun]- El archivo se encontró con exito. Empezamos a leer para calcular la cantidad de lineas.");
		printf("El archivo buscado en la dirección %s fue encontrato. Vamos a leerlo.\n",
				path);

		while ((caracter = fgetc(fd)) != EOF) {

			if (caracter == '\n') {

				contador++;
			}

		}

		log_info(log_kernel,"[rafagaComandoRun]- Fuera del while principal, la cantidad de lineas del archivo es: %d",contador);
		rewind(fd);
		fclose(fd);
		//free(path);

		log_info(log_kernel, "[rafagaComandoRun]- Por retornar contador");
		return contador;
	}

	return 0;
}

void nivelMultiprogramacion(int* este_nivel) {

	int nivel = *este_nivel;
	log_info(log_kernel, "[nivelMultiprogramacion]- Entrando a nivel %d de multiprogramacion",*este_nivel);
	free(este_nivel);

	while (1) {
		t_pcb* pcb = planificarCortoPlazo();
		log_info(log_kernel, "[nivelMultiprogramacion]- Ejecutando el nivel: %d", nivel);
		ejecutar(pcb, arc_config->quantum, nivel);
	}
}

void planificadorLargoPlazo(char* linea) {

	agregarANuevo(linea);

	t_pcb* pcbProceso = crearEstructurasAdministrativas(linea);

	if (pcbProceso == NULL) {

		printf("Hubo un error al crear las estructuras administrativas");

		log_error(log_kernel,"[planificadorLargoPlazo]- Hubo un error al crear las estructuras administrativas");

		return;
	}

	log_info(log_kernel,"[planificadorLargoPlazo]- Por llamar a agregarAListo para pasarle el PCB creado.");

	agregarAListo(pcbProceso);
}

t_pcb* planificarCortoPlazo() {

	sem_wait(&sem_planificador);
	t_pcb* pcb = obtenerColaListos();

	agregarAEjecutando(pcb);

	return pcb;
}

void agregarANuevo(char* linea) {

	log_info(log_kernel, "[COLA | agregarANuevo]- Por bloquear Mutex de Cola Nuevos.");
	mutexBloquear(&mutexColaNuevos);
	list_add(colaNuevos, linea);
	mutexDesbloquear(&mutexColaNuevos);

	log_info(log_kernel,"[COLA | agregarANuevo]- Se agrego a la cola la linea %s: .",linea);
	log_info(log_kernel, "[COLA | agregarANuevo]- Size colaNuevos: %d", list_size(colaNuevos));

}

void agregarAListo(t_pcb* pcbParaAgregar) {

	if (pcbParaAgregar->estado == nuevo) {
		log_info(log_kernel, "[COLA | agregarAListo]- Por sacar primer elemento de la cola de nuevos.");

		mutexBloquear(&mutexColaNuevos);

		list_remove(colaNuevos, 0);

		mutexDesbloquear(&mutexColaNuevos);

	}

	log_info(log_kernel,"[COLA | agregarAListo]- Bloqueamos Mutex para poder insertar el elemento en la cola de listos.");

	mutexBloquear(&mutexColaListos);

	pcbParaAgregar->estado = listo;
	list_add(colaListos, pcbParaAgregar);

	for (int i = 0; i < list_size(colaListos); i++) {
		t_pcb *aux = list_get(colaListos, i);
		log_info(log_kernel,"[COLA | agregarAListo]- En la posicion %d de la cola de listos esta el PID: %d",i, aux->pid);
	}

	mutexDesbloquear(&mutexColaListos);

	log_info(log_kernel, "[COLA | agregarAListo]-  Size colaListos luego de agregar PCB: %d", list_size(colaListos));

	log_info(log_kernel, "[COLA | agregarAListo]- Por hacer POST a sem_planificador");
	sem_post(&sem_planificador);

	log_info(log_kernel, "[COLA | agregarAListo]- Salimos de la funcion AgregarAListo");

}

void agregarAEjecutando(t_pcb* pcb) {

	log_info(log_kernel, "[COLA | agregarAEjecutando]- Llego el PID: %d ,para agregar a colaEjecucion.", pcb->pid);
	mutexBloquear(&mutexColaEjecucion);
	list_add(colaEjecucion, pcb);
	pcb->estado = ejecucion;
	mutexDesbloquear(&mutexColaEjecucion);
	log_info(log_kernel, "[COLA | agregarAEjecutando]- Luego de agregar PCB, el Size colaEjecucion: %d", list_size(colaEjecucion));
}

t_pcb* obtenerColaListos(void) {

	t_pcb* pcb = malloc(sizeof(t_pcb));
	mutexBloquear(&mutexColaListos);
	pcb = list_remove(colaListos, 0);
	mutexDesbloquear(&mutexColaListos);
	log_info(log_kernel, "[COLA | obtenerColaListos]- El que voy a ejecutar es el PID: %d",pcb->pid);
	return pcb;
}

t_pcb* crearEstructurasAdministrativas(char* linea) {

	log_info(log_kernel, "[crearEstructurasAdministrativas]- Por llamar a mutexBloquear y aumentar countPID: %d",countPID);

	mutexBloquear(&countProcess);
	countPID++;
	mutexDesbloquear(&countProcess);

	log_info(log_kernel, "[crearEstructurasAdministrativas]- CountPID quedo en: %d",countPID);

	log_info(log_kernel, "[crearEstructurasAdministrativas]- Por llamar a crearPCB");

	t_pcb* proceso;

	proceso = crearPcb(linea);

	log_info(log_kernel, "[crearEstructurasAdministrativas]- PCB creado ==> PID: %d", proceso->pid);

	return proceso;
}

void ejecutar(t_pcb* pcb, int quantum, int nivel) {

	//semaforoWait(&multiprocesamiento);

	log_info(log_kernel, "[EJECUTAR]- Entrando a ejecutar rafaga de PID: %d", pcb->pid);

	if (pcb->comando == RUN && pcb->estado == ejecucion) {

		char* bufferRun = malloc(100);
//		char* bufferRun2 = malloc(100);

		//KEY INexistente

		log_info(log_kernel, "[EJECUTAR]- POR ENTRAR A EJECUTAR QUANTUM CON VALOR: %d",arc_config->quantum);
		//Rafaga restante del PCB sea mayor o igual que el Quantum
		if ((pcb->rafaga - pcb->progamCounter) >= quantum) {

			log_info(log_kernel, "[EJECUTAR]- DENTRO DEL QUANTUM");
			for (int i = 1; quantum >= i; i++) {

				log_info(log_kernel, "[EJECUTAR]- RAFAGA: %d; del PID; %d, nivel: %d", i,pcb->pid,nivel);

				//log_info(log_kernel, "[EJECUTAR]- %d", pcb->comando);
				//log_info(log_kernel, "[EJECUTAR]- %p", pcb->archivo);

//				bufferRun2 = fgets(bufferRun, 100, pcb->archivo);
				bufferRun = fgets(bufferRun, 100, pcb->archivo);

				log_info(log_kernel, "[EJECUTAR]- Linea para ejecutar: %s", bufferRun);

				resp_com_t respuesta = resolverPedido(bufferRun);
				loggearEjecucion(nivel, pcb->pid, bufferRun);

				log_info(log_kernel, "[EJECUTAR]- RESULTADO FUE: %s", respuesta.msg.str);
				pcb->tipoRespuesta = respuesta.tipo;
				pcb->progamCounter++;
				borrar_respuesta(respuesta);
				if(pcb->tipoRespuesta >= RESP_ERROR_PEDIDO_DESCONOCIDO){
					log_warning(log_kernel,"[EJECUTAR]- Entro para exit porque ocurrio el error: %d",pcb->tipoRespuesta);
					agregarAExit(pcb);
					break;
				}

				log_info(log_kernel, "[EJECUTAR]- Por llamar a aplicar Retardo");
				aplicarRetardo();

			}
			log_info(log_kernel, "[EJECUTAR]- FUERA DEL QUANTUM");
			if(pcb->estado == ejecucion){
				if (pcb->rafaga > pcb->progamCounter) {
					sacarDeColaEjecucion(pcb);
					agregarAListo(pcb);
				} else {
					log_info(log_kernel,"[EJECUTAR]- Entro para exit porque termino la ejecucion");
					agregarAExit(pcb);
				}
			}


			/*
			if (pcb->rafaga > pcb->progamCounter && pcb->tipoRespuesta == RESP_OK) {

				sacarDeColaEjecucion(pcb);
				agregarAListo(pcb);

			} else {
				if(pcb->rafaga == pcb->progamCounter){//@martin @lorenzo si no siempre logea que se produjo un error
					log_info(log_kernel,"Entro para exit porque termino la ejecucion");
				}
				else{
					log_warning(log_kernel,"Entro para exit porque ocurrio el error %d",pcb->tipoRespuesta);
				}
				agregarAExit(pcb);
			}*/

		} //Aca termina el si la rafaga restante del proceso (asociado al comando RUN) es mayor que el QUANTUM
		else if (pcb->estado == ejecucion) {

			log_info(log_kernel,"[EJECUTAR]- ===>Seccion de Quantum MAYOR que rafaga restante del proceso.");

			int rafagaRestante = pcb->rafaga - pcb->progamCounter;

			log_info(log_kernel, "[EJECUTAR]- ===> Rafaga restante: %d.", rafagaRestante);

			for (int i = 1; rafagaRestante >= i; i++) {

				log_info(log_kernel, "[EJECUTAR]- Vuelta de EJECUCION: %d.", i);

				bufferRun = fgets(bufferRun, 100, pcb->archivo);

				log_info(log_kernel, "[EJECUTAR]- Linea para ejecutar: %s.", bufferRun);

				resp_com_t respuesta = resolverPedido(bufferRun);
				loggearEjecucion(nivel, pcb->pid, bufferRun);

				log_info(log_kernel, "[EJECUTAR]- RESULTADO FUE: %s", respuesta.msg.str);

				pcb->tipoRespuesta = respuesta.tipo;

				borrar_respuesta(respuesta);

				log_info(log_kernel, "[EJECUTAR]- Por llamar a aplicar Retardo");

				pcb->progamCounter++;

				if (pcb->tipoRespuesta >= RESP_ERROR_PEDIDO_DESCONOCIDO) {

					log_info(log_kernel,"[EJECUTAR]- Entro para exit porque ocurrio el error %d",pcb->tipoRespuesta);
					agregarAExit(pcb);
					break;

				}

			}

		}
		free(bufferRun);

	} //Hasta aca si el comando es RUN

	//Si es otro comando:
	else if (pcb->estado == ejecucion) {
		log_info(log_kernel, "[EJECUTAR]- Por ejecutar comando ingresado por consola: %s.", pcb->linea);
		resp_com_t respuesta = resolverPedido(pcb->linea);
		loggearEjecucion(nivel, pcb->pid, pcb->linea);

		pcb->tipoRespuesta = respuesta.tipo;
		if(respuesta.tipo == RESP_OK){
			if(respuesta.msg.str>0){
				log_info(log_kernel, "[EJECUTAR]- RESULTADO FUE: %s", respuesta.msg.str);
				printf("RESULTADO A <%s> FUE: %s\n", pcb->linea, respuesta.msg.str);
			}else{
				log_info(log_kernel, "[EJECUTAR]- <%s> RESUELTO OK\n",pcb->linea);
				printf("<%s> RESUELTO OK\n",pcb->linea);

			}
		}else{
			log_warning(log_kernel,"[EJECUTAR]- <%s> NO SE PUDO RESOLVER. Error <%d>\n",pcb->linea,respuesta.tipo);
			printf("<%s> NO SE PUDO RESOLVER. Error <%d>\n",pcb->linea,respuesta.tipo);
		}

		borrar_respuesta(respuesta);

		log_info(log_kernel, "[EJECUTAR]- Por llamar a aplicar Retardo");
		aplicarRetardo();

		pcb->progamCounter++;
		if (pcb->tipoRespuesta >= RESP_ERROR_PEDIDO_DESCONOCIDO) {

			log_info(log_kernel, "[EJECUTAR]- Entro para exit porque ocurrio el error %d", pcb->tipoRespuesta);
			agregarAExit(pcb);

		}
	}

	if (pcb->progamCounter == pcb->rafaga && pcb->estado == ejecucion) {

		log_info(log_kernel, "[EJECUTAR]- Entro para exit por fin de rafaga.");
		agregarAExit(pcb);
	}

}

void agregarAExit(t_pcb* pcb) {

	log_info(log_kernel, "[COLA | agregarAExit]- Size colaExit antes: %d.",	list_size(colaExit));

	int resultado = sacarDeColaEjecucion(pcb);

	if (pcb->comando == RUN) {

		log_info(log_kernel, "[COLA | agregarAExit]- La direccion del archivo a cerrar: %p.", pcb->archivo);

		fclose(pcb->archivo);

		log_info(log_kernel, "[COLA | agregarAExit]- Archivo cerrado.");

	}

	log_info(log_kernel, "[COLA | agregarAExit]- Resultado de sacar comando de ejecucion: %d",	resultado);

	if (resultado >= 0) {

		mutexBloquear(&mutexColaExit);
		pcb->estado = exiT;
		list_add(colaExit, pcb);

		mutexDesbloquear(&mutexColaExit);

		log_info(log_kernel, "[COLA | agregarAExit]- Size colaExit despues: %d.",list_size(colaExit));

		if(pcb->tipoRespuesta < RESP_ERROR_PEDIDO_DESCONOCIDO ){

			printf("El comando: %s termino de ejecutarse", pcb->linea);
			printf("\n>");
			puts("");
			log_info(log_kernel, "[COLA | agregarAExit]- La linea: %s termino de ejecutarse.", pcb->linea);

		}
		else{

			printf("El comando: %s ", pcb->linea);
			puts("finalizo antes por error en la linea ");
			printf("%d",pcb->progamCounter);
			//printf("\n>");
			puts("");
			log_info(log_kernel, "[COLA | agregarAExit]- El comando: %s termino con error, en la linea %d. Error de TIPO: %d", pcb->linea,pcb->progamCounter,pcb->tipoRespuesta);

		}
//		free(pcb->linea);
	}
}

int sacarDeColaEjecucion(t_pcb* pcb) {

	mutexBloquear(&mutexColaEjecucion);
	int posicion = buscarPcbEnColaEjecucion(pcb);

	if (posicion >= 0) {
		log_info(log_kernel, "[COLA | sacarDeColaEjecucion]- Por sacar de ejecucion");
		list_remove(colaEjecucion, posicion);
	}

	mutexDesbloquear(&mutexColaEjecucion);
	log_info(log_kernel, "[COLA | sacarDeColaEjecucion]- colaEjecucion (s): %d",	list_size(colaEjecucion));

	return posicion;
}

int buscarPcbEnColaEjecucion(t_pcb* pcb) {

	//@martin @lorenzo no va un mutex aca?
	int pos = -1;

	int tamanio = list_size(colaEjecucion);
	for (int i = 0; i <= tamanio; i++) {
		t_pcb* aux = list_get(colaEjecucion, i);
		if (aux->pid == pcb->pid) {
			pos = i;
			break;
		}
	}

	/*for(int i = 0; i <= tamanio ;i++){

	 if(list_get(colaEjecucion,i) == pcb){

	 pos = i;
	 }
	 }*/

	return pos;
}

void aplicarRetardo(void) {

	log_info(log_kernel,"[KERNEL | aplicarRetardo] Por ejecutar instruccion sleep de: %d.",	arc_config->sleep_ejecucion);

	usleep(arc_config->sleep_ejecucion * 1000); //usleep recibe microsegundos

	log_info(log_kernel,"[KERNEL | aplicarRetardo] Luego de instruccion sleep.");
}

void gossiping_Kernel(pthread_t *hiloGossiping) {

	inicializar_estructuras_gossiping(log_kernel,arc_config->retardo_gossiping);

	char auxPuerto[LARGO_PUERTO];

	sprintf(auxPuerto, "%d", arc_config->puerto_memoria);

	agregar_seed(-1, arc_config->ip_memoria, auxPuerto);

	iniciar_hilo_gossiping(&soy, hiloGossiping, actualizarMemoriasDisponibles);

}

void actualizarMemoriasDisponibles() {

	//Logear diferencias de memorias TODO (Listoo)
	//log_info(log_kernel, "[actualizarMemoriasDisponibles]- Cantidad Memorias ANTES: %d", memoriasConocidasKernel.cant);

	pthread_mutex_lock(&memorias_conocidas_mutex);
	if (memoriasConocidasKernel.cant != 0) {
		free(memoriasConocidasKernel.seeds);
	}
	memoriasConocidasKernel = armar_vector_seeds(soy);
	log_info(log_kernel, "[actualizarMemoriasDisponibles]- Cantidad Memorias DESPUES: %d", memoriasConocidasKernel.cant);
	pthread_mutex_unlock(&memorias_conocidas_mutex);
}

/*seed_com_t* buscarMemoria(char** pruebaPath) {

 seed_com_t *aux = malloc(sizeof(seed_com_t));

 int aux_num = atoi(pruebaPath[2]);

 lista_memorias = list_create();

 lista_memorias = lista_seeds();

 log_info(log_kernel, "El numero de la memoria a buscar es: %s",
 pruebaPath[2]);

 for (int i = 0; i < list_size(lista_memorias); i++) {

 aux = list_get(lista_memorias, i);

 if (aux->numMemoria == aux_num) {

 log_info(log_kernel,
 "Se encontró la memoria: %s en a lista de seeds. Por devolver",
 pruebaPath[2]);
 //retval = aux_num;
 return aux;
 //break;
 }
 }

 return NULL;
 //pthread_mutex_unlock(&gossip_table_mutex);

 //	/if (retval >= 0) {
 //	 log_info(log_kernel, "La memoria se encontró,devolviendo");
 //	 return retval;
 //	 } else {
 //	 log_info(log_kernel, "La memoria no se encontró");
 //	 return retval;
 //	 }/
 }*/

seed_com_t* buscarMemoria(int numMemoria) {
	seed_com_t *memoria_buscada = NULL;
	int iMem = -1;
	log_info(log_kernel, "[buscarMemoria]- Voy a buscar memoria %d",numMemoria);
	pthread_mutex_lock(&memorias_conocidas_mutex);
	for (int i = 0; i < memoriasConocidasKernel.cant; i++) {
		if (memoriasConocidasKernel.seeds[i].numMemoria == numMemoria) {
			iMem = i;
			break;
		}
	}
	if (iMem != -1) {
		memoria_buscada = malloc(sizeof(seed_com_t));

		strcpy(memoria_buscada->ip, memoriasConocidasKernel.seeds[iMem].ip);

		strcpy(memoria_buscada->puerto,memoriasConocidasKernel.seeds[iMem].puerto);

		memoria_buscada->numMemoria =memoriasConocidasKernel.seeds[iMem].numMemoria;
		log_info(log_kernel,"[buscarMemoria]- Se encontro memoria %d: Ip %s. Puerto %s",numMemoria, memoria_buscada->ip, memoria_buscada->puerto);
	} else {
		log_info(log_kernel, "[buscarMemoria]- No se encontro memoria %d",iMem);
	}
	pthread_mutex_unlock(&memorias_conocidas_mutex);

	return memoria_buscada;
}

void comandoAdd(char** comandoSeparado) {

	log_info(log_kernel, "[comandoAdd]- Llego el comando ADD.");

	//seed_com_t* resultado = buscarMemoria(comandoSeparado);

	int criterioInt = buscarCriterio(comandoSeparado[4]);
	if (criterioInt < 0 || criterioInt > EC) {

		log_error(log_kernel, "[comandoAdd]- El criterio %s no existe", comandoSeparado[4]);

		printf("El criterio %s no existe\n", comandoSeparado[4]);
		return;
	}
	seed_com_t *resultado = buscarMemoria(atoi(comandoSeparado[2]));

	if (resultado != NULL) {

		log_info(log_kernel, "[comandoAdd]- La memoria numero %s ha sido encontrada.\n",comandoSeparado[2]);

		log_info(log_kernel,"[comandoAdd]- El criterio para asociar es el: %s,corresponde al valor: %d",comandoSeparado[4], criterioInt);

		int resultado_agregar = agregarMemoriaCriterio(resultado, criterioInt);

		switch(resultado_agregar){
			case 1:
				printf("La memoria %s fue asociada al criterio %s con exito.\n",comandoSeparado[2], comandoSeparado[4]);
				agregarMemoriaAsociada(resultado);
				break;
			case 0:
				printf("La memoria %s ya estaba asociada al criterio %s .\n",comandoSeparado[2], comandoSeparado[4]);
				break;
			case -1:
				printf("La memoria %s no pudo ser asociada al criterio %s.\n",	comandoSeparado[2], comandoSeparado[4]);
				break;
			case -2:
				printf("No se agrega la memoria %s al criterio %s porque ya hay una memoria en el.\n",comandoSeparado[2],comandoSeparado[4]);
				break;
		}
		free(resultado);

		/*
		 criterio_memoria.criterio = criterioInt;
		 criterio_memoria.listMemoriaas = resultado;
		 list_add(lista_memorias, resultado);*/

		//log_info(log_kernel, "Llenamos la estructura de criterio/memoria.");
//		printf("La memoria: %s fue asociada al criterio %s con exito.\n",
//				comandoSeparado[2], comandoSeparado[4]);
	} else {

		printf("No se encontro la memoria.\n");
		log_info(log_kernel, "[comandoAdd]- La memoria %s memoria no ha sido encontrada", comandoSeparado[2]);
	}

}

int buscarCriterio(char* criterio) {

	if (criterio == NULL) {
		log_info(log_kernel, "[buscarCriterio]- Recibimos el criterio: NULL");
		return -1;
	}

	log_info(log_kernel, "[buscarCriterio]- Recibimos el criterio: %s", criterio);

	int i = 0;

	for (i; i <= EC && strcmp(criterios[i], criterio); i++) {
	}

	log_info(log_kernel, "[buscarCriterio]- Se devuelve el valor %d", i);

	return i;
}
/*
 void comandoJournal(char** comandoSeparado) {

 seed_com_t *aux = malloc(sizeof(seed_com_t));
 req_com_t req;
 req.tam = strlen(comandoSeparado[0]);
 req.str = malloc(req.tam);

 log_info(log_kernel, "Tamanio cadena grabada en req:%d", req.tam);

 strcpy(req.str, comandoSeparado[0]);

 log_info(log_kernel, "Cadena grabada en req:%s", req.str);

 for (int i = 0; i < list_size(lista_memorias); i++) {

 aux = list_get(lista_memorias, i);

 socket_CMemoria = conectar_a_memoria(aux->ip, aux->puerto);

 int respuesta = enviar_request(socket_CMemoria, req);

 if (respuesta != 0) {
 log_info(log_kernel,
 "Hubo un error al enviar la request a memoria");
 return;
 }

 log_info(log_kernel, "No Hubo error al enviar la request a memoria");

 msg_com_t msg = recibir_mensaje(socket_CMemoria);

 if (msg.tipo == RESPUESTA) {

 log_info(log_kernel, "Llego un mensaje de tipo RESPUESTA");

 resp_com_t respuesta = procesar_respuesta(msg);
 if (respuesta.tipo == RESP_OK) {
 printf("La respuesta fue correcta %d: ", respuesta.tipo);
 if (respuesta.msg.tam > 0)
 printf("Respuesta recibida %s: \n", respuesta.msg.str);
 log_info(log_kernel,
 "La respuesta fue correcta luego de procesarla");
 } else {
 log_info(log_kernel,
 "La respuesta no fue correcta luego de procesarla");
 }

 borrar_respuesta(respuesta);

 }

 free(req.str);
 }
 }*/

void comandoMetrics() {

	imprimirMetricas(true);
}

int conectar_a_memoria(char ip[LARGO_IP], char puerto[LARGO_PUERTO]) {

	char puerto_memoria[20];

	snprintf(puerto_memoria, 19, "%d", arc_config->puerto_memoria);

	imprimirMensaje2(log_kernel,"[conectar_a_memoria]- Me voy a intentar conectar a ip: <%s> puerto: <%s>",arc_config->ip_memoria, puerto_memoria);

	int socket = conectar_a_servidor(ip, puerto, soy);
	if (socket == -1) {
		imprimirError(log_kernel,"[conectar_a_memoria]- No fue posible conectarse con la Memoria. TERMINANDO\n");
		return -1;
	}

	imprimirMensaje(log_kernel,"[conectar_a_memoria]- Me conecté con éxito a la MEMORIA. Espero su hs");
	//Si me conecté, espero su msg de bienvenida

	msg_com_t msg = recibir_mensaje(socket);

	if (msg.tipo != HANDSHAKECOMANDO) {
		borrar_mensaje(msg);
		imprimirError(log_kernel,"[conectar_a_memoria]- MEMORIA no responde el hs. TERMINANDO\n");
		return -1;
	}

	handshake_com_t hs = procesar_handshake(msg);
	borrar_mensaje(msg);

	if (hs.id == RECHAZADO) {
		if (hs.msg.tam == 0)
			imprimirError(log_kernel,"[conectar_a_memoria]- MEMORIA rechazo la conexión. TERMINANDO\n");
		else
			imprimirError1(log_kernel,"[conectar_a_memoria]- MEMORIA rechazo la conexión [%s]. TERMINANDO\n",hs.msg.str);
		borrar_handshake(hs);
		close(socket);
		return -1;
	}

	borrar_handshake(hs);
	imprimirMensaje(log_kernel,"[conectar_a_memoria]- Me conecté con éxito a la MEMORIA");

	return socket;
}

void inotifyAutomatico(char* pathDelArchivoAEscuchar) {
	int length, i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];
	while (1) {

		fd = inotify_init();

		if (fd < 0) {
			perror("inotify_init");
		}

		wd = inotify_add_watch(fd, pathDelArchivoAEscuchar,IN_MODIFY | IN_CREATE | IN_DELETE);
		length = read(fd, buffer, BUF_LEN);

		if (length < 0) {
			perror("read");
		}

		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len) {
				if (event->mask && IN_CREATE) {
					printf("The file %s was created.\n", event->name);
				} else if (event->mask && IN_DELETE) {
					printf("The file %s was deleted.\n", event->name);
				} else if (event->mask && IN_MODIFY) {
					printf("The file %s was modified.\n", event->name);
				}
			}
			i += EVENT_SIZE + event->len;
		}
		printf("\nSe han realizado cambios en %s\n", pathDelArchivoAEscuchar);
		recargarConfiguracion(PATH_KERNEL_CONFIG);
	}
	(void) inotify_rm_watch(fd, wd);
	(void) close(fd);

	return;
}

void recargarConfiguracion(char* path_config) {

	log_info(log_kernel, "[recargarConfiguracion]- Voy a actualizar");

	mutexBloquear(&mutex_retardos_kernel);

	t_config* auxConfigFile = config_create(path_config);

	if (auxConfigFile != NULL) {

		log_info(log_kernel,"[recargarConfiguracion]- LEYENDO CONFIGURACION...");

		if (config_has_property(auxConfigFile, "SLEEP_EJECUCION")) {

			arc_config->sleep_ejecucion = config_get_int_value(auxConfigFile,"SLEEP_EJECUCION");
			log_info(log_kernel,"[recargarConfiguracion]- SLEEP_EJECUCION: %d",	arc_config->sleep_ejecucion);

		} else {
			log_error(log_kernel,"[recargarConfiguracion]- NO HAY SLEEP_EJECUCION CONFIGURADO");
		} // SLEEP_EJECUCION

		if (config_has_property(auxConfigFile, "METADATA_REFRESH")) {

			arc_config->metadata_refresh = config_get_int_value(auxConfigFile,"METADATA_REFRESH");
			log_info(log_kernel,"[recargarConfiguracion] METADATA_REFRESH es de: %d",arc_config->metadata_refresh);

		} else {
			log_error(log_kernel,"[recargarConfiguracion]- NO HAY METADATA_REFRESH CONFIGURADO");
		} // METADATA_REFRESH

		if (config_has_property(auxConfigFile, "QUANTUM")) {

			arc_config->quantum = config_get_int_value(auxConfigFile,"QUANTUM");
			log_info(log_kernel,"[recargarConfiguracion]- Valor DEL QUANTUM: %d",arc_config->quantum);

		} else {
			log_error(log_kernel,"[recargarConfiguracion]- NO HAY QUANTUM CONFIGURADO");
		} // QUANTUM

	} else {
		log_error(log_kernel,"[recargarConfiguracion]- NO HAY ARCHIVO DE CONFIGURACION DE MODULO DEL KERNEL"); // ERROR: SIN ARCHIVO CONFIGURACION
	}

	config_destroy(auxConfigFile);

	log_info(log_kernel,"[recargarConfiguracion]- RETARDOS y QUANTUM ACTUALIZADOS CORRECTAMENTE");

	mutexDesbloquear(&mutex_retardos_kernel);
}

void *hilo_metadata_refresh(void *args) {
	log_info(log_kernel, "[hilo_metadata_refresh]- Entrando a hilo de actualizacion");

	while (1) {

		log_info(log_kernel,"[hilo_metadata_refresh]- Voy a actualizar la metadata de las tablas");

		actualizarMetadataTablas();

		usleep(arc_config->metadata_refresh * 1000);
	}

}

int actualizarMetadataTablas(void) {
	req_com_t request;
	request.tam = strlen("DESCRIBE") + 1;
	request.str = malloc(request.tam);
	strcpy(request.str, "DESCRIBE");

	seed_com_t *memoria = elegirMemoria();
	if (memoria == NULL) {
		log_error(log_kernel,"[actualizarMetadataTablas]- No tengo memorias para mandar el describe");
		borrar_request_com(request);
		return -1;
	}

	int socket_memoria = conectar_a_memoria(memoria->ip, memoria->puerto);
	if (socket_memoria == -1) {
		log_error(log_kernel,"[actualizarMetadataTablas]- Error al conectarse a la memoria para hacer el describe");
		eliminarMemoriaAsociada(memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera
		free(memoria);
		borrar_request_com(request);
		return -1;
	}

	log_info(log_kernel,"[actualizarMetadataTablas]- Voy a enviar un describe global a la memoria %d",memoria->numMemoria);

	if (enviar_request(socket_memoria, request) == -1) {

		log_error(log_kernel,"[actualizarMetadataTablas]- NO SE PUDO ENVIAR EL DESCRIBE A LA MEMORIA %d",memoria->numMemoria);
		borrar_request_com(request);
		free(memoria);
		return -1;
	}

	borrar_request_com(request);

	msg_com_t msg = recibir_mensaje(socket_memoria);
	if (msg.tipo != RESPUESTA) {

		log_error(log_kernel,"[actualizarMetadataTablas]- ERROR AL RECIBIR RESPUESTA DE MEMORIA %d",	memoria->numMemoria);

		borrar_mensaje(msg);
		free(memoria);
		return -1;
	}

	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);

	if (resp.tipo == RESP_OK && resp.msg.tam > 0) {

		log_info(log_kernel,"[actualizarMetadataTablas]- La memoria %d respondió con %s",memoria->numMemoria, resp.msg.str);

		t_list *nuevaListaTablas = procesarDescribe(resp.msg.str);

		actualizarTablasCriterios(nuevaListaTablas);

	} else {

		log_warning(log_kernel,"[actualizarMetadataTablas]- La memoria %d no pudo resolver el describe. Error <%d>",memoria->numMemoria, resp.tipo);

		borrar_respuesta(resp);

		free(memoria);

		return -1;
	}

	free(memoria);

	borrar_respuesta(resp);

	return 1;
}

//tabla|consistencia|particiones|t_compactacion|tabla|consistencia|particiones|t_compactacion|...
t_list *procesarDescribe(char *str) {
	char ** separado = string_split(str, "|");
	if (separado[0] == NULL) {
		free(separado);
		return NULL;
	}
	t_list *lista_tablas = list_create();
	t_tablas *aux = NULL;
	int i;
	for (i = 0; separado[i] != NULL; i++) {
		if (i % 4 == 0) { //Nombre tabla
			aux = malloc(sizeof(t_tablas));
			aux->nombreTabla = malloc(strlen(separado[i]) + 1);
			strcpy(aux->nombreTabla, separado[i]);
		} else if (i % 4 == 1) { //Criterio
			aux->criterio = buscarCriterio(separado[i]);
		} else if (i % 4 == 2) { //Particiones

		} else { //Tiempo de compactacion
				 //La agrego recién acá para asegurarme que haya venido bien el describe. No sería necesario
			list_add(lista_tablas, aux);
			log_info(log_kernel, "[procesarDescribe]- Tabla recibida <%d> %s %d",i / 4, aux->nombreTabla, aux->criterio);
		}
	}
	if (i % 4 != 0 && aux != NULL) {
		//Quiere decir que no hice el list_add y que el último aux no lo guardé
		//Borro lo que haya quedado en aux ya que no lo uso
		free(aux->nombreTabla);
		free(aux);
	}

	//Libero la memoria que aloca el string_split
	for (i = 0; separado[i] != NULL; i++)
		free(separado[i]);
	free(separado);

	return lista_tablas;
}

void actualizarTablasCriterios(t_list *nuevas) {

	log_info(log_kernel, "[actualizarTablasCriterios]- Se va a actualizar la lista de tablas");

	pthread_mutex_lock(&lista_tablas_mutex);

	if (g_lista_tablas != NULL) {

		list_destroy_and_destroy_elements(g_lista_tablas,(void *) borrarEntradaListaTablas);
	}

	g_lista_tablas = nuevas;

	pthread_mutex_unlock(&lista_tablas_mutex);

	log_info(log_kernel, "[actualizarTablasCriterios]- Se actualizó la lista de tablas");
}

//No hago copia de la tabla, por lo que no hay que hacer un free en el describe
void agregarTablaCriterio(t_tablas *tabla) {

	pthread_mutex_lock(&lista_tablas_mutex);

	if (g_lista_tablas == NULL)
		g_lista_tablas = list_create();

	list_add(g_lista_tablas, tabla);

	pthread_mutex_unlock(&lista_tablas_mutex);

	log_info(log_kernel,"[agregarTablaCriterio]- Se agregó la tabla %s a la lista de tablas. Criterio: %s",
			tabla->nombreTabla, criterios[tabla->criterio]);
}

int buscarCriterioTabla(char *nombre_tabla) {
	int criterio = -1;

	//@todo @martin revisar sincro de esta función

	log_info(log_kernel, "[buscarCriterioTabla]- Voy a buscar tabla %s",nombre_tabla);

	pthread_mutex_lock(&lista_tablas_mutex);

	for (int i = 0; i < list_size(g_lista_tablas); i++) {

		t_tablas *aux = list_get(g_lista_tablas, i);

		if (!strcmp(aux->nombreTabla, nombre_tabla)) {

			log_info(log_kernel,"[buscarCriterioTabla]- El criterio de la tabla %s es %s",nombre_tabla, criterios[aux->criterio]);

			criterio = aux->criterio;

			break;
		}
	}
	pthread_mutex_unlock(&lista_tablas_mutex);
	if (criterio == -1) {
		log_error(log_kernel,"[buscarCriterioTabla]- No se conoce la tabla %s",	nombre_tabla);
	}
	return criterio;
}

void borrarEntradaListaTablas(t_tablas *tabla) {
	free(tabla->nombreTabla);
	free(tabla);
}

seed_com_t *elegirMemoria(void) {
	seed_com_t *retval = NULL;

	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
	if (list_size(g_lista_memorias_asociadas) == 0) {
		pthread_mutex_unlock(&lista_memorias_asociadas_mutex);
		return NULL;
	}
	int elegida = numMemoriaRandom(list_size(g_lista_memorias_asociadas));//0; //@todo @martin: ver como se elige la memoria

	seed_com_t *aux = list_get(g_lista_memorias_asociadas, elegida);
	retval = malloc(sizeof(seed_com_t));
	strcpy(retval->ip, aux->ip);
	strcpy(retval->puerto, aux->puerto);
	retval->numMemoria = aux->numMemoria;

	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);

	return retval;
}

seed_com_t *elegirMemoriaCriterio(int num_criterio, uint16_t key) {
	//@todo @lorenzo revisar
	//@todo @martin revisar sincro de esta función
	seed_com_t *retval = NULL;

	int elegida;
	pthread_mutex_lock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
	t_criterios criterio;
	if (num_criterio == SC) {
		criterio = criterioSC;
		elegida = 0;
	} else if (num_criterio == SHC) {
		criterio = criterioSHC;
		elegida = numMemoriaHash(key);
		log_info(log_kernel,"[elegirMemoriaCriterio | SHC]- La key es: %d.",key);
	} else {
		criterio = criterioEC;
		elegida = numMemoriaRandom(list_size(criterio.listMemorias));
	}
	log_info(log_kernel,"[elegirMemoriaCriterio]- El criterio es el: %s.",criterios[criterio.criterio]);
	//Agregue este Mutex_Verificarlo por las dudas!
//	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
	t_list *memorias_criterio = criterio.listMemorias;

	if (list_size(memorias_criterio) == 0){
//		pthread_mutex_unlock(&lista_memorias_asociadas_mutex);
		pthread_mutex_unlock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
		return NULL;
	}

	seed_com_t *aux = list_get(memorias_criterio, elegida);

	retval = malloc(sizeof(seed_com_t));
	strcpy(retval->ip, aux->ip);
	strcpy(retval->puerto, aux->puerto);
	retval->numMemoria = aux->numMemoria;

//	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);

	log_info(log_kernel, "[elegirMemoriaCriterio]- La memoria elegida es la %d",retval->numMemoria);

	pthread_mutex_unlock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
	return retval;
}

int numMemoriaRandom(int cantidadMemorias)
{
	if(cantidadMemorias == 0)
		return -1;
	return (int) random()%cantidadMemorias;
}

int numMemoriaHash(uint16_t key)
{
	//@todo @martin revisar sincro
	if(list_size(criterioSHC.listMemorias)==0){
		return -1;
	}

	log_info(log_kernel,"[numMemoriaHash]- KEY %d, cantidad memorias %d",key,list_size(criterioSHC.listMemorias));

	return (key % list_size(criterioSHC.listMemorias));
}

int vaciarMemoriasSHC(void)
{
	//@todo @martin revisar sincro | (lorenzo) para mi no deberia sincronizar al ser 1 sola
	log_info(log_kernel, "[vaciarMemoriasSHC]- Se va a mandar Journal a todas las memorias asociadas al criterio");
	t_list *copiaSHC = list_duplicate(criterioSHC.listMemorias);
	request_t request = parser("JOURNAL");
	resp_com_t resp = resolverJournal(request,copiaSHC);

	borrar_respuesta(resp);
	borrar_request(request);

	list_destroy(copiaSHC); //@martin @revisar tengo que destruir los elementos?
	log_info(log_kernel, "[vaciarMemoriasSHC]- Se mando Journal a todas las memorias asociadas al criterio");
	return 1;
}

bool estaMemoriaEnCriterio(int numMemoria, t_list *lista)
{
	//@todo @martin revisar sincro de esta función (sincro en agregarMemoriaCriterio)

	bool encontrada = false;

	for(int i=0; i<list_size(lista); i++){
		seed_com_t *aux = list_get(lista, i);
		if(aux->numMemoria == numMemoria){
			encontrada = true;
			break;
		}
	}

	return encontrada;
}

int agregarMemoriaCriterio(seed_com_t *memoria, int num_criterio) {
	//@todo @martin revisar sincro de esta función
	pthread_mutex_lock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
	t_criterios criterio;
	if (num_criterio == SC) {

		if(list_size(criterioSC.listMemorias)>0){

			log_warning(log_kernel, "[agregarMemoriaCriterio]- No se puede asociar mas de una memoria al criterio SC");
			pthread_mutex_unlock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
			return -2;

		}

		criterio = criterioSC;

	} else if (num_criterio == SHC) {

		criterio = criterioSHC;

	} else if (num_criterio == EC) {

		criterio = criterioEC;

	} else {

		log_error(log_kernel, "[agregarMemoriaCriterio]- No existe un criterio con el numero %d",num_criterio);
		pthread_mutex_unlock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
		return -1;
	}

	//Agregue este Mutex (lorenzo) Verificarlo!
//	pthread_mutex_lock(&lista_memorias_criterio_mutex);
	t_list *memorias_criterio = criterio.listMemorias;
	if(estaMemoriaEnCriterio(memoria->numMemoria,memorias_criterio))
	{
		log_info(log_kernel, "[agregarMemoriaCriterio]- La memoria %d ya estaba asociada al criterio %s",memoria->numMemoria,criterios[num_criterio]);
		pthread_mutex_unlock(&lista_memorias_criterio_mutex); //@martin agrego esto aca
		return 0;
	}
	if(num_criterio == SHC){

		log_info(log_kernel, "[agregarMemoriaCriterio]- Se manda journal a todas las memorias asociadas a este criterio");
		vaciarMemoriasSHC();
	}

	seed_com_t *copia = malloc(sizeof(seed_com_t));
	strcpy(copia->ip, memoria->ip);
	strcpy(copia->puerto, memoria->puerto);
	copia->numMemoria = memoria->numMemoria;

	list_add(memorias_criterio, copia);
	//NO HACER UN FREE DE 'copia' EN ESTA FUNCIÓN, SINO ROMPE
	pthread_mutex_unlock(&lista_memorias_criterio_mutex);

	log_info(log_kernel, "[agregarMemoriaCriterio]- Memoria %d agregada al criterio %d",copia->numMemoria, num_criterio);
	return 1;
}

bool estaMemoriaAsociada(int numMemoria) {
	bool encontrada = false;
	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
	for (int i = 0; i < list_size(g_lista_memorias_asociadas); i++) {
		seed_com_t *aux = list_get(g_lista_memorias_asociadas, i);
		if (aux->numMemoria == numMemoria) {
			encontrada = true;
			break;
		}
	}
	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);
	return encontrada;
}

int agregarMemoriaAsociada(seed_com_t *memoria) {
	seed_com_t *copia;

	if (!estaMemoriaAsociada(memoria->numMemoria)) {

		copia = malloc(sizeof(seed_com_t));

		strcpy(copia->ip, memoria->ip);
		strcpy(copia->puerto, memoria->puerto);
		copia->numMemoria = memoria->numMemoria;

		pthread_mutex_lock(&lista_memorias_asociadas_mutex);

		list_add(g_lista_memorias_asociadas, copia);

		pthread_mutex_unlock(&lista_memorias_asociadas_mutex);

		log_info(log_kernel,"[agregarMemoriaAsociada]- La memoria %d fue agregada a la lista de memorias asociadas",memoria->numMemoria);
		agregarMemoriaMetricas(memoria->numMemoria);
	}
	else{

		log_info(log_kernel,"[agregarMemoriaAsociada]- La memoria %d ya estaba asociada a algun criterio antes",memoria->numMemoria);

	}

	return 1;
}

int eliminarMemoriaAsociada(int numMemoria) {

	int cont = 0;
	pthread_mutex_lock(&lista_memorias_asociadas_mutex);

	for (int i = 0; i < list_size(g_lista_memorias_asociadas); i++) {

		seed_com_t *aux = list_get(g_lista_memorias_asociadas, i);

		if (aux->numMemoria == numMemoria) {

			list_remove(g_lista_memorias_asociadas, i);

			free(aux);

			cont++; //Para saber si estaba asociada o no

			break;
			//Cambie la función asociar memoria para que no haya duplicadas
			//i--; //Ahora la lista tiene un elemento menos
		}
	}

	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);

	log_info(log_kernel, "[eliminarMemoriaAsociada]- Memoria %d eliminada de las memorias asociadas",numMemoria);
	//Si estaba en algún criterio, también la saco
	pthread_mutex_lock(&lista_memorias_criterio_mutex);
	if(eliminarMemoriaCriterio(numMemoria, criterioEC.listMemorias) > 0){

		log_info(log_kernel,"[eliminarMemoriaAsociada]- Memoria %d eliminada del criterio EC",numMemoria);

		log_info(log_kernel,"[eliminarMemoriaAsociada]- El criterio EC ahora tiene %d memorias",list_size(criterioEC.listMemorias));
	}
	if(eliminarMemoriaCriterio(numMemoria, criterioSC.listMemorias) > 0){

		log_info(log_kernel,"[eliminarMemoriaAsociada]- Memoria %d eliminada del criterio SC",numMemoria);

		log_info(log_kernel,"[eliminarMemoriaAsociada]- No se podran resolver pedidos SC hasta agregar otra memoria a este criterio",numMemoria);
	}
	if(eliminarMemoriaCriterio(numMemoria, criterioSHC.listMemorias) > 0){
		log_info(log_kernel,"[eliminarMemoriaAsociada]- Memoria %d eliminada del criterio SHC",numMemoria);
		vaciarMemoriasSHC();
		log_info(log_kernel,"[eliminarMemoriaAsociada]- Se mendo journal a todas las memorias que quedan en el criterio",numMemoria);
	}
	pthread_mutex_unlock(&lista_memorias_criterio_mutex);
	//Finalmente la saco de las metricas
	sacarMemoriaMetricas(numMemoria);

	/*@martin: revisar si : @hecho
	 * 1) estaba en el criterio SHC y hay que actualizar la logica (mandar journal, cambiar hash, etc)
	 * 2) era la memoria de SC y hay que informarlo
	 */
	return cont;
}

int eliminarMemoriaCriterio(int numMemoria, t_list *lista_memorias) {
	//@martin: falta sincro
	//@lorenzo--> agregue cosas, revisar

	int cont=0;

	//Agregue este mutex
//	pthread_mutex_lock(&lista_memorias_criterio_mutex); //Lo comento y lo pongo en la función que llama a esta
	for(int i=0;i<list_size(lista_memorias);i++){
		seed_com_t *aux = list_get(lista_memorias,i);

		if(aux->numMemoria == numMemoria){
			list_remove(lista_memorias,i);
			free(aux);
			cont++; //Para saber si estaba asociada o no
			break;
			//Cambié la función de agregar para que la memoria no esté duplicada nunca
//			i--; //Ahora la lista tiene un elemento menos
			//No hago el break porque la memoria puede estar asociada a más de un criterio y aparecer duplicada en esta lista
		}
	}
//	pthread_mutex_unlock(&lista_memorias_criterio_mutex);
//	log_info(log_kernel, "[BAJA MEMORIA] La memoria %d estaba en el criterio %d veces",numMemoria,cont);
	return cont;
}

//Uso este tipo de respuesta para poder especificar tipo de error, en el caso de que lo haya, y respuesta
resp_com_t resolverPedido(char *linea) {

	if (linea[strlen(linea) - 1] == '\n') {
		linea[strlen(linea) - 1] = '\0';
	}
	resp_com_t resp;
	request_t request = parser(linea);

	log_info(log_kernel, "[resolverPedido]- Voy a resolver pedido %s",linea);

	switch(request.command){
		case SELECT_PARSER:
			resp = resolverSelect(request);
			break;
		case INSERT_PARSER:
			resp = resolverInsert(request);
			break;
		case DESCRIBE_PARSER:
			resp = resolverDescribe(request);
			break;
		case CREATE_PARSER:
			resp = resolverCreate(request);
			break;
		case DROP_PARSER:
			resp = resolverDrop(request);
			break;
		case JOURNAL_PARSER:
			resp = resolverJournalGlobal(request);
			break;

		//Pienso la función para resolver los comandos planificables y desde la función ejecutar
		/*case ADD:
		 break;
		 case RUN:
		 break;
		 case METRICS:
		 break;
		 case SALIR:
		 break;*/
	default:
		break;
	}

	borrar_request(request);

	if (resp.tipo == RESP_OK) {

		log_info(log_kernel, "[resolverPedido]- Pedido %s resuelto OK", linea);
		if (resp.msg.str != NULL && resp.msg.tam > 0)
			log_info(log_kernel, "[resolverPedido]- Respuesta a pedido %s es %s", linea,resp.msg.str);

	} else {

		log_error(log_kernel,"[resolverPedido]- El pedido %s no se pudo resolver. Error <%d>", linea,resp.tipo);
	}

	return resp;
}

resp_com_t resolverSelect(request_t request) {
	//  SELECT  NOMBRE_TABLA    KEY
	//<command>   args[0]     args[1]

	if (request.cant_args != 2)
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);

	uint16_t key = atoi(request.args[1]);

	int criterio = buscarCriterioTabla(request.args[0]);

	if (criterio == -1) {

		log_error(log_kernel,"[resolverSelect]- No tenemos metadata de la tabla %s, no podemos seguir",request.args[0]);

		return armar_respuesta(RESP_ERROR_DESCONOZCO_CRITERIO_TABLA, NULL);
	}

	log_info(log_kernel, "[resolverSelect]- La tabla es de criterio %s",criterios[criterio]);

	seed_com_t *datos_memoria = elegirMemoriaCriterio(criterio, key);

	if (datos_memoria == NULL) {

		log_error(log_kernel,"[resolverSelect]- No se encontro una memoria a la que mandar el select");

		return armar_respuesta(RESP_ERROR_SIN_MEMORIAS_CRITERIO, NULL);
	}

	log_info(log_kernel, "[resolverSelect] Memoria elegida: %d, que corresponde al criterio: %s",datos_memoria->numMemoria,criterios[criterio]);

	int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);

	if (socket_memoria == -1) {

		log_error(log_kernel,"[resolverSelect]- No se pudo establecer conexion con la memoria %d",datos_memoria->numMemoria);

		eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera

		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}

	uint64_t t0 = timestamp();

	resp_com_t resp = enviar_recibir(socket_memoria,request.request_str);

	contar_select(datos_memoria->numMemoria,criterio,timestamp()-t0);

	free(datos_memoria);
	close(socket_memoria);
	return resp;
}

resp_com_t resolverInsert(request_t request) {
	//  INSERT  NOMBRE_TABLA   KEY     VALUE  [TIMESTAMP]
	//<command>   args[0]    args[1]  args[2]   args[3]

	if (request.cant_args != 3 && request.cant_args != 4)
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);

	log_info(log_kernel, "[resolverInsert | DEBUG] Cant args = %d",request.cant_args);

	uint16_t key = atoi(request.args[1]);

	int criterio = buscarCriterioTabla(request.args[0]);

	if (criterio == -1) {
		log_error(log_kernel,"[resolverInsert]- No tenemos metadata de la tabla %s, no podemos seguir",request.args[0]);

		return armar_respuesta(RESP_ERROR_DESCONOZCO_CRITERIO_TABLA, NULL);
	}
	log_info(log_kernel, "[resolverInsert]- La tabla es de criterio %s",criterios[criterio]);

	seed_com_t *datos_memoria = elegirMemoriaCriterio(criterio, key);

	if (datos_memoria == NULL) {
		log_error(log_kernel,"[resolverInsert]- No se encontro una memoria a la que mandar el insert");

		return armar_respuesta(RESP_ERROR_SIN_MEMORIAS_CRITERIO, NULL);
	}
	log_info(log_kernel, "[resolverInsert]- Memoria elegida: %d, que corresponde al criterio: %s",datos_memoria->numMemoria,criterios[criterio]);

	int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);
	if (socket_memoria == -1) {

		log_error(log_kernel,"[resolverInsert]- No se pudo establecer conexion con la memoria %d",datos_memoria->numMemoria);

		eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera

		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}

	uint64_t t0 = timestamp();

	resp_com_t resp = enviar_recibir(socket_memoria,request.request_str);

	contar_insert(datos_memoria->numMemoria,criterio,timestamp()-t0);

	free(datos_memoria);
	close(socket_memoria);
	return resp;
}

resp_com_t resolverCreate(request_t request) {
	// CREATE  NOMBRE_TABLA CONSISTENCIA  PARTICIONES  T_COMPACTACION
	//<command>   args[0]      args[1]      args[2]       args[3]
	if (request.cant_args != 4)
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);

	seed_com_t *datos_memoria = elegirMemoria();

	if (datos_memoria == NULL) {
		log_error(log_kernel,"[resolverCreate]- No se encontro una memoria a la que mandar el create");

		return armar_respuesta(RESP_ERROR_SIN_MEMORIAS_ASOCIADAS, NULL);
	}
	log_info(log_kernel, "[resolverCreate]- Memoria elegida: %d",datos_memoria->numMemoria);

	int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);
	if (socket_memoria == -1) {

		log_error(log_kernel,"[resolverCreate]- No se pudo establecer conexion con la memoria %d",datos_memoria->numMemoria);

		eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera

		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}
	resp_com_t resp = enviar_recibir(socket_memoria, request.request_str);

	if (resp.tipo == RESP_OK) {
		log_info(log_kernel,"[resolverCreate]- El create de la tabla %s se realizo correctamente. Se agrega la metadata de la tabla",request.args[0]);

		//La tabla se pudo crear y tengo que agregar su metadata a la estructura de tablas conocidas
		t_tablas *nueva_tabla = malloc(sizeof(t_tablas)); //NO HACER FREE ACA, LO HAGO EN OTRA FUNCION
		nueva_tabla->criterio = buscarCriterio(request.args[1]);
		nueva_tabla->nombreTabla = malloc(strlen(request.args[0] + 1));

		strcpy(nueva_tabla->nombreTabla, request.args[0]);
		agregarTablaCriterio(nueva_tabla);

	} else {
		log_warning(log_kernel,"[resolverCreate]- El create de la tabla %s no se pudo resolver. Error de tipo <%d>",request.args[0], resp.tipo);
	}
	free(datos_memoria);
	close(socket_memoria);
	return resp;
}

resp_com_t resolverDescribe(request_t request) {
	// DESCRIBE  [NOMBRE_TABLA]*
	//<command>   	args[0]

	seed_com_t *datos_memoria;
	bool describeGlobal;

	if (request.cant_args > 1) {
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	} else if (request.cant_args == 0) {
		describeGlobal = true;
		datos_memoria = elegirMemoria();
	} else {
		describeGlobal = false;
		datos_memoria = elegirMemoria(); //@martin deberia mandar el describe a una memoria del criterio de la tabla?
	}

	if (datos_memoria == NULL) {
		log_error(log_kernel,"[resolverDescribe]- No se encontro una memoria a la que mandar el pedido");

		return armar_respuesta(RESP_ERROR_SIN_MEMORIAS_ASOCIADAS, NULL);
	}

	log_info(log_kernel, "[resolverDescribe]- Memoria elegida: %d",datos_memoria->numMemoria);

	int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);
	if (socket_memoria == -1) {
		log_error(log_kernel,"[resolverDescribe]- No se pudo establecer conexion con la memoria %d",datos_memoria->numMemoria);

		eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera

		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}
	resp_com_t resp = enviar_recibir(socket_memoria, request.request_str);

	if (resp.tipo == RESP_OK && resp.msg.tam > 0) {

		log_info(log_kernel, "[resolverDescribe]- La memoria %d respondió con %s",datos_memoria->numMemoria, resp.msg.str);

		t_list *nuevaListaTablas = procesarDescribe(resp.msg.str);

		if (describeGlobal)
			actualizarTablasCriterios(nuevaListaTablas);
		else {
			t_tablas *aux = list_get(nuevaListaTablas, 0);
			t_tablas *tabla = malloc(sizeof(t_tablas)); //NO HACER FREE

			tabla->criterio = aux->criterio;
			tabla->nombreTabla = malloc(strlen(aux->nombreTabla) + 1);
			strcpy(tabla->nombreTabla, aux->nombreTabla);

			agregarTablaCriterio(tabla);

			list_destroy_and_destroy_elements(nuevaListaTablas,(void *) borrarEntradaListaTablas);
		}
	} else {

		log_warning(log_kernel,"[resolverDescribe]- La memoria %d no pudo resolver el describe. Error <%d>",datos_memoria->numMemoria, resp.tipo);
	}

	free(datos_memoria);
	close(socket_memoria);

	return resp;
}

resp_com_t resolverDrop(request_t request) {
	//  DROP      NOMBRE_TABLA
	//<command>   	args[0]
	seed_com_t *datos_memoria;
	if (request.cant_args != 1) {
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}
	datos_memoria = elegirMemoria();
	if (datos_memoria == NULL) {
		log_error(log_kernel,"[resolverDrop]- No se encontro una memoria a la que mandar el pedido");
		return armar_respuesta(RESP_ERROR_SIN_MEMORIAS_ASOCIADAS, NULL);
	}

	log_info(log_kernel, "[resolverDrop]- Memoria elegida: %d",	datos_memoria->numMemoria);

	int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);
	if (socket_memoria == -1) {
		log_error(log_kernel,"[resolverDrop]- No se pudo establecer conexion con la memoria %d",datos_memoria->numMemoria);

		eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera

		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}

	resp_com_t resp = enviar_recibir(socket_memoria, request.request_str);

	//@martin: hace falta borrar la tabla de los criterios?
	free(datos_memoria);
	close(socket_memoria);

	return resp;
}

resp_com_t resolverJournalGlobal(request_t request)
{
	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
	t_list *copia = list_duplicate(g_lista_memorias_asociadas);
	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);
	resp_com_t resp = resolverJournal(request, copia);
	list_destroy(copia); //@martin @revisar tengo que destruir los elementos?
	return resp;
}

resp_com_t resolverJournal(request_t request, t_list *lista_memorias)
{
	//@martin: siempre se manda a todas las memorias asociadas?
	// JOURNAL
	//<command>
	if (request.cant_args != 0) {
		return armar_respuesta(RESP_ERROR_CANT_PARAMETROS, NULL);
	}
	seed_com_t *datos_memoria;
	int cont = 0;

//	pthread_mutex_lock(&lista_memorias_asociadas_mutex);
//
//	t_list *copia_memorias = list_duplicate(g_lista_memorias_asociadas);
//
//	pthread_mutex_unlock(&lista_memorias_asociadas_mutex);


	for(int i = 0; i<list_size(lista_memorias);i++){
		datos_memoria = list_get(lista_memorias,i);
		log_info(log_kernel,"[resolverJournal]- Enviando journal a memoria %d",datos_memoria->numMemoria);
		int socket_memoria = conectar_a_memoria(datos_memoria->ip,datos_memoria->puerto);
		if(socket_memoria == -1){

			log_error(log_kernel,"[resolverJournal]- No se pudo establecer conexion con la memoria %d", datos_memoria->numMemoria);

			eliminarMemoriaAsociada(datos_memoria->numMemoria); //esta función también la saca de todos los criterios en los que estuviera
			break;
		}
		resp_com_t resp = enviar_recibir(socket_memoria, request.request_str);

		if (resp.tipo == RESP_OK) {
			log_info(log_kernel,"[resolverJournal]- La memoria %d resolvio journal con exito",datos_memoria->numMemoria);
			cont++;
		} else {

			log_error(log_kernel,"[resolverJournal]- La memoria %d no pudo resolver el journal. Error <%d>",datos_memoria->numMemoria, resp.tipo);
		}

		borrar_respuesta(resp);
		close(socket_memoria);
	}

	log_info(log_kernel, "[resolverJournal]- Journal resuelto correctamente por %d memorias del total de %d", cont, list_size(lista_memorias));

	return armar_respuesta(RESP_OK, NULL);
}

resp_com_t enviar_recibir(int socket,char *req_str)
{
	req_com_t a_enviar;
	a_enviar.tam = strlen(req_str) + 1;
	a_enviar.str = malloc(a_enviar.tam);
	strcpy(a_enviar.str, req_str);
	log_info(log_kernel, "[enviar_recibir]- Voy a enviar request: %s", req_str);

	if (enviar_request(socket, a_enviar) == -1) {
		log_error(log_kernel, "[enviar_recibir]- Error al enviar request: %s",req_str);
		borrar_request_com(a_enviar);
		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}
	borrar_request_com(a_enviar);

	log_info(log_kernel, "[enviar_recibir] Request enviado: %s", req_str);

	msg_com_t msg = recibir_mensaje(socket);
	if (msg.tipo != RESPUESTA) {

		log_error(log_kernel,"[enviar_recibir] Error al recibir respuesta para el request: %s",req_str);
		borrar_mensaje(msg);
		return armar_respuesta(RESP_ERROR_COMUNICACION, NULL);
	}
	resp_com_t resp = procesar_respuesta(msg);
	borrar_mensaje(msg);

	if (resp.tipo == RESP_OK) {
		log_info(log_kernel,"[enviar_recibir]- El request <%s> se realizó correctamente",req_str);

		if (resp.msg.str != NULL && resp.msg.tam > 0) {

			log_info(log_kernel, "[enviar_recibir]- La respuesta es <%s>",resp.msg.str);

		}

	} else {

		log_error(log_kernel,"[enviar_recibir]- El request <%s> no pudo realizarse. Error <%d>",req_str, resp.tipo);

	}

	return resp;
}

void loggearEjecucion(int nivel, int pid, char* linea) {

	fprintf(fp_trace_ejecucion, "\n<NIVEL %d><PID: %d>: %s", nivel, pid, linea);
}

/* METRICAS */

void inicializarMetricas(void)
{
	g_metricas.operaciones_memorias = list_create();
	g_metricas.ultima_actualiz = timestamp();
	g_metricas.operaciones_totales = 0;
	for(int i=0; i<3; i++){
		g_metricas.criterio[i].cant_insert = 0;
		g_metricas.criterio[i].cant_select = 0;
		g_metricas.criterio[i].tiempo_insert = 0;
		g_metricas.criterio[i].tiempo_select = 0;
	}
}

void *correrHiloMetricas(void *args)
{
	inicializarMetricas();
	while(1){
		usleep(30000000);
		imprimirMetricas(false);
		reiniciarMetricas();
	}
}

void reiniciarMetricas(void)
{
	pthread_mutex_lock(&mutex_metricas);
	g_metricas.ultima_actualiz = timestamp();
	g_metricas.operaciones_totales = 0;
	for(int i=0; i<3; i++){
		g_metricas.criterio[i].cant_insert = 0;
		g_metricas.criterio[i].cant_select = 0;
		g_metricas.criterio[i].tiempo_insert = 0;
		g_metricas.criterio[i].tiempo_select = 0;
	}
	t_metricas_memoria *aux;
	for(int i=0;i<list_size(g_metricas.operaciones_memorias);i++){
		aux = list_get(g_metricas.operaciones_memorias,i);
		aux->operaciones = 0;
	}
	pthread_mutex_unlock(&mutex_metricas);
}

void agregarMemoriaMetricas(int num_memoria)
{
	t_metricas_memoria *nueva = malloc(sizeof(t_metricas_memoria));
	nueva->num_memoria = num_memoria;
	nueva->operaciones = 0;
	log_info(log_kernel, "[agregarMemoriaMetricas]- Inicializando estructuras para las metricas de la memoria %d",num_memoria);

	pthread_mutex_lock(&mutex_metricas);
	list_add(g_metricas.operaciones_memorias,nueva);
	pthread_mutex_unlock(&mutex_metricas);
}

void sacarMemoriaMetricas(int num_memoria)
{
	t_metricas_memoria *aux;
	log_info(log_kernel, "[sacarMemoriaMetricas]- Sacando la memoria %d de las estructuras de las metricas",num_memoria);

	pthread_mutex_lock(&mutex_metricas);

	for(int i=0; i<list_size(g_metricas.operaciones_memorias);i++){
		aux = list_get(g_metricas.operaciones_memorias,i);
		if(aux->num_memoria == num_memoria){
			list_remove(g_metricas.operaciones_memorias,i);
			free(aux);
			break;
		}
	}
	pthread_mutex_unlock(&mutex_metricas);
}

void contar_insert(int num_memoria, int criterio, uint64_t duracion)
{
	t_metricas_memoria *aux;
	log_info(log_kernel, "[contar_insert]- Agregando informacion de insert. Memoria: %d. Criterio: %s. Duracion: %llums",num_memoria,criterios[criterio],duracion);

	pthread_mutex_lock(&mutex_metricas);
	g_metricas.operaciones_totales++;

	for(int i=0; i<list_size(g_metricas.operaciones_memorias);i++){
		aux = list_get(g_metricas.operaciones_memorias,i);
		if(aux->num_memoria == num_memoria){
			aux->operaciones++;
			break;
		}
	}
	g_metricas.criterio[criterio].cant_insert++;
	g_metricas.criterio[criterio].tiempo_insert += duracion;
	pthread_mutex_unlock(&mutex_metricas);

}

void contar_select(int num_memoria, int criterio, uint64_t duracion)
{
	t_metricas_memoria *aux;
	log_info(log_kernel, "[contar_select]- Agregando informacion de select. Memoria: %d. Criterio: %s. Duracion: %llums",num_memoria,criterios[criterio],duracion);
	pthread_mutex_lock(&mutex_metricas);
	g_metricas.operaciones_totales++;

	for(int i=0; i<list_size(g_metricas.operaciones_memorias);i++){
		aux = list_get(g_metricas.operaciones_memorias,i);
		if(aux->num_memoria == num_memoria){
			aux->operaciones++;
			break;
		}
	}
	g_metricas.criterio[criterio].cant_select++;
	g_metricas.criterio[criterio].tiempo_select += duracion;
	pthread_mutex_unlock(&mutex_metricas);
}


void imprimirMetricas(bool enConsola)
{
	t_metricas_memoria *aux;
	pthread_mutex_lock(&mutex_metricas);

	log_info(log_kernel, "[imprimirMetricas]- Ultima actualizacion hace %d segundos",(timestamp()-g_metricas.ultima_actualiz)/1000);
	if(enConsola){
		printf("\n[METRICAS] Ultima actualizacion hace %d segundos",(timestamp()-g_metricas.ultima_actualiz)/1000);
	}
	float read_latency, write_latency, memory_load;

	for(int i=0;i<3;i++){

		if( g_metricas.criterio[i].cant_select > 0 )
			read_latency = (float)g_metricas.criterio[i].tiempo_select / g_metricas.criterio[i].cant_select;
		else
			read_latency = 0;
		if( g_metricas.criterio[i].cant_insert > 0 )
			write_latency = (float)g_metricas.criterio[i].tiempo_insert / g_metricas.criterio[i].cant_insert;
		else
			write_latency = 0;
		imprimirStringFloat(enConsola,"[METRICAS] Criterio %s. Read latency: %.2fms",criterios[i],read_latency);
		imprimirStringFloat(enConsola,"[METRICAS] Criterio %s. Write latency: %.2fms",criterios[i],write_latency);
		imprimirStringInt(enConsola,"[METRICAS] Criterio %s. Reads: %d",criterios[i],g_metricas.criterio[i].cant_select);
		imprimirStringInt(enConsola,"[METRICAS] Criterio %s. Writes: %d",criterios[i],g_metricas.criterio[i].cant_insert);
	}
	for(int i=0; i<list_size(g_metricas.operaciones_memorias);i++){

		aux = list_get(g_metricas.operaciones_memorias,i);
		if(g_metricas.operaciones_totales > 0)
			memory_load = ((float)aux->operaciones/g_metricas.operaciones_totales)*100;
		else
			memory_load = 0;
		imprimirIntFloat(enConsola,"[METRICAS] Memoria %d. Memory load %.1f%%",aux->num_memoria,memory_load);
	}
	if(enConsola)
		printf("\n");
	pthread_mutex_unlock(&mutex_metricas);
}

void imprimirStringFloat(bool consola, const char *format,const char *p1, float p2)
{
	log_info(log_kernel,format,p1,p2);
	if(consola){
		printf("\n");
		printf(format,p1,p2);
	}
}

void imprimirStringInt(bool consola, const char *format, const char *p1, int p2)
{
	log_info(log_kernel,format,p1,p2);
	if(consola){
		printf("\n");
		printf(format,p1,p2);
	}
}

void imprimirIntFloat(bool consola, const char *format, int p1, float p2)
{
	log_info(log_kernel,format,p1,p2);
	if(consola){
		printf("\n");
		printf(format,p1,p2);
	}
}
