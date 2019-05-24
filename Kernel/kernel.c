/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"
#include "kernel_aux.h"

int main() {

	// (BEGIN) LISTA COMPARTIDA POR LOS HILOS. ALMACENA LOS QUERYS A EJECUTAR.
	list_queries = list_create();

	log_kernel = archivoLogCrear(LOG_PATH, "Proceso Kernel");

	log_info(log_kernel,
			"Ya creado el Log, continuamos cargando la estructura de configuracion, llamando a la funcion.");

	cargarConfiguracion();

	log_info(log_kernel,
			"Devuelta en Main, Funcion cargarConfiguracion() finalizo.");

	//multiProcesamiento = arc_config->multiprocesamiento;

	//sem_init(&multiprogramacion,0,arc_config->multiprocesamiento);

	log_info(log_kernel,
			"El valor que le vamos a poner al semaforo de multiprocesamiento es: %d",
			arc_config->multiprocesamiento);
	semaforoIniciar(&multiprocesamiento, arc_config->multiprocesamiento);

	inicializarListasPlanificador();

	mutexIniciar(&countProcess);
	mutexIniciar(&mutexColaNuevos);
	//mutexIniciar(&mutexMultiprocesamiento);

	countPID = 0;

	log_info(log_kernel, "Creamos thread para Consola");

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);

	//pthread_t hiloConexion;
	//pthread_create(&hiloConexion,NULL,(void*) conexionKernel,NULL);

	//pthread_join(hiloConexion,NULL);
	pthread_join(hiloConsola, NULL);

	mostrarQueries(log_kernel, list_queries);
	list_destroy(list_queries); // FREE de la lista de queries;

	log_info(log_kernel, "Salimoooos");

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

	free(configFile);

}

void consola() {

	log_info(log_kernel, "En el hilo de consola");

	menu();

	char bufferComando[MAXSIZE_COMANDO];

	while (1) {

		//printf(">");

		linea = readline(">");

		if (linea) {
			add_history(linea);
		}

		if (!strncmp(linea, "exit", 4)) {
			free(linea);
			break;
		}

		//fgets(bufferComando, MAXSIZE_COMANDO, stdin); -> Implementacion anterior

		strtok(linea, "\n");

		planificar(linea);

		char* lineaCopia = calloc(1, sizeof(linea) + 1);
		strncpy(lineaCopia, linea, sizeof(linea) + 1);
		// AÑADE LA LINEA A LA LISTA PRINCIPAL
		list_add(list_queries, (void*) lineaCopia);

		comandoSeparado = string_split(linea, separator);

		validarLinea(comandoSeparado, log_kernel);

		free(linea);

	}
}

void menu() {

	printf("Los comandos que se pueden ingresar son: \n"
			"COMANDOS \n"
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

void validarLinea(char** lineaIngresada, t_log* logger) {

	for (int i = 0; lineaIngresada[i] != NULL; i++) {

		log_info(log_kernel, "En la posicion %d del array esta el valor %s", i,
				lineaIngresada[i]);

		tamanio = i + 1;
	}

	log_info(log_kernel, "El tamanio del vector de comandos es: %d", tamanio);

	validarComando(lineaIngresada, tamanio, log_kernel);

}

int conexionKernel() { // Retorn socket por el cual se envían los mensajes.

	socket_CMemoria = nuevoSocket(log_kernel);

	if (socket_CMemoria == ERROR) {

		log_error(log_kernel,
				"Hubo un problema al querer crear el socket desde Kernel. Salimos del Proceso");

		return ERROR;
	}

	log_info(log_kernel, "El Socket creado es: %d .", socket_CMemoria);

	log_info(log_kernel,
			"por llamar a la funcion connectarSocket() para conectarnos con Memoria");

	log_info(log_kernel, "PRUEBA: %d ", arc_config->puerto_memoria);

	resultado_Conectar = conectarSocket(socket_CMemoria, arc_config->ip_memoria,
			arc_config->puerto_memoria, log_kernel);

	if (resultado_Conectar == ERROR) {
		log_error(log_kernel,
				"Hubo un problema al querer Conectarnos con Memoria. Salimos del proceso");
		return -1;
	} else {
		log_info(log_kernel, "Nos conectamos con exito, el resultado fue %d",
				resultado_Conectar);
		return socket_CMemoria;
	}
} // int conexionKernel()

void validarComando(char** comando, int tamanio, t_log* logger) {

	int resultadoComando = buscarComando(comando[0], logger);

	int tamanioCadena = 0;

	switch (resultadoComando) {

	case Select: {
		printf("Se selecciono Select\n");

		log_info(log_kernel, "Se selecciono select");

		if (tamanio == 3) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando select");

			//	t_pcb* procesoNuevo = crearEstructurasAdministrativas();

			//agregarAListo(procesoNuevo);

			mensaje = malloc(
					string_length(comando[1]) + string_length(comando[2]) + 1);

			log_info(log_kernel, "El tamanio de la cadena a guardar es: %d",
					tamanioCadena);

			strcpy(mensaje, comando[1]);
			strcpy(mensaje, comando[2]);

			log_info(log_kernel, "En mensaje ya tengo: %s", mensaje);

			armarMensajeBody(tamanio, mensaje, comando);

			log_info(log_kernel, "Por llamar a enviarMensaje");
			int resultadoEnviarComando = enviarMensaje(Select, tamanio, mensaje,
					log_kernel);
		}

	}
		break;

	case insert: {
		printf("Insert seleccionado\n");

		log_info(log_kernel, "Se selecciono insert");

		if (tamanio == 4 || tamanio == 5) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando insert");

			log_info(log_kernel, "Por llamar a enviarMensaje");

			if (tamanio == 4) {

				mensaje = malloc(
						string_length(comando[1]) + string_length(comando[2])
								+ string_length(comando[3]) + 2);
				log_info(log_kernel, "Por guardar memoria para 3 argumentos");
			} else {
				mensaje = malloc(
						string_length(comando[1]) + string_length(comando[2])
								+ string_length(comando[3])
								+ string_length(comando[4]) + 3);
				log_info(log_kernel, "Por guardar memoria para 4 argumentos");
			}

			log_info(log_kernel, "Por copiar el primer parametro del comando");

			strcpy(mensaje, comando[1]);

			log_info(log_kernel, "En mensaje ya tengo: %s", mensaje);

			armarMensajeBody(tamanio, mensaje, comando);
		}

		int resultadoEnviarComando = enviarMensaje(insert, tamanio, mensaje,
				log_kernel);
	}
		break;

	case create: {
		printf("Create seleccionado\n");
		log_info(log_kernel, "Se selecciono Create");

		if (tamanio == 5) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando create");

			mensaje = malloc(
					string_length(comando[1]) + string_length(comando[2])
							+ string_length(comando[3])
							+ string_length(comando[4]) + 3);

			strcpy(mensaje, comando[1]);

			armarMensajeBody(tamanio, mensaje, comando);

			log_info(log_kernel, "Por llamar a enviarResultado");

			int resultadoEnviarComando = enviarMensaje(create, tamanio, mensaje,
					log_kernel);
		}

	}
		break;

	case describe: {
		printf("Describe seleccionado\n");
		log_info(log_kernel, "Se selecciono Describe");

		if (tamanio == 1) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando Describe");

			mensaje = malloc(string_length(comando[0]));

			strcpy(mensaje, comando[0]);

			log_info(log_kernel, "En mensaje ya tengo: %s y es de tamanio: %d",
					mensaje, string_length(mensaje));

			log_info(log_kernel, "Por llamar a enviarMensaje");

			int resultadoEnviarComando = enviarMensaje(describe, tamanio,
					mensaje, log_kernel);

		}
		if (tamanio == 2) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando Describe");

			mensaje = malloc(string_length(comando[1]));

			strcpy(mensaje, comando[1]);

			log_info(log_kernel, "En mensaje ya tengo: %s y es de tamanio: %d",
					mensaje, string_length(mensaje));

			log_info(log_kernel, "Por llamar a enviarMensaje");

			int resultadoEnviarComando = enviarMensaje(describe, tamanio,
					mensaje, log_kernel);

		}

	}
		break;

	case drop: {
		printf("Drop seleccionado\n");
		log_info(log_kernel, "Se selecciono Drop");

		if (tamanio == 2) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando Drop");

			mensaje = malloc(string_length(comando[1]));

			strcpy(mensaje, comando[1]);

			log_info(log_kernel, "Queriamos mandar esto: %s", comando[1]);
			log_info(log_kernel, "Y se mando esto: %s", mensaje);

			int resultadoEnviarComando = enviarMensaje(drop, tamanio, mensaje,
					log_kernel);
		}

	}
		break;

	case add: {
		printf("Add seleccionado\n");
		log_info(log_kernel, "Se selecciono Add");

		if (tamanio == 5) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando add");

			printf(
					"Cantidad de parametros correctos ingresados para el comando add \n");

		}

	}
		break;

	case run: {
		printf("Run seleccionado\n");
		log_info(log_kernel, "Se selecciono Run");

		if (tamanio == 2) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando run");

			printf(
					"Cantidad de parametros correctos ingresados para el comando run \n");

			comandoRun(comando[1], logger);

		}

	}
		break;

	case journal: {

		printf("Journal seleccionado\n");
		log_info(log_kernel, "Se selecciono Journal");

		if (tamanio == 1) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando Journal");

			printf(
					"Cantidad de parametros correctos ingresados para el comando Journal \n");

		}

	}
		break;

	case metrics: {

		printf("Metrics seleccionado\n");
		log_info(log_kernel, "Se selecciono Metrics");

		if (tamanio == 1) {

			log_info(log_kernel,
					"Cantidad de parametros correctos ingresados para el comando Metrics");

			printf(
					"Cantidad de parametros correctos ingresados para el comando Metrics \n");

		}

	}
		break;

	default: {
		printf("Comando mal ingresado. \n");
		log_error(log_kernel, "Opcion mal ingresada por teclado en la consola");
	}
		break;

	}
}

int buscarComando(char* comando, t_log* logger) {

	log_info(logger, "Recibimos el comando: %s", comando);

	int i = 0;

	for (i; i <= salir && strcmp(comandosPermitidos[i], comando); i++) {

	}

	log_info(logger, "Se devuelve el valor %d", i);

	return i;

}

int enviarMensaje(int comando, int tamanio, char* mensaje, t_log* logger) {

	log_info(logger, "En funcion enviarMensaje.");

	t_header* headerParaEnviar = malloc(sizeof(t_header));

	log_info(logger, "Por guardar en la estructura del Header, el comando: %d",
			comando);
	headerParaEnviar->comando = comando;

	log_info(logger,
			"Por guardar en la estructura del Header, la cantidad de argumentos: %d",
			tamanio);
	headerParaEnviar->cantArgumentos = tamanio - 1; //Le restamos uno ya esta suma tiene en cuenta el comando

	log_info(logger, "Conectamos por socket con la memoria.");
	socket_CMemoria = conexionKernel();

	log_info(logger, "El tamanio del mensaje que se va a mandar es de: %d",
			string_length(mensaje));
	headerParaEnviar->tamanio = string_length(mensaje);

	log_info(logger, "tamanio del header a enviar: %d", sizeof(t_header));

	resultado_sendMsj = socketEnviar(socket_CMemoria, headerParaEnviar,
			sizeof(t_header), log_kernel);

	if (resultado_sendMsj == ERROR) {

		log_error(log_kernel, "Error al enviar mensaje a memoria. Salimos");

		return ERROR;
	}

	log_info(log_kernel, "El mensaje se envio correctamente");

	log_info(log_kernel, "Preparados para recibir %d",
			sizeof(confirmacionRecibida));
	int recibiendoMensaje = socketRecibir(socket_CMemoria,
			&confirmacionRecibida, sizeof(confirmacionRecibida), log_kernel);

	log_info(logger, "Lo que llego fue: %d", confirmacionRecibida);

	if (confirmacionRecibida == sizeof(t_header)) {

		log_info(logger,
				"La confirmacion que llego fue correcta, se procedera a enviar el body: %s con tamanio: %d",
				mensaje, strlen(mensaje));
		printf(
				"La confirmacion que llego fue correcta, se procedera a enviar el body: %s\n",
				mensaje);

		resultado_sendMsj = socketEnviar(socket_CMemoria, mensaje,
				strlen(mensaje), log_kernel);

		if (resultado_sendMsj == strlen(mensaje)) {

			log_info(logger, "Se envio el body correctamente");
			printf("Se envio el body Correctamente. \n");
		} else {

			log_error(logger, "El body se envio Incorrectamente");
			printf("El body se envio InCorrectamente. \n");
		}
	} else {
		log_error(logger,
				"La confirmacion que llego no es correcta, no se enviar el body");
		printf(
				"La confirmacion que llego no es correcta, no se envia el body. \n");

	}

	return 0;

}

void armarMensajeBody(int tamanio, char* mensaje, char** comando) {

	log_info(log_kernel, "En funcion armarMensajeBody");

	for (int i = 2; tamanio > i; i++) {

		string_append(&mensaje, &SEPARADOR);

		string_append(&mensaje, comando[i]);

		log_info(log_kernel, "Armando mensaje..: %s", mensaje);

	}

	log_info(log_kernel, "Finalizo el armado con el mensaje Final: %s",
			mensaje);
}

void comandoRun(char* path, t_log* logger) {

	fd = fopen(path, "r");

	if (fd == NULL) {

		log_info(log_kernel, "El archivo pasado por path no se encontró");
		printf("El archivo %s No existe\n", path);

		free(path);
		return;
	} else {

		log_info(log_kernel,
				"El archivo se encontró con exito. Vamos a leerlo");
		printf("El archivo buscado en la dirección %s existe. Vamos a leerlo\n",
				path);

		while (!feof(fd)) {

			log_info(log_kernel, "Dentro del while.");

			char bufferPath[MAXSIZE_COMANDO];

			fgets(bufferPath, MAXSIZE_COMANDO, fd);

			strtok(bufferPath, "\n");

			printf("Se leyo del archivo: %s\n", bufferPath);

			log_info(log_kernel, "El tamanio de la cadena es: %d",
					string_length(bufferPath));

			log_info(log_kernel, "Se leyo del archivo: %s", bufferPath);

			lineaSeparada = string_split(bufferPath, separator);

			validarLinea(lineaSeparada, logger);

		}

		log_info(log_kernel, "Fuera del while principal");

		fclose(fd);
		free(path);

		return;
	}
}

void inicializarListasPlanificador() {

	colaNuevos = list_create();
	colaListos = list_create();
	colaExit = list_create();
	colaEjecucion = list_create();
}

t_pcb* crearPcb(char* comando) {

	log_info(log_kernel, "Creando PCB ==> PID: %d", countPID);

	t_pcb* pcbProceso = malloc(sizeof(t_pcb));

	comandoSeparado = string_split(linea, separator);

	for (int i = 0; comandoSeparado[i] != NULL; i++) {

		log_info(log_kernel, "En la posicion %d del array esta el valor %s", i,
				comandoSeparado[i]);

		tamanio = i + 1;
	}

	int comando_aux = buscarComando(comandoSeparado[0], log_kernel);

	//En caso que sea el comando run voy a leer el archivo
	if (comando_aux == run) {

		log_info(log_kernel,"Vino Run de comando, vamos a buscar cuantas lineas tiene el archivo");
		int aux_rafaga = rafagaComandoRun(comandoSeparado[1]);

		pcbProceso->pid = countPID;
		pcbProceso->comando = comando_aux;
		pcbProceso->rafaga = aux_rafaga;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estadp en la cola new porque recien se crea

	}
	//en caso que sea otro comando la rafaga es 1
	else {
		log_info(log_kernel,"En la condicion de que no es un comando RUN");
		pcbProceso->pid = countPID;
		pcbProceso->comando = comando_aux;
		pcbProceso->rafaga = 1;
		pcbProceso->argumentos = tamanio - 1;
		pcbProceso->estado = 0; //Estadp en la cola new porque recien se crea
	}
	return pcbProceso;
}

int rafagaComandoRun(char* path) {

	log_info(log_kernel, "Vamos a buscar la cantidad de lineas que tiene el archivo");
	int caracter, contador;

	contador = 0;

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

			if(caracter == '\n'){

				contador++;
			}


		}

		log_info(log_kernel, "Fuera del while principal, la cantidad de lineas del archivo es: %d",contador);
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

	//semaforoWait(&multiprocesamiento);

	agregarAEjecutar();

	//termino

	agregarAExit();

}

void agregarAListo(t_pcb* pcbParaAgregar) {

	log_info(log_kernel, "Verificamos si la cola de nuevos tiene un elemento");

	if (list_size(colaNuevos) > 0) {

		//Eliminar de la lista de new

		pcbParaAgregar->estado = listo;

		log_info(log_kernel, "Sacamos el elemento de la cola de nuevos");

		mutexBloquear(&mutexColaNuevos);
		list_remove(colaNuevos, 0);
		mutexDesbloquear(&mutexColaNuevos);

		log_info(log_kernel,
				"Bloqueamos Mutex para poder insertar el elemento en la cola de listos");

		mutexBloquear(&mutexColaListos);
		list_add(colaListos, pcbParaAgregar);
		mutexDesbloquear(&mutexColaListos);

		log_info(log_kernel,
				"Desbloqueamos el mutex y agregamos el PCB a la cola de listos.");
	}

	log_info(log_kernel, "Salimos de la funcion AgregarAListo");

}

void agregarAEjecutar() {

	t_pcb* pcbListoParaEjecutar;

	semaforoValor(&multiprocesamiento, &valorMultiprocesamiento);

	log_info(log_kernel,
			"El valor del semaforo contador multiprocesamiento, antes de agregar a ejecutar un proceso es: %d",
			valorMultiprocesamiento);

	semaforoWait(&multiprocesamiento);

	log_info(log_kernel,
			"Bloqueamos Mutex para poder insertar el elemento en la cola de listos");

	mutexBloquear(&mutexColaListos);
	pcbListoParaEjecutar = list_remove(colaListos, 0);
	mutexDesbloquear(&mutexColaListos);

	semaforoValor(&multiprocesamiento, &valorMultiprocesamiento);

	log_info(log_kernel,
			"El valor del semaforo contador multiprocesamiento, despues de agregar a ejecutar un proceso es: %d",
			valorMultiprocesamiento);

	pcbListoParaEjecutar->estado = ejecucion;

	mutexBloquear(&mutexColaEjecucion);
	list_add(colaEjecucion, pcbListoParaEjecutar);
	mutexDesbloquear(&mutexColaEjecucion);

	log_info(log_kernel,
			"Desbloqueamos el Mutex de Ejecucion y el PCB fue encolado a la cola de Ejecucion.");

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
