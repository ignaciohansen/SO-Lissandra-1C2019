/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"
/*
 * REQUERIMIENTOS:
 *  - ¿ verificarExistenciaTabla    () ? Nota: está hecho el método de verificarExistenciaTabla()
 *  - crearTabla(nombre, tipoConsistencia, nroParticiones, compactationTime)  // e.g.: CREATE TABLA1 SC 4 60000
 *  - describe(nombre)
 *  - bool      :verificarExistenciaTabla(nombre)
 *  - obtenerMetadata(nombre)                           // ver que hacer acá
 *  - crearMemtable()
 *  - << todo lo necesario para gestionar las memTables >>
 *  - registro**:escanearTabla    (nombre,key)          // retorna un array de punterosa registros.
 *              :escaneaBinario   (key)
 *              :escaneaTemp      (key)
 *              :escaneaTempc     (key)
 *  - registro**:escanearMemtable (key)                 // retorna un array de punterosa registros.
 *
 *  NOTA: nombres de tablas no se distingue uppercase de lowercase. Doesn't do difference by cases.
 */
int main() {

	pantallaLimpiar();

	sem_init(&semaforoQueries, 0, 1);
	list_queries = list_create();

	LisandraSetUP(); // CONFIGURACIÓN Y SETEO SOCKET

	pthread_t* hiloListening, hiloConsola, hiloEjecutor;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_create(&hiloListening, NULL, (void*) listenSomeLQL, NULL);

	//pthread_create(&hiloEjecutor , NULL,(void*) consola, NULL);

	pthread_join(hiloListening, NULL);
	pthread_join(hiloConsola  , NULL);

	sem_destroy(&semaforoQueries);
	// consola();
	return 0;
}

/********************************************************************************************
 * 							SET UP LISANDRA, FILE SYSTEM Y COMPRIMIDOR
 ********************************************************************************************
 */

void LisandraSetUP() {

	imprimirMensajeProceso("Iniciando el modulo LISSANDRA FILE SYSTEM\n");

	logger = archivoLogCrear(LOG_PATH, "Proceso Lissandra File System");

	imprimirVerde(logger,
			"[LOG CREADO] continuamos cargando la estructura de configuracion.");

	if (cargarConfiguracion()) {
		//SI SE CARGO BIEN LA CONFIGURACION ENTONCES PROCESO DE ABRIR UN SERVIDOR
		imprimirMensajeProceso("Levantando el servidor del proceso Lisandra");
		abrirServidorLissandra();
	}

}

int abrirServidorLissandra() {

	socketEscuchaMemoria = nuevoSocket(logger);

	if (socketEscuchaMemoria == ERROR) {
		imprimirError(logger, "[ERROR] Fallo al crear Socket.");
		return -1;
	} else {
		imprimirVerde1(logger, "[  OK  ] Se ha creado el socket nro.: %d.",
				socketEscuchaMemoria);

	}

	int puerto_a_escuchar = configFile->puerto_escucha;

	imprimirMensaje1(logger, "[PUERTO] Asociando a puerto: %i.",
			puerto_a_escuchar);

	asociarSocket(socketEscuchaMemoria, puerto_a_escuchar, logger);

	imprimirMensaje(logger, "[  OK  ] Asociado.");

	socketEscuchar(socketEscuchaMemoria, 10, logger);

	return 1;

} // int abrirServidorLissandra()

bool cargarConfiguracion() {

	log_info(logger, "Por reservar memoria para variable de configuracion.");

	configFile = malloc(sizeof(t_lfilesystem_config));

	t_config* archivoCOnfig;

	log_info(logger,
			"Por crear el archivo de config para levantar archivo con datos.");

	archivoCOnfig = config_create(PATH_LFILESYSTEM_CONFIG);

	if (archivoCOnfig == NULL) {
		imprimirMensajeProceso(
				"NO se ha encontrado el archivo de configuracion\n");
		log_info(logger, "NO se ha encontrado el archivo de configuracion");
	}

	if (archivoCOnfig != NULL) {
		int ok = 1;
		imprimirMensajeProceso(
				"Se ha encontrado el archivo de configuracion\n");

		log_info(logger, "LissandraFS: Leyendo Archivo de Configuracion...");

		if (config_has_property(archivoCOnfig, "PUERTO_ESCUCHA")) {

			log_info(logger, "Almacenando el puerto");

			//Por lo que dice el texto
			configFile->puerto_escucha = config_get_int_value(archivoCOnfig,
					"PUERTO_ESCUCHA");

			log_info(logger, "El puerto de escucha es: %d",
					configFile->puerto_escucha);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el PUERTO_ESCUCHA");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "PUNTO_MONTAJE")) {

			log_info(logger, "Almacenando el PUNTO DE MONTAJE: %s",
					config_get_string_value(archivoCOnfig, "PUNTO_MONTAJE"));

			//Por lo que dice el texto		

			configFile->punto_montaje = config_get_string_value(archivoCOnfig,
					"PUNTO_MONTAJE");

			log_info(logger, "El punto de montaje es: %d",
					configFile->punto_montaje);

			tabla_Path = malloc(string_length(configFile->punto_montaje) + 8);

			tabla_Path = string_duplicate(configFile->punto_montaje);

			log_info(logger, "La variabla tabla_path queda con: %s",
					tabla_Path);

			strtok(tabla_Path, "\"");
			strtok(tabla_Path, "\"");

			string_append(&tabla_Path, "/Tables/");

			log_info(logger, "Y ahora la variabla tabla_path queda con: %s",
					tabla_Path);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el PUNTO_MONTAJE");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "RETARDO")) {

			log_info(logger, "Almacenando el retardo");

			//Por lo que dice el texto
			configFile->retardo = config_get_int_value(archivoCOnfig,
					"RETARDO");

			log_info(logger, "El retardo de respuesta es: %d",
					configFile->retardo);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene RETARDO");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "TAMANIO_VALUE")) {

			log_info(logger, "Almacenando el tamaño del valor de una key");

			//Por lo que dice el texto
			configFile->tamanio_value = config_get_int_value(archivoCOnfig,
					"TAMANIO_VALUE");

			log_info(logger, "El tamaño del valor es: %d",
					configFile->tamanio_value);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el TAMANIO_VALUE");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "TIEMPO_DUMP")) {

			log_info(logger, "Almacenando el puerto");

			//Por lo que dice el texto
			configFile->tiempo_dump = config_get_int_value(archivoCOnfig,
					"TIEMPO_DUMP");

			log_info(logger, "El tiempo de dumpeo es: %d",
					configFile->tiempo_dump);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el TIEMPO_DUMP");
			ok--;

		}

		if (ok > 0) {
			imprimirVerde(logger,
					"Se ha cargado todos los datos del archivo de configuracion");
			//	log_info(logger, "Se ha cargado todos los datos del archivo de configuracion");
			return true;

		} else {
			imprimirError(logger,
					"ERROR: No Se han cargado todos o algunos los datos del archivo de configuracion\n");
			//		imprimirMensajeProceso("ERROR: No Se han cargado todos los datos del archivo de configuracion\n");
			return false;
		}

	}

}

void consola() {

	log_info(logger, "En el hilo de consola");

	menu();

	char bufferComando[MAXSIZE_COMANDO];
	char **comandoSeparado;

	while (1) {

		//printf(">");

		linea = readline(">");

		if (linea) {
			add_history(linea);

			sem_wait(&semaforoQueries);
			list_add(list_queries, linea);
			sem_post(&semaforoQueries);
		}

		if (!strncmp(linea, "exit", 4)) {
			free(linea);
			break;
		}

		//fgets(bufferComando, MAXSIZE_COMANDO, stdin); -> Implementacion anterior

		strtok(linea, "\n");

		comandoSeparado = string_split(linea, separator);

		validarLinea(comandoSeparado, logger);

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
			"SALIR \n"
			"\n");

}

void validarLinea(char** lineaIngresada, t_log* logger) {

	for (int i = 0; lineaIngresada[i] != NULL; i++) {

		log_info(logger, "En la posicion %d del array esta el valor %s", i,
				lineaIngresada[i]);

		// log_info(logger,);

		tamanio = i + 1;
	}

	log_info(logger, "El tamanio del vector de comandos es: %d", tamanio);

	switch (tamanio) {

	case 1: {
		if (strcmp(lineaIngresada[0], "describe") == 0) {

			printf("Describe seleccionado\n");

		} else {
			printf("Comando mal ingresado. \n");

			log_error(logger, "Opcion mal ingresada por teclado en la consola");
			break;
		}
		break;
	}
	case 2:
		validarComando(lineaIngresada, tamanio, logger);

		break;

	case 3:
		validarComando(lineaIngresada, tamanio, logger);

		break;

	case 4:
		validarComando(lineaIngresada, tamanio, logger);

		break;

	case 5:
		validarComando(lineaIngresada, tamanio, logger);

		break;

	default: {
		printf("Comando mal ingresado. \n");

		log_error(logger, "Opcion mal ingresada por teclado en la consola");
	}

		break;

	}
}

void validarComando(char** comando, int tamanio, t_log* logger) {

	int resultadoComando = buscarComando(comando[0], logger);

	int tamanioCadena = 0;

	switch (resultadoComando) {

	case Select: {
		printf("Se selecciono Select\n");

		log_info(logger, "Se selecciono select");

		if (tamanio == 3) {

			log_info(logger,
					"Cantidad de parametros correctos ingresados para el comando select");

			int resultado = comandoSelect(comando[1], comando[2]);

			log_info(logger, "El resultado de la operacion fue: %d", resultado);

		}

	}
		break;

	case insert: {
		printf("Insert seleccionado\n");

		log_info(logger, "Se selecciono insert");

		if (tamanio == 4 || tamanio == 5) {

			if (tamanio == 4) {

			} else {

			}

		}

	}
		break;

	case create: {
		printf("Create seleccionado\n");
		log_info(logger, "Se selecciono Create");

	}
		break;

	case describe: {
		printf("Describe seleccionado\n");
		log_info(logger, "Se selecciono Describe");

		if (tamanio == 2) {

			log_info(logger,
					"Cantidad de parametros correctos ingresados para el comando Describe");

			mensaje = malloc(string_length(comando[1]));

			strcpy(mensaje, comando[1]);

			log_info(logger, "En mensaje ya tengo: %s y es de tamanio: %d",
					mensaje, string_length(mensaje));

			log_info(logger, "Por llamar a enviarMensaje");

		}

	}
		break;

	case drop: {
		printf("Drop seleccionado\n");
		log_info(logger, "Se selecciono Drop");

		if (tamanio == 2) {

			log_info(logger,
					"Cantidad de parametros correctos ingresados para el comando Drop");

			mensaje = malloc(string_length(comando[1]));

			strcpy(mensaje, comando[1]);

			log_info(logger, "Queriamos mandar esto: %s", comando[1]);
			log_info(logger, "Y se mando esto: %s", mensaje);

		}

	}
		break;

	default: {
		printf("Comando mal ingresado. \n");
		log_error(logger, "Opcion mal ingresada por teclado en la consola");
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

// LOGGEA todo lo que escucha.
void listenSomeLQL() {

	char bufferComando[MAXSIZE_COMANDO];
	char **comandoSeparado;

	while (1) {

		imprimirMensaje(logger,
				" \n ====== LFS Listener: waiting for client connections ====== \n ");

		conexionEntrante = aceptarConexionSocket(socketEscuchaMemoria, logger);

		//Puntero buffer = (void*)string_new(); // malloc(sizeof(char)*100);

		Puntero buffer = malloc(sizeof(char) * 100);

		recibiendoMensaje = socketRecibir(conexionEntrante, buffer, 50, logger);

		char* msg = string_new();

		// char * msg = malloc(sizeof(char)*100);
		msg = string_duplicate(buffer); // <-- ésto hace funcionar del string por red.

		sem_wait(&semaforoQueries);
		list_add(list_queries, msg);
		sem_post(&semaforoQueries);

		comandoSeparado = string_split(msg, separator);

		validarLinea(comandoSeparado, logger);

		string_append(&msg, "Mensaje recibido: \"");
		string_append(&msg, buffer);
		string_append(&msg, "\".");

		imprimirVerde(logger, msg);
		// liberar ¿msg?
		free(buffer);

	}

}

int comandoSelect(char* tabla, char* key) {

	verificarTabla(tabla);

	obtenerMetadata(tabla); // 0: OK. -1: ERROR.

	int valorKey    = atoi(key);
	int particiones = determinarParticion(valorKey, metadata->particiones);

	escanearParticion(particiones);

	log_info(logger, "LLEGÓ ACÁ.");

	char* keyEncontrado = buscarBloque(key); // GUardar memoria

	// ver key con timestamp mas grande

	return valorKey;

}

//void insert(char* tabla, int key,char* value){
// verificarTabla(tabla);
// obtener el metadata de la tabla
// verificar si existe lista a dumpear
//

//}

int verificarTabla(char* tabla) {

	tablaAverificar = malloc(string_length(tabla_Path) + string_length(tabla));

	log_info(logger,
			"Se reservo memoria para contatenar punto de montaje con la tabla");
	tablaAverificar = string_new();

	log_info(logger, "%s", tablaAverificar);
	string_append(&tablaAverificar, tabla_Path);
	string_append(&tablaAverificar, tabla);
	log_info(logger, "Concatenamos: %s a tablaAVerificar", tabla);
	path_tabla_metadata = string_duplicate(tablaAverificar);
	string_append(&path_tabla_metadata, "/metadata");
	log_info(logger, "[VERIFICADO] La direccion de la tabla que se quiere verificar es: %s",tablaAverificar);


	FILE *file;

	file = fopen(tablaAverificar, "r");

	if (file == NULL) {
		log_error(logger, "[ERROR] No existe la tabla");
		return 0;
		perror("Error!!");
	} else {
		log_error(logger, "[ OK ] Metadata de tabla abierto. \n");
		return 1;
		fclose(file);
	} // if (file == NULL)

} // int verificarTabla(char* tabla)

int obtenerMetadata(char* tabla) {

	log_info(logger, "[obtenerMetadata] (+) metadata a abrir : %s",path_tabla_metadata);

	int result = 0;

	metadata = malloc(sizeof(t_metadata_tabla)); // Vatiable global.

	t_config* metadataFile;
	metadataFile = config_create(path_tabla_metadata);

	if (metadataFile != NULL) {

		log_info(logger, "LFS: Leyendo metadata...");

		if (config_has_property(metadataFile, "CONSISTENCY")) {

			log_info(logger, "Almacenando consistencia");
				// PROBLEMA.
			metadata->consistency = config_get_string_value( metadataFile, "CONSISTENCY");

			log_info(logger, "La consistencia  es: %d", metadata->consistency);

		} else {

			log_error(logger, "El metadata no contiene la consistencia");

		} // if (config_has_property(metadataFile, "CONSISTENCY"))

		if (config_has_property(metadataFile, "PARTITIONS")) {

			log_info(logger, "Almacenando particiones");

			metadata->particiones = config_get_int_value( metadataFile, "PARTITIONS");

			log_info(logger, "Las particiones son : %d", metadata->particiones);

		} else {

			log_error(logger, "El metadata no contiene particiones");

		} // if (config_has_property(metadataFile, "PARTITIONS"))

		if (config_has_property(metadataFile, "COMPACTION_TIME")) {

			metadata->compaction_time = config_get_int_value( metadataFile, "COMPACTION_TIME");

			log_info(logger, "el tiempo de compactacion es: %d", metadata->compaction_time);

		} else {

			log_error(logger, "El metadata no contiene el tiempo de compactacion");

		} // if (config_has_property(metadataFile, "COMPACTION_TIME"))

	} else {

		log_error(logger, "[ERROR] Archivo metadata de partición no encontrado.");

		result = -1;

	} // if (metadataFile != NULL)

	log_info(logger, "[FREE] variable metadataFile utlizada para navegar el metadata.");

	free(metadataFile);

	log_info(logger, "[obtenerMetadata] (-) metadata a abrir : %s",tablaAverificar);

	return result;

} // int obtenerMetadata()

int determinarParticion(int key, int particiones) {

	log_info(logger, "KEY: %d ", key);

	int retornar = key % particiones;

	log_info(logger, "PARTICION: %d ", retornar);

	return retornar;

}

void escanearParticion(int particion) {

	log_info(logger,"[escanearParticion] (+) ");

	log_info(logger,"[escanearParticion] (+) %s",tabla_Path);

	log_info(logger,"[escanearParticion] (+) %s",tablaAverificar);

	char * stringParticion = malloc(sizeof(char));

	sprintf(stringParticion, "%d", particion);
	log_info(logger, "resultado de sprintf %s", stringParticion);

	char* archivoParticion = malloc(
			string_length(tablaAverificar) + string_length(stringParticion)
					+ string_length(PATH_BIN));

	log_info(logger,
			"Se reservo memoria para concatenar ruta de la tabla con la particion");
	archivoParticion = string_new();
	string_append(&archivoParticion, tablaAverificar);

	string_append(&archivoParticion, "/");

	string_append(&archivoParticion, stringParticion);

	string_append(&archivoParticion, ".bin");

	particionTabla = malloc(sizeof(t_particion));

	t_config* particionFile;

	particionFile = config_create(archivoParticion);

	log_info(logger, "%s", archivoParticion);

	FILE *file;
	file = fopen(archivoParticion, "r");
	if (file == NULL) {
		//log_error(logger, "No existe la particion");
		perror("Error");
	} else {
		log_info(logger, "Abrimos particion %d", particion);
		fclose(file);
	}

	if (particion != NULL) {

		log_info(logger, "LFS: Leyendo metadata de la particion...");

		if (config_has_property(particionFile, "SIZE")) {

			log_info(logger, "Almacenando el tamaño de la particion");

			particionTabla->size = config_get_int_value(particionFile, "SIZE");

			log_info(logger, "el tamaño de la particion  es: %d",
					particionTabla->size);
		} else {
			log_error(logger, "El metadata de la tabla no contiene el tamaño");

		}
		if (config_has_property(particionFile, "BLOCKS")) {

			log_info(logger, "Almacenando los bloques");

			particionTabla->bloques = config_get_array_value(particionFile,
					"BLOCKS");

			log_info(logger, "Las bloques son : %s",
					particionTabla->bloques[0]);
		} else {
			log_error(logger, "El metadata de la tabla no contiene bloques");

		}

	} else {

		log_error(logger,
				"No se encontro el metadata para cargar la estructura");

	}

	log_info(logger,
			"Cargamos todo lo que se encontro en el metadata. Liberamos la variable metadataFile que fue utlizada para navegar el metadata");

	free(particionFile);

	log_info(logger,"[escanearParticion] (-) ");

//LO que habia antes para levantar el archivo de metadata.bin
	/*	FILE *file;
	 file = fopen(archivoParticion, "r");
	 if (file == NULL) {
	 //log_error(logger, "No existe la particion");
	 perror("Error");
	 } else {
	 log_info(logger, "Abrimos particion %d",particion);
	 fclose(file);
	 }*/
}

char* buscarBloque(char* key) {

	char* bloqueObjetivo = malloc(
			string_length(configFile->punto_montaje)
					+ string_length(PATH_BLOQUES) + 11);

	log_info(logger, "Se reservo memoria para concatenar ruta de blqoues");

	bloqueObjetivo = string_new();

	string_append(&bloqueObjetivo, configFile->punto_montaje);

	log_info(logger, "BloqueObjetivo: %s", bloqueObjetivo);

	string_append(&bloqueObjetivo, PATH_BLOQUES);
	log_info(logger, "BloqueObjetivo: %s", bloqueObjetivo);

	char* bloque = malloc(2);

	bloque = particionTabla->bloques[0];

	string_append(&bloqueObjetivo, bloque);
	log_info(logger, "BloqueObjetivo: %s", bloqueObjetivo);

	string_append(&bloqueObjetivo, ".bin");
	log_info(logger, "BloqueObjetivo: %s", bloqueObjetivo);

	FILE *file;
	file = fopen(bloqueObjetivo, "r");

	if (file == NULL) {
		//log_error(logger, "No existe la particion");
		perror("Error");
	} else {
		log_info(logger, "Abrimos Bloque");
		fclose(file);
	}

	return "";

}

