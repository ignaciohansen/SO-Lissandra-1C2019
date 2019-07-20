/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"
#include "kernel_aux.h"

int main() {

	log_kernel = archivoLogCrear(LOG_PATH, "Proceso Kernel");

	log_info(log_kernel,
			"Ya creado el Log, continuamos cargando la estructura de configuracion, llamando a la funcion.");

	cargarConfiguracion();

	log_info(log_kernel, "La carga de archivo de configuracion finalizo.");

	log_info(log_kernel,
			"El valor que le vamos a poner al semaforo de multiprocesamiento es: %d.",
			arc_config->multiprocesamiento);
	semaforoIniciar(&multiprocesamiento, arc_config->multiprocesamiento);

	inicializarListasPlanificador();
	lista_memorias = list_create();

	mutexIniciar(&countProcess);
	mutexIniciar(&mutexColaNuevos);

	countPID = 0;
	mutexIniciar(&mutex_retardos_kernel);
	char* path_de_kernel = malloc(strlen(PATH_KERNEL_CONFIG) + 1);
	strcpy(path_de_kernel, PATH_KERNEL_CONFIG);
	pthread_t inotify_c;
	pthread_create(&inotify_c, NULL, (void *) inotifyAutomatico,
			path_de_kernel);
	pthread_detach(inotify_c);
	printf("\n*Hilo de actualización de retardos y Quantum corriendo.\n");
	log_info(log_kernel,
			"Hilo de actualización de retardos y Quantum corriendo");

	gossiping_Kernel();

	log_info(log_kernel, "Creamos hilo para Consola.");
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);

	pthread_join(hiloConsola, NULL);

	log_info(log_kernel, "Salimoooos, fin del main.");

	return 0;

}

void cargarConfiguracion() {

	log_info(log_kernel,
			"Por reservar memoria para variable de configuracion.");

	arc_config = malloc(sizeof(t_kernel_config));

	t_config* configFile;

	log_info(log_kernel,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_KERNEL_CONFIG);

	if (configFile != NULL) {

		log_info(log_kernel, "Kernel: Leyendo Archivo de Configuracion...");

		if (config_has_property(configFile, "PUERTO_MEMORIA")) {

			log_info(log_kernel, "Almacenando el puerto");

			arc_config->puerto_memoria = config_get_int_value(configFile,
					"PUERTO_MEMORIA");

			log_info(log_kernel, "El puerto de la memoria es: %d",
					arc_config->puerto_memoria);

		} else {
			log_error(log_kernel,
					"El archivo de configuracion no contiene el PUERTO de la Memoria");

		}

		if (config_has_property(configFile, "IP_MEMORIA")) {

			log_info(log_kernel, "Almacenando la IP de la Memoria");

			arc_config->ip_memoria = config_get_string_value(configFile,
					"IP_MEMORIA");

			log_info(log_kernel, "La Ip de la memoria es: %s",
					arc_config->ip_memoria);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no contiene la IP de la Memoria");

		}

		if (config_has_property(configFile, "QUANTUM")) {

			log_info(log_kernel, "Almancenando el Quantum del planificador");

			arc_config->quantum = config_get_int_value(configFile, "QUANTUM");

			log_info(log_kernel, "El Quantum del planificador es: %d",
					arc_config->quantum);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no contiene el Quantum del planificador");

		}

		if (config_has_property(configFile, "MULTIPROCESAMIENTO")) {

			log_info(log_kernel,
					"Almacenando el valor del Multiprocesamiento para el Planificador");

			arc_config->multiprocesamiento = config_get_int_value(configFile,
					"MULTIPROCESAMIENTO");

			log_info(log_kernel,
					"El grado de multiprocesamiento del planificador es: %d",
					arc_config->multiprocesamiento);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no el grado de multiprocesamiento del planificador");

		}

		if (config_has_property(configFile, "METADATA_REFRESH")) {

			log_info(log_kernel,
					"Almacenando el valor del Metadata Refresh para el Kernel");

			arc_config->metadata_refresh = config_get_int_value(configFile,
					"METADATA_REFRESH");

			log_info(log_kernel, "El valor del Metadata Refresh es: %d",
					arc_config->metadata_refresh);
		} else {

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Metadata refresh");

		}

		if (config_has_property(configFile, "SLEEP_EJECUCION")) {

			log_info(log_kernel,
					"Almacenando el valor del Sleep Ejecucion para el Kernel");

			arc_config->sleep_ejecucion = config_get_int_value(configFile,
					"SLEEP_EJECUCION");

			log_info(log_kernel, "El valor del Sleep Ejecucion es: %d",
					arc_config->sleep_ejecucion);
		} else {

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Sleep Ejecucion");

		}

	} else {

		log_error(log_kernel,
				"No se encontro el archivo de configuracion para cargar la estructura de Kernel");

	}

	log_info(log_kernel,
			"Cargamos todo lo que se encontro en el archivo de configuracion. Liberamos la variable config que fue utlizada para navegar el archivo de configuracion");

	free(configFile->path);
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
	free(configFile);

}

void consola() {

	log_info(log_kernel, "En el hilo de consola.");

	menu();

	while (1) {

		linea = readline(">");

		if (linea) {
			add_history(linea);
			comandoSeparado = string_split(linea, separator);
		}

		if (!strncmp(linea, "SALIR", 4)) {
			log_info(log_kernel, "Viene el comando en la cadena: %s",
					comandoSeparado[0]);
			free(linea);
			break;
		}

		strtok(linea, "\n");

		log_info(log_kernel, "Viene el comando en la cadena: %s",
				comandoSeparado[0]);

		int comando = buscarComando(comandoSeparado[0]);

		log_info(log_kernel, "El enum correspondiente para el comando es: %d",
				comando);

		switch (comando) {
		case ADD:
			printf("Vino ADD.\n");
			log_info(log_kernel, "ADD.");
			comandoAdd(comandoSeparado);
			break;
		case JOURNAL:
			printf("Vino journal.\n");
			log_info(log_kernel, "Journal.");
			comandoJournal(comandoSeparado);
			break;
		case METRICS:
			printf("Vino meterics.\n");
			log_info(log_kernel, "Metrics.");
			comandoMetrics();
			break;
		case SALIR:
			printf("Salimos de la consola y el proceso!.\n");
			log_info(log_kernel, "Vino comando salir. Cerramos todo");
			free(linea);
			return;
		default:
			log_info(log_kernel, "Entramos por default, a Planificar");
			strtok(linea, "\n");
			planificar(linea);
			break;
		}

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
		log_info(log_kernel, "Recibimos el comando: NULL");
		return -1;
	}

	log_info(log_kernel, "Recibimos el comando: %s", comando);

	int i = 0;

	for (i; i <= SALIR && strcmp(comandosPermitidos[i], comando); i++) {
	}

	log_info(log_kernel, "Se devuelve el valor %d", i);

	return i;

}

void validarLinea(char** lineaIngresada, t_log* logger) {

	for (int i = 0; lineaIngresada[i] != NULL; i++) {

		log_info(log_kernel, "En la posicion %d del array esta el valor %s", i,
				lineaIngresada[i]);

		tamanio = i + 1;
	}

	log_info(log_kernel, "El tamanio del vector de comandos es: %d", tamanio);

}

void inicializarListasPlanificador() {

	colaNuevos = list_create();
	colaListos = list_create();
	colaExit = list_create();
	colaEjecucion = list_create();
}

t_pcb* crearPcb(char* linea) {

	log_info(log_kernel, "Creando PCB ==> PID: %d", countPID);

	t_pcb* pcbProceso = malloc(sizeof(t_pcb));

	pcbProceso->linea = linea;

	comandoSeparado = string_split(linea, separator);

	for (int i = 0; comandoSeparado[i] != NULL; i++) {

		log_info(log_kernel, "En la posicion %d del array esta el valor %s", i,
				comandoSeparado[i]);

		tamanio = i + 1;
	}

	int auxComandoInt;

	if (strcmp(comandoSeparado[0], "RUN") == 0) {

		auxComandoInt = RUN;

	} else if(strcmp(comandoSeparado[0], "ADD") == 0){

		auxComandoInt = ADD;

	} else if(strcmp(comandoSeparado[0], "SELECT") == 0){

		auxComandoInt = SELECT;

	}else if(strcmp(comandoSeparado[0], "INSERT") == 0){

		auxComandoInt = INSERT;

	}else if(strcmp(comandoSeparado[0], "CREATE") == 0){

		auxComandoInt = CREATE;
	}else if(strcmp(comandoSeparado[0], "DESCRIBE") == 0){

		auxComandoInt = DESCRIBE;
	}

	log_info(log_kernel, "El valor del comando para el ENUM es: %d",
			auxComandoInt);

	switch (auxComandoInt) {

	case RUN: {

		log_info(log_kernel,
				"Vino Run de comando, vamos a buscar cuantas lineas tiene el archivo");
		int aux_rafaga = rafagaComandoRun(comandoSeparado[1]);

		log_info(log_kernel, "La rafaga del run es: %d", aux_rafaga);
		pcbProceso->pid = countPID;
		pcbProceso->comando = auxComandoInt;
		pcbProceso->rafaga = aux_rafaga;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estado en la cola new porque recien se crea
		pcbProceso->progamCounter = 0;

	}
		break;
	default: {

		log_info(log_kernel, "En la condicion de que no es un comando RUN");
		pcbProceso->pid = countPID;
		pcbProceso->comando = auxComandoInt;
		pcbProceso->rafaga = 1;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estado en la cola new porque recien se crea
		pcbProceso->progamCounter = 0;

	}
		break;

	}

	return pcbProceso;
}

int rafagaComandoRun(char* path) {

	log_info(log_kernel,
			"Vamos a buscar la cantidad de lineas que tiene el archivo");
	int caracter, contador;

	contador = 0;

	FILE* fd;

	fd = fopen(path, "r");

	if (fd == NULL) {

		log_info(log_kernel, "El archivo pasado por path no se encontró");
		printf("El archivo %s No existe\n", path);

		free(path);
		return -1;
	} else {

		log_info(log_kernel,
				"El archivo se encontró con exito. Vamos a leerlo para ver la cantidad de lineas");
		printf("El archivo buscado en la dirección %s existe. Vamos a leerlo\n",
				path);

		while ((caracter = fgetc(fd)) != EOF) {

			if (caracter == '\n') {

				contador++;
			}

		}

		log_info(log_kernel,
				"Fuera del while principal, la cantidad de lineas del archivo es: %d",
				contador);
		rewind(fd);
		fclose(fd);
		free(path);

		log_info(log_kernel, "Por retornar contador");
		return contador;
	}

	return 0;
}

void agregarANuevo(char* linea) {

	log_info(log_kernel, "Por bloquear Mutex de Cola Nuevos");
	mutexBloquear(&mutexColaNuevos);
	list_add(colaNuevos, linea);
	mutexDesbloquear(&mutexColaNuevos);

	log_info(log_kernel,
			"Se desbloqueo la cola de nuevos y se agrego la linea a la cola de nuevos");

}

t_pcb* crearEstructurasAdministrativas(char* linea) {

	log_info(log_kernel, "Por llamar a mutexBloquear y aumentar countPID: %d",
			countPID);

	mutexBloquear(&countProcess);
	countPID++;
	mutexDesbloquear(&countProcess);

	log_info(log_kernel, "Ya desbloqueamos el mutex y countPID quedo en: %d",
			countPID);

	log_info(log_kernel, "Por crear el PCB");

	t_pcb* proceso;

	proceso = crearPcb(linea);

	log_info(log_kernel, "PCB creado ==> PID: %d", proceso->pid);

	return proceso;
}

void planificar(char* linea) {

	log_info(log_kernel,
			"En funcion planificar, por agregar la linea a la cola de nuevos");

	agregarANuevo(linea);

	log_info(log_kernel, "Por Crear estructuras administrativas");

	t_pcb* pcbProceso = crearEstructurasAdministrativas(linea);

	log_info(log_kernel,
			"Retornamos la estructura administrativa, se encuentra en %p",
			pcbProceso);

	if (pcbProceso == NULL) {

		printf("Hubo un error al crear las estructuras administrativas");

		log_error(log_kernel,
				"Hubo un error al crear las estructuras administrativas");

		return;
	}

	log_info(log_kernel, "Por agregar el PCB a Listo");

	agregarAListo(pcbProceso);

	//Valido multiprocesamiento

	semaforoValor(&multiprocesamiento, &valorMultiprocesamiento);

	log_info(log_kernel,
			"El valor del semaforo contador multiprocesamiento, antes de agregar a ejecutar un proceso es: %d",
			valorMultiprocesamiento);

	agregarAEjecutar(pcbProceso);

	//termino

	agregarAExit();

	//2 LINEAS AGRGADAS PARA LIMPIAR LEAKS

	free(pcbProceso->linea);

	free(pcbProceso);

}

void agregarAListo(t_pcb* pcbParaAgregar) {

	log_info(log_kernel, "Verificamos si la cola de nuevos tiene un elemento");

	//Eliminar de la lista de new

	pcbParaAgregar->estado = listo;

	log_info(log_kernel, "Sacamos el elemento de la cola de nuevos");

	if (list_size(colaNuevos) > 0) {

		mutexBloquear(&mutexColaNuevos);
		list_remove(colaNuevos, 0);
		mutexDesbloquear(&mutexColaNuevos);
	}
	log_info(log_kernel,
			"Bloqueamos Mutex para poder insertar el elemento en la cola de listos");

	mutexBloquear(&mutexColaListos);
	list_add(colaListos, pcbParaAgregar);
	mutexDesbloquear(&mutexColaListos);

	log_info(log_kernel,
			"Desbloqueamos el mutex y agregamos el PCB a la cola de listos.");

	log_info(log_kernel, "Salimos de la funcion AgregarAListo");

}

void agregarAEjecutar(t_pcb* pcbParaAgregar) {

	req_com_t req;

	log_info(log_kernel, "En funcion agregarAEjecutar");

	log_info(log_kernel, "En cola de listos tenemos: %d elementos",	listaCantidadElementos(colaListos));

	char** pruebaPath = string_split(pcbParaAgregar->linea, separator);

	semaforoWait(&multiprocesamiento);
	FILE* fd;
	fd = fopen(pruebaPath[1], "r");

	while (listaCantidadElementos(colaListos) > 0) {

		int count = 0;
		log_info(log_kernel,"Entrando While == colaListos > 0");
		if (pcbParaAgregar->comando == RUN) {

			//FILE* fd;

			//fd = fopen(pruebaPath[1], "r");

			log_info(log_kernel, "Valor de Rafaga es %d",pcbParaAgregar->rafaga);
			log_info(log_kernel, "Valor de programCounter es %d",pcbParaAgregar->progamCounter);
			log_info(log_kernel, "Le queda por ejecutar %d",pcbParaAgregar->rafaga - pcbParaAgregar->progamCounter);

			log_info(log_kernel, "Valor de Quantum es %d", arc_config->quantum);

			mutexBloquear(&mutexColaListos);
			list_remove(colaListos, 0);
			mutexDesbloquear(&mutexColaListos);
			//char* bufferRun = malloc(1024);
			char* bufferRun = malloc(100);
			char* bufferRun2 = malloc(100);


			//Rafaga restante del PCB sea mayor o igual que el Quantum
			if ((pcbParaAgregar->rafaga - pcbParaAgregar->progamCounter)>= arc_config->quantum) {

				for (int i = 1; arc_config->quantum >= i; i++) {

					count++;
					log_info(log_kernel, "Vuelta del FOR: %d", i);

					log_info(log_kernel, "Reservé memoria para bufferRun");

					bufferRun2 = fgets(bufferRun, 100, fd);

					log_info(log_kernel, "1");
					log_info(log_kernel, "Linea para ejecutar: %s", bufferRun2);

					pruebaPath = string_split(bufferRun2, separator);
					log_info(log_kernel, "El comando es: %s", pruebaPath[0]);
					int aux_comando = buscarComando(pruebaPath[0]);

					if (aux_comando == CREATE) {

						tablaPrueba.criterio = buscarCriterio(pruebaPath[2]);
						tablaPrueba.nombreTabla = malloc(
								strlen(pruebaPath[1]) + 1);
						strcpy(tablaPrueba.nombreTabla, pruebaPath[1]);
						log_info(log_kernel,
								"En la tabla/criterios se guardo el criterio: %d",
								tablaPrueba.criterio);
						log_info(log_kernel,
								"En la tabla/criterios se guardo el nombre %s",
								tablaPrueba.nombreTabla);
					}

					req.tam = strlen(bufferRun2) + 1;

					log_info(log_kernel, "Tamanio cadena grabada en req:%d",
							req.tam);

					req.str = malloc(req.tam);

					strcpy(req.str, bufferRun2);

					semaforoValor(&multiprocesamiento,
							&valorMultiprocesamiento);

					log_info(log_kernel,
							"El valor del semaforo contador multiprocesamiento, despues de agregar a ejecutar un proceso es: %d",
							valorMultiprocesamiento);

					log_info(log_kernel, "Cadena grabada en req:%s", req.str);

					pcbParaAgregar->estado = ejecucion;

					socket_CMemoria = conectar_a_memoria(criterio_memoria.listMemoriaas->ip,criterio_memoria.listMemoriaas->puerto);

					list_add(colaEjecucion, pcbParaAgregar);

					int respuesta = enviar_request(socket_CMemoria, req);

					if (respuesta != 0) {
						log_info(log_kernel,
								"Hubo un error al enviar la request a memoria");
						return;
					}
					log_info(log_kernel,
							"No Hubo error al enviar la request a memoria");

					msg_com_t msg = recibir_mensaje(socket_CMemoria);
					if (msg.tipo == RESPUESTA) {

						log_info(log_kernel,
								"Llego un mensaje de tipo RESPUESTA");

						resp_com_t respuesta = procesar_respuesta(msg);
						borrar_mensaje(msg);
						if (respuesta.tipo == RESP_OK) {
							printf("La respuesta fue correcta %d \n",respuesta.tipo);
							if(respuesta.msg.str != NULL)
								printf("Respuesta recibida %s\n",respuesta.msg.str);
							log_info(log_kernel,
									"La respuesta fue correcta luego de procesarla");
						} else {

							log_info(log_kernel,
									"La respuesta no fue correcta luego de procesarla");
						}

						borrar_respuesta(respuesta);

					}

					mutexBloquear(&mutexColaEjecucion);

					list_add(colaEjecucion, pcbParaAgregar);

					mutexDesbloquear(&mutexColaEjecucion);

					log_info(log_kernel,
							"Desbloqueamos el Mutex de Ejecucion y el PCB fue encolado a la cola de Ejecucion.");

					free(req.str);

					pcbParaAgregar->progamCounter++;

					log_info(log_kernel,
							"El valor del ProgramCounter para el proceso es: %d",
							pcbParaAgregar->progamCounter);

					log_info(log_kernel, "Ultima instruccion del FOR");

					//AGREGADO PARA LIMPIAR LEAKSs

					//free(bufferRun);

				}

				mutexBloquear(&mutexColaEjecucion);
				list_remove(colaEjecucion, 0);
				mutexDesbloquear(&mutexColaEjecucion);

				agregarAListo(pcbParaAgregar);

			} else {

				log_info(log_kernel,
						"===>Seccion de Quantum mayor que rafaga restante del proceso");

				int rafagaRestante = pcbParaAgregar->rafaga
						- pcbParaAgregar->progamCounter;

				log_info(log_kernel, "===>Rafaga restante: %d", rafagaRestante);

				for (int i = 1; rafagaRestante >= i; i++) {
					count++;
					char* bufferRun = malloc(1024);

					log_info(log_kernel, "Vuelta del FOR: %d", i);

					char* lineaRun = malloc(1024);

					lineaRun = fgets(bufferRun, 1024, fd);

					strtok(lineaRun, "\n");
					log_info(log_kernel, "Linea para ejecutar: %s", lineaRun);

					pruebaPath = string_split(lineaRun, separator);
					log_info(log_kernel, "El comando es: %s", pruebaPath[0]);
					int aux_comando = buscarComando(pruebaPath[0]);

					if (aux_comando == CREATE) {

						tablaPrueba.criterio = buscarCriterio(pruebaPath[2]);
						tablaPrueba.nombreTabla = malloc(
								strlen(pruebaPath[1]) + 1);
						strcpy(tablaPrueba.nombreTabla, pruebaPath[1]);
						log_info(log_kernel,
								"En la tabla/criterios se guardo el criterio: %d",
								tablaPrueba.criterio);
						log_info(log_kernel,
								"En la tabla/criterios se guardo el nombre %s",
								tablaPrueba.nombreTabla);
					}

					//strtok(lineaRun, "\n");
					req.tam = strlen(lineaRun) + 1;

					log_info(log_kernel, "Tamanio cadena grabada en req:%d",
							req.tam);

					req.str = malloc(req.tam);

					strcpy(req.str, lineaRun);

					log_info(log_kernel, "Cadena grabada en req:%s", req.str);

					semaforoValor(&multiprocesamiento,
							&valorMultiprocesamiento);

					log_info(log_kernel,
							"El valor del semaforo contador multiprocesamiento, despues de agregar a ejecutar un proceso es: %d",
							valorMultiprocesamiento);

					pcbParaAgregar->estado = ejecucion;

					mutexBloquear(&mutexColaEjecucion);

					socket_CMemoria = conectar_a_memoria(
							criterio_memoria.listMemoriaas->ip,
							criterio_memoria.listMemoriaas->puerto);

					list_add(colaEjecucion, pcbParaAgregar);

					int respuesta = enviar_request(socket_CMemoria, req);

					if (respuesta != 0) {
						log_info(log_kernel,
								"Hubo un error al enviar la request a memoria");
						return;
					}
					log_info(log_kernel,
							"No Hubo error al enviar la request a memoria");

					msg_com_t msg = recibir_mensaje(socket_CMemoria);
					if (msg.tipo == RESPUESTA) {

						log_info(log_kernel,
								"Llego un mensaje de tipo RESPUESTA");

						resp_com_t respuesta = procesar_respuesta(msg);
						if (respuesta.tipo == RESP_OK) {
							printf("La respuesta fue correcta %d: \n",
									respuesta.tipo);
							if(respuesta.msg.tam >0)
									printf("Respuesta recibida %s: \n",respuesta.msg.str);
							log_info(log_kernel,
									"La respuesta fue correcta luego de procesarla");
						} else {

							log_info(log_kernel,
									"La respuesta no fue correcta luego de procesarla");
						}

						borrar_respuesta(respuesta);

					}

					if (msg.tipo != RESPUESTA) {
						imprimirError(log_kernel,
								"[CREATE] Memoria no responde como se espera");
						borrar_mensaje(msg);

					}

					mutexDesbloquear(&mutexColaEjecucion);

					log_info(log_kernel,
							"Desbloqueamos el Mutex de Ejecucion y el PCB fue encolado a la cola de Ejecucion.");

					free(req.str);

					pcbParaAgregar->progamCounter =
							pcbParaAgregar->progamCounter + i;

					log_info(log_kernel, "Ultima instruccion del FOR");

					free(lineaRun);	//AGREGADO PARA LIMPIAR LEAKSs
					//free(bufferRun);

				}
			}
			

		} else {
			log_info(log_kernel, "Entro por ELSE. Ya que el comando vino por consola y no por RUN.");
			count++;
			mutexBloquear(&mutexColaListos);
			list_remove(colaListos, 0);
			mutexDesbloquear(&mutexColaListos);

			if (pcbParaAgregar->comando == CREATE) {

				tablaPrueba.criterio = buscarCriterio(pruebaPath[2]);
				tablaPrueba.nombreTabla = malloc(
				strlen(pruebaPath[1]) + 1);
				strcpy(tablaPrueba.nombreTabla, pruebaPath[1]);
				log_info(log_kernel,"En la tabla/criterios se guardo el criterio: %d",tablaPrueba.criterio);
				log_info(log_kernel,"En la tabla/criterios se guardo el nombre %s",tablaPrueba.nombreTabla);
			}

			req.tam = strlen(pcbParaAgregar->linea) + 1;

			log_info(log_kernel, "Tamanio cadena grabada en req:%d",req.tam);

			req.str = malloc(req.tam);

			strcpy(req.str, pcbParaAgregar->linea);

			log_info(log_kernel, "Cadena grabada en req:%s", req.str);

			semaforoValor(&multiprocesamiento,&valorMultiprocesamiento);

			log_info(log_kernel,"El valor del semaforo contador multiprocesamiento, despues de agregar a ejecutar un proceso es: %d",valorMultiprocesamiento);

			pcbParaAgregar->estado = ejecucion;

			//mutexBloquear(&mutexColaEjecucion);

			log_info(log_kernel,"Conectando con la memoria numero: %d",criterio_memoria.listMemoriaas->numMemoria);
			socket_CMemoria = conectar_a_memoria(criterio_memoria.listMemoriaas->ip,criterio_memoria.listMemoriaas->puerto);

			list_add(colaEjecucion, pcbParaAgregar);

			log_info(log_kernel,"Por enviar request a memoria");
			int respuesta = enviar_request(socket_CMemoria, req);

			if (respuesta != 0) {
				log_info(log_kernel,"Hubo un error al enviar la request a memoria");
				return;
			}
		
			log_info(log_kernel,"No Hubo error al enviar la request a memoria");

			msg_com_t msg = recibir_mensaje(socket_CMemoria);
			if (msg.tipo == RESPUESTA) {

				log_info(log_kernel,"Llego un mensaje de tipo RESPUESTA");

				resp_com_t respuesta = procesar_respuesta(msg);
						
				if (respuesta.tipo == RESP_OK) {
							printf("La respuesta fue correcta: %d \n",respuesta.tipo);
							log_info(log_kernel,"La respuesta fue correcta luego de procesarla");
							if(respuesta.msg.tam >0)
									printf("Respuesta recibida %s: \n",respuesta.msg.str);
				} else {
							printf("La respuesta no fue correcta, Llegó: %d\n",respuesta.tipo);
							log_info(log_kernel,"La respuesta no fue correcta luego de procesarla, llegó: %d",respuesta.tipo);
					}

				borrar_respuesta(respuesta);

			}

			if (msg.tipo != RESPUESTA) {
				imprimirError(log_kernel,"[CREATE] Memoria no responde como se espera");
				borrar_mensaje(msg);

			}/*
			mutexBloquear(&mutexColaEjecucion);
			list_remove(colaEjecucion, 0);
			mutexDesbloquear(&mutexColaEjecucion);		*/

			//DAM: NO se que sentido tiene que este req este aqui si no lo usa nadie y ademas es local
			free(req.str);	//LO AGREGO POR LAS DUDAS
			
		}

		log_info(log_kernel,"El total es: %d",count);

		log_info(log_kernel,
				"Al finalizar el while tenemos en cola de listos tenemos: %d elementos",
				listaCantidadElementos(colaListos));

		
	}
	//AGREGADO PARA LIMPIAR LEAKSs
	int indice = 0;
	while (pruebaPath[indice] != NULL) {
		free(pruebaPath[indice]);
		indice++;
	}
}

void agregarAExit() {

	semaforoValor(&multiprocesamiento, &valorMultiprocesamiento);

	log_info(log_kernel,
			"El valor del semaforo contador multiprocesamiento, antes de colocar un proceso en exit es: %d",
			valorMultiprocesamiento);

	semaforoSignal(&multiprocesamiento);

	semaforoValor(&multiprocesamiento, &valorMultiprocesamiento);

	log_info(log_kernel,
			"El valor del semaforo contador multiprocesamiento, despues de agregar un proceso a la cola exit es: %d",
			valorMultiprocesamiento);
}

void gossiping_Kernel() {

	inicializar_estructuras_gossiping(log_kernel, 10000);

	char auxPuerto[LARGO_PUERTO];

	sprintf(auxPuerto, "%d", arc_config->puerto_memoria);

	agregar_seed(-1, arc_config->ip_memoria, auxPuerto);

	pthread_t hiloGossiping;
	iniciar_hilo_gossiping(&soy, &hiloGossiping, actualizarMemoriasDisponibles);

	pthread_detach(hiloGossiping);

	/*	criterioSC;
	 criterioSHC;
	 criterioEC;*/

}

void actualizarMemoriasDisponibles() {

	//Logear diferencias de memorias TODO
	gossipingKernel = armar_vector_seeds(soy);

	log_info(log_kernel, "Cantidad Memorias: %d", gossipingKernel.cant);

}

seed_com_t* buscarMemoria(char** pruebaPath) {

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

	/*if (retval >= 0) {
	 log_info(log_kernel, "La memoria se encontró,devolviendo");
	 return retval;
	 } else {
	 log_info(log_kernel, "La memoria no se encontró");
	 return retval;
	 }*/
}

void comandoAdd(char** comandoSeparado) {

	log_info(log_kernel, "Llego el comando ADD.");

	seed_com_t* resultado = buscarMemoria(comandoSeparado);

	if (resultado != NULL) {

		log_info(log_kernel, "La memoria numero %s ha sido encontrada.\n",
				comandoSeparado[2]);

		log_info(log_kernel, "El criterio para asociar es el: %s",
				comandoSeparado[4]);
		int criterioInt = buscarCriterio(comandoSeparado[4]);
		log_info(log_kernel,
				"El criterio para asociar es el: %s,corresponde al valor: %d",
				comandoSeparado[4], criterioInt);
		criterio_memoria.criterio = criterioInt;
		criterio_memoria.listMemoriaas = resultado;
		list_add(lista_memorias, resultado);

		log_info(log_kernel, "Llenamos la estructura de criterio/memoria.");

		printf("La memoria: %s fue asociada al criterio %s con exito.\n",
				comandoSeparado[2], comandoSeparado[4]);

	} else {

		printf("No se encontro la memoria.\n");
		log_info(log_kernel,
				"Ese numero de memoria no ha sido encontrada, la misma con numero: %s",
				comandoSeparado[2]);
	}

}

int buscarCriterio(char* criterio) {

	if (criterio == NULL) {
		log_info(log_kernel, "Recibimos el criterio: NULL");
		return -1;
	}

	log_info(log_kernel, "Recibimos el criterio: %s", criterio);

	int i = 0;

	for (i; i <= SALIR && strcmp(criterios[i], criterio); i++) {
	}

	log_info(log_kernel, "Se devuelve el valor %d", i);

	return i;
}

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
				if(respuesta.msg.tam >0)
													printf("Respuesta recibida %s: \n",respuesta.msg.str);
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
}

void comandoMetrics() {

}

int conectar_a_memoria(char ip[LARGO_IP], char puerto[LARGO_PUERTO]) {

	char puerto_memoria[20];
	snprintf(puerto_memoria, 19, "%d", arc_config->puerto_memoria);
	imprimirMensaje2(log_kernel,
			"[CONECTANDO A MEMORIA] Me voy a intentar conectar a ip: <%s> puerto: <%s>",
			arc_config->ip_memoria, puerto_memoria);
	int socket = conectar_a_servidor(ip, puerto, soy);
	if (socket == -1) {
		imprimirError(log_kernel,
				"[CONECTANDO A MEMORIA] No fue posible conectarse con la Memoria. TERMINANDO\n");
		return -1;
	}

	imprimirMensaje(log_kernel,
			"[CONECTANDO A MEMORIA] Me conecté con éxito a la MEMORIA. Espero su hs");
	//Si me conecté, espero su msg de bienvenida

	msg_com_t msg = recibir_mensaje(socket);

	if (msg.tipo != HANDSHAKECOMANDO) {
		borrar_mensaje(msg);
		imprimirError(log_kernel,
				"[CONECTANDO A MEMORIA] MEMORIA no responde el hs. TERMINANDO\n");
		return -1;
	}

	handshake_com_t hs = procesar_handshake(msg);
	borrar_mensaje(msg);

	if (hs.id == RECHAZADO) {
		if (hs.msg.tam == 0)
			imprimirError(log_kernel,
					"[CONECTANDO A MEMORIA] MEMORIA rechazo la conexión. TERMINANDO\n");
		else
			imprimirError1(log_kernel,
					"[CONECTANDO A MEMORIA] MEMORIA rechazo la conexión [%s]. TERMINANDO\n",
					hs.msg.str);
		borrar_handshake(hs);
		close(socket);
		return -1;
	}

	imprimirMensaje(log_kernel,
			"[CONECTANDO A MEMORIA] Me conecté con éxito a la MEMORIA");

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

		wd = inotify_add_watch(fd, pathDelArchivoAEscuchar,
		IN_MODIFY | IN_CREATE | IN_DELETE);
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

	log_info(log_kernel, "[ACTUALIZANDO RETARDOS y QUANTUM] Voy a actualizar");

	mutexBloquear(&mutex_retardos_kernel);

	t_config* auxConfigFile = config_create(path_config);

	if (auxConfigFile != NULL) {

		log_info(log_kernel,
				"[ACTUALIZANDO RETARDOS y QUANTUM] LEYENDO CONFIGURACION...");

		if (config_has_property(auxConfigFile, "SLEEP_EJECUCION")) {

			arc_config->sleep_ejecucion = config_get_int_value(auxConfigFile,
					"SLEEP_EJECUCION");
			log_info(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] SLEEP_EJECUCION: %d",
					arc_config->sleep_ejecucion);

		} else {
			log_error(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] NO HAY SLEEP_EJECUCION CONFIGURADO");
		} // SLEEP_EJECUCION

		if (config_has_property(auxConfigFile, "METADATA_REFRESH")) {

			arc_config->metadata_refresh = config_get_int_value(auxConfigFile,
					"METADATA_REFRESH");
			log_info(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] METADATA_REFRESH es de: %d",
					arc_config->metadata_refresh);

		} else {
			log_error(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] NO HAY METADATA_REFRESH CONFIGURADO");
		} // METADATA_REFRESH

		if (config_has_property(auxConfigFile, "QUANTUM")) {

			arc_config->quantum = config_get_int_value(auxConfigFile,
					"QUANTUM");
			log_info(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] Valor DEL QUANTUM: %d",
					arc_config->quantum);

		} else {
			log_error(log_kernel,
					"[ACTUALIZANDO RETARDOS y QUANTUM] NO HAY QUANTUM CONFIGURADO");
		} // QUANTUM

	} else {
		log_error(log_kernel,
				"[ACTUALIZANDO RETARDOS y QUANTUM] NO HAY ARCHIVO DE CONFIGURACION DE MODULO DEL KERNEL"); // ERROR: SIN ARCHIVO CONFIGURACION
	}

	config_destroy(auxConfigFile);

	log_info(log_kernel,
			"[ACTUALIZANDO RETARDOS y QUANTUM] RETARDOS y QUANTUM ACTUALIZADOS CORRECTAMENTE");

	mutexDesbloquear(&mutex_retardos_kernel);
}
