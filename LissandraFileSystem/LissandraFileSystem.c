/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"


int cantidad_de_dumps = 0;
int dumps_a_dividir = 1;


//pthread_mutex_t mutex_dump = PTHREAD_MUTEX_INITIALIZER; //@martin, @gian, @nacho lo puse cuando hice el dump pero habría que revisarlo al hacer la sincro

int main(int argc, char **argv) {
	bool logEnConsola = true;
	if(argc == 2){
		if(!strcmp(argv[1],"-cl"))
			logEnConsola = false;
	}


	pantallaLimpiar();

	sem_init(&semaforoQueries, 0, 1);
	mutexIniciar(&memtable_mx);
	mutexIniciar(&listaTablasInsertadas_mx);
	//mutexIniciar(&semaforo); MERGE

//	list_queries = list_create();

	LisandraSetUP(logEnConsola); // CONFIGURACION Y SETEO SOCKET

	cargarBitmap();

	char puertoString[LARGO_PUERTO];
	snprintf(puertoString, LARGO_PUERTO, "%d", configFile->puerto_escucha);
	int socketLFS = iniciar_servidor(configFile->ip, puertoString); // AGREGAR IP A ARCHIV CONFIG @martin
	if (socketLFS == -1) {
		printf("%d ****************************** ", socketLFS);
		return 0;
	}

	inicializar_comunicacion(logger, configFile->tamanio_value); //tamanio value config

	pthread_t hiloConsola, hiloDump, hiloServidor, hiloInotifyLFS;

	char* path_de_config_lfs = malloc(strlen(PATH_LFILESYSTEM_CONFIG)+1);
	strcpy(path_de_config_lfs, PATH_LFILESYSTEM_CONFIG);
	pthread_create(&hiloInotifyLFS,NULL, (void *)inotifyAutomaticoLFS,path_de_config_lfs);
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_create(&hiloServidor, NULL, (void*) hilo_servidor, &socketLFS);
	pthread_create(&hiloDump, NULL, (void*) esperarTiempoDump, NULL);
	pthread_detach(hiloInotifyLFS);
	pthread_detach(hiloDump);
	pthread_detach(hiloServidor);

	signal(SIGINT, INThandler);
	//pthread_join(hiloDump, NULL);

	pthread_join(hiloConsola, NULL);

	//pthread_kill(hiloDump, SIGKILL);
	//pthread_kill(hiloServidor, SIGKILL);
	pthread_cancel(hiloInotifyLFS);
	pthread_cancel(hiloDump);
	pthread_cancel(hiloServidor);


	if (socketLFS == -1) {
		close(socketLFS);
	}

	cerrar_todos_clientes(); //@LFSCOMUNICACION
	cerrarTodo();

	return 0;
}

/********************************************************************************************
 * 							SET UP LISANDRA, FILE SYSTEM Y COMPRIMIDOR
 ********************************************************************************************
 */

void LisandraSetUP(bool logEnConsola) {

	imprimirMensajeProceso("==Iniciando el modulo LISSANDRA FILE SYSTEM==\n");
	archivoLogValidar(LOG_PATH);
//	logger = archivoLogCrear(LOG_PATH, "Proceso Lissandra File System");
	logger = log_create(LOG_PATH, "Proceso Lissandra File System", logEnConsola, LOG_LEVEL_INFO);

	imprimirVerde(logger,"[LOG CREADO] continuamos cargando la estructura de configuracion.");

	if (cargarConfiguracion()) {
		if (!obtenerMetadata()) {
			//SI SE CARGO BIEN LA CONFIGURACION ENTONCES PROCESO DE ABRIR UN SERVIDOR
			//abrirServidorLissandra();
			crearBloques();
		}
	}

	log_info(logger, "La configuracion y la metadata se levantaron correctamente");


	if(!existeDirectorio(tabla_Path)){
		if(mkdir(tabla_Path,0777)==-1){
			log_info(logger,"NO SE PUDO CREAR LA CARPETA PARA LAS TABLAS %s",tabla_Path);
			exit(-1);
		}
	}

	memtable = dictionary_create();
	dicSemTablas = dictionary_create();
	dicHilosCompactacion = dictionary_create();
	listaTablasInsertadas = list_create();
	listaRegistrosMemtable = list_create();

	rwLockIniciar(&sem_rw_memtable);
	iniciarSemaforosCompactacion();
}

bool existeDirectorio(char *path)
{
	DIR* dir = opendir(path);
	if (dir) {
	    /* Directory exists. */
	    closedir(dir);
	    return true;
	} else if (ENOENT == errno) {
	    /* Directory does not exist. */
	} else {
	    /* opendir() failed for some other reason. */
	}
	return false;
}

bool cargarConfiguracion() {



	configFile = malloc(sizeof(t_lfilesystem_config));

	t_config* archivoCOnfig;

	log_info(logger,"Por crear el archivo de config para levantar archivo con datos.");

	archivoCOnfig = config_create(PATH_LFILESYSTEM_CONFIG);

	if (archivoCOnfig == NULL) {
		imprimirMensajeProceso("NO se ha encontrado el archivo de configuracion\n");
		log_info(logger, "NO se ha encontrado el archivo de configuracion");
	} else {
		int ok = 1;
		imprimirMensajeProceso("Se ha encontrado el archivo de configuracion\n");

		log_info(logger, "LissandraFS: Leyendo Archivo de Configuracion...");

		if (config_has_property(archivoCOnfig, "PUERTO_ESCUCHA")) {


			//Por lo que dice el texto
			configFile->puerto_escucha = config_get_int_value(archivoCOnfig,
					"PUERTO_ESCUCHA");

			log_info(logger, "El puerto de escucha es: %d",configFile->puerto_escucha);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el PUERTO_ESCUCHA");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "PUNTO_MONTAJE")) {


			//Por lo que dice el texto		
			char *aux = config_get_string_value(archivoCOnfig,"PUNTO_MONTAJE");
			configFile->punto_montaje = malloc(strlen(aux)+1);
			strcpy(configFile->punto_montaje,aux);

			log_info(logger, "El punto de montaje es: %s",configFile->punto_montaje);

			int tamanio = strlen(configFile->punto_montaje) + strlen(TABLE_PATH)+ 30;

			tabla_Path = malloc(tamanio);

			snprintf(tabla_Path, tamanio, "%s%s", configFile->punto_montaje,TABLE_PATH);

			//log_info(logger, "Y ahora la variable tabla_path queda con: %s",tabla_Path);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el PUNTO_MONTAJE");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "RETARDO")) {


			//Por lo que dice el texto
			configFile->retardo = config_get_int_value(archivoCOnfig,
					"RETARDO");

			log_info(logger, "El retardo de respuesta es: %d",configFile->retardo);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene RETARDO");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "TAMANIO_VALUE")) {


			//Por lo que dice el texto
			configFile->tamanio_value = config_get_int_value(archivoCOnfig,
					"TAMANIO_VALUE");

			log_info(logger, "El tamanio del valor es: %d",configFile->tamanio_value);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el TAMANIO_VALUE");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "TIEMPO_DUMP")) {


			//Por lo que dice el texto
			configFile->tiempo_dump = config_get_int_value(archivoCOnfig,
					"TIEMPO_DUMP");

			log_info(logger, "El tiempo de dumpeo es: %d",configFile->tiempo_dump);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene el TIEMPO_DUMP");
			ok--;

		}

		if (config_has_property(archivoCOnfig, "IP")) {

			char *aux = config_get_string_value(archivoCOnfig,"IP");
			configFile->ip = malloc(strlen(aux)+1);
			strcpy(configFile->ip,aux);
			//Por lo que dice el texto

			log_info(logger, "La IP es: %s", configFile->ip);

		} else {
			imprimirError(logger,
					"El archivo de configuracion no contiene la IP");
			ok--;

		}

		if (ok > 0) {
			config_destroy(archivoCOnfig);
			return true;

		} else {

			config_destroy(archivoCOnfig);
			imprimirError(logger,"ERROR: No se han cargado los datos del archivo de configuracion\n");
			return false;
		}

	}

	return false;

}

void recargarConfiguracionLFS(char *path_config){

	t_config* auxConfigFile = config_create(path_config);

	if (auxConfigFile != NULL) {

		log_info(logger, "[ACTUALIZANDO RETARDOS] LEYENDO CONFIGURACION...");

		if (config_has_property(auxConfigFile, "RETARDO")) {

			configFile->retardo = config_get_int_value(auxConfigFile,"RETARDO");

			log_info(logger, "[ACTUALIZANDO RETARDOS] RETARDO : %d",configFile->retardo);

		} else {
			log_error(logger, "[ACTUALIZANDO RETARDOS] NO HAY RETARDO CONFIGURADO");
		} // RETARDO

		if (config_has_property(auxConfigFile, "TIEMPO_DUMP")) {

			configFile->tiempo_dump = config_get_int_value(auxConfigFile, "TIEMPO_DUMP");
			log_info(logger, "[ACTUALIZANDO RETARDOS] TIEMPO DUMP: %d",configFile->tiempo_dump);

		} else {
			log_error(logger, "[ACTUALIZANDO RETARDOS] NO HAY TIEMPO DUMP CONFIGURADO");
		} // TIEMPO_DUMP


	} else {
		log_error(logger,
				"[ACTUALIZANDO RETARDOS] NO HAY ARCHIVO DE CONFIGURACION DEL LFS"); // ERROR: SIN ARCHIVO CONFIGURACION
	}

	config_destroy(auxConfigFile);


	log_info(logger, "[ACTUALIZANDO RETARDOS] RETARDOS ACTUALIZADOS CORRECTAMENTE");

}


int existeArchivo(char* path) {
	FILE* reader = fopen(path, "r");
	if (reader == NULL)
		return false;
	fclose(reader);
	return true;
}

void cargarBitmap() {
	log_info(logger, "Voy a cargar bitmap");

	int tamanio = strlen(configFile->punto_montaje)+ strlen(PATH_LFILESYSTEM_BITMAP) + 1;
	bitmapPath = malloc(tamanio);
	snprintf(bitmapPath, tamanio, "%s%s", configFile->punto_montaje,
	PATH_LFILESYSTEM_BITMAP);

	if (!existeArchivo(bitmapPath)) {
		log_info(logger,"Archivo de bitmap no existe, se procede a crear el bitmap");
		crearBitarray();
		abrirBitmap();
	} else {
		log_info(logger, "Existe archivo, se procede a abrir el bitmap");
		abrirBitmap();

	}

	log_info(logger, "cantidad de bloques libres en el bitmap: %d",cantBloquesLibresBitmap());


}

int abrirBitmap() {

	int bitmap = open(bitmapPath, O_RDWR);
	struct stat mystat;

	if (fstat(bitmap, &mystat) < 0) {
		log_error(logger, "Error en el fstat\n");
		close(bitmap);
	}

	char *bmap;
	bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED,
			bitmap, 0);

//	log_info(logger,"inicialicé mmap");

	if (bmap == MAP_FAILED) {
		log_error(logger, "Fallo el mmap");
	}

	int bytesAEscribirAux = metadataLFS->blocks / 8;

	if (metadataLFS->blocks % 8 != 0)
		bytesAEscribirAux++;
	bitarray = bitarray_create_with_mode(bmap, bytesAEscribirAux,
			MSB_FIRST);

	imprimirMensajeProceso("[BITMAP ABIERTO] Ya se puede operar con los bloques");

	//free(fs_path);
	return 0;
}

void crearBitarray() {

	int bytesAEscribirAux = metadataLFS->blocks / 8;

	if (metadataLFS->blocks % 8 != 0)
		bytesAEscribirAux++;


	char* bitarrayAux = malloc(bytesAEscribirAux);
	bzero(bitarrayAux, bytesAEscribirAux);

	archivoBitmap = fopen(bitmapPath, "wb");

	if (archivoBitmap == NULL) {
		imprimirError(logger,"El archivoBitmap no se pudo abrir correctamente");
		exit(-1);
	}

	fwrite(bitarrayAux, bytesAEscribirAux, 1, archivoBitmap);


	//t_bitarray* bitarrayReturn = bitarray_create_with_mode(bitarrayAux, bytesAEscribir, MSB_FIRST);

	fclose(archivoBitmap);
	free(bitarrayAux);

	//return bitarrayReturn;

}

int cantBloquesLibresBitmap() {
	int cantidad = 0;

	for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
		if (bitarray_test_bit(bitarray, i) == 0) {
			cantidad++;
		}
	}
	return cantidad;
}

int estadoBloqueBitmap(int bloque) {

	return bitarray_test_bit(bitarray, bloque);
}

int ocuparBloqueLibreBitmap(int bloque) {

	bitarray_set_bit(bitarray, bloque);


	return 0;
}

int liberarBloqueBitmap(int bloque) {

	bitarray_clean_bit(bitarray, bloque);


	return 0;
}

int obtenerPrimerBloqueLibreBitmap() {

	int posicion = -1;

	for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
		if (bitarray_test_bit(bitarray, i) == 0) {
			posicion = i;
			break;
		}
	}

	return posicion;
}

int obtenerPrimerBloqueOcupadoBitmap() {

	int posicion = -1;
	for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
		if (bitarray_test_bit(bitarray, i) == 1) {
			posicion = i;
			break;
		}
	}

	return posicion;
}

int cantidadBloquesOcupadosBitmap() {

	int cantidad = 0;

	for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
		if (bitarray_test_bit(bitarray, i) == 1) {
			cantidad++;
		}
	}

	return cantidad;
}

void consola() {


	menu();

	char* linea;

	while (1) {

		linea = readline(">");

		if (linea && strcmp(linea, "\n") && strcmp(linea, "")) {
			add_history(linea);

		}

		if (!strncmp(linea, "SALIR", 5)) {
			log_info(logger,"\n\n**************************** SALIENDO ****************************\n\n");
			free(linea);
			break;
		}

		if (linea && strcmp(linea, "\n") && strcmp(linea, "")) { //Así no rompe cuando se apreta enter
			atenderRequest(linea);
		}
		free(linea);
	}

}

void atenderRequest(char* linea) {
	request_t req;
	req = parser(linea);
	resp_com_t respuesta;
	respuesta.msg.tam = 0;
	switch (req.command) {

	case INSERT:

		imprimirMensaje(logger, "[RESOLVIENDO PEDIDO] Voy a resolver INSERT");
		respuesta = resolver_insert(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,
					"[RESOLVIENDO PEDIDO] INSERT hecho correctamente");
			printf("[INSERT] Realizado correctamente\n");
		} else {
			imprimirError(logger,
					"[RESOLVIENDO PEDIDO] El INSERT no pudo realizarse");
			printf("[INSERT] No pudo realizarse\n");
		}

		break;
	case SELECT:

		imprimirMensaje(logger, "[RESOLVIENDO PEDIDO] Voy a resolver SELECT");
		respuesta = resolver_select(req);
		if (respuesta.tipo == RESP_OK && respuesta.msg.tam > 0) {
			imprimirMensaje1(logger,"[RESOLVIENDO PEDIDO] SELECT hecho correctamente. Valor %s obtenido",respuesta.msg.str);
			printf("[SELECT] Valor obtenido: %s\n",respuesta.msg.str);
		} else {
			imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] El SELECT no pudo realizarse");
			printf("[SELECT] El comando no pudo realizarse\n");
		}
		break;
	case DESCRIBE:

		imprimirAviso(logger, "[RESOLVIENDO PEDIDO] Voy a resolver DESCRIBE");
		respuesta = resolver_describe(req);
		if (respuesta.tipo == RESP_OK && respuesta.msg.tam > 0) {
			imprimirMensaje1(logger,"[RESOLVIENDO PEDIDO] DESCRIBE hecho correctamente. Valor %s obtenido",
					respuesta.msg.str);
			//printf("[DESCRIBE] Valor obtenido: %s\n",respuesta.msg.str);
		} else {
			imprimirError(logger,"[RESOLVIENDO PEDIDO] El DESCRIBE no pudo realizarse");
			printf("[DESCRIBE] El comando no pudo realizarse\n");
		}

		break;
	case DROP:

		imprimirMensaje(logger, "[RESOLVIENDO PEDIDO] Voy a resolver DROP");
		respuesta = resolver_drop(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] DROP hecho correctamente");
			printf("[DROP] La tabla %s se borro correctamente\n",req.args[0]);
		} else {
			imprimirError(logger,"[RESOLVIENDO PEDIDO] El DROP no pudo realizarse");
			printf("[DROP] El comando no pudo realizarse\n");
		}

		break;
	case CREATE:

		imprimirMensaje(logger,
				"[RESOLVIENDO PEDIDO] Voy a resolver CREATE\n\n");
		respuesta = resolver_create(req);
		if (respuesta.tipo == RESP_OK) {
			imprimirMensaje(logger,"[RESOLVIENDO PEDIDO] CREATE hecho correctamente");
			printf("[CREATE] La tabla %s se creo correctamente\n",req.args[0]);
		} else {
			if(respuesta.tipo == RESP_ERROR_TABLA_NO_EXISTE){printf("[CREATE] La %s ya existe\n",req.args[0]);}
			else if(respuesta.tipo == RESP_ERROR_METADATA){printf("[CREATE] No se pudo crear el metadata\n");}
			imprimirError(logger,"[RESOLVIENDO PEDIDO] El CREATE no pudo realizarse");

		}
		break;
	case SALIR:
		printf("Se selecciono SALIR\n");
		//cerrarTodo(); terminar

		break;

	default:
		printf("Comando mal ingresado\n");
		break;
	}

	borrar_request(req);
	borrar_respuesta(respuesta);

}

void menu() {

	printf(
			"Los comandos que se pueden ingresar son: \n"
					"COMANDOS [ARGUMENTOS] (* -> opcional) \n"
					"insert [TABLA] [KEY] [VALUE] [TIMESTAMP]* \n"
					"select [TABLA] [KEY]\n"
					"create [TABLA] [TIPO_CONSISTENCIA] [NRO_PARTICION] [TIEMPO_COMPACTACION]\n"
					"describe [TABLA]*\n"
					"drop [TABLA]\n"
					"SALIR \n"
					"\n");

}


char *armarPathTabla(char *nombre_tabla) {

	char *nombre_upper = malloc(strlen(nombre_tabla) + 1);
	strcpy(nombre_upper, nombre_tabla);
	string_to_upper(nombre_upper);
	int tamanio = strlen(tabla_Path) + strlen(nombre_upper) + 1;
	char *path = malloc(tamanio);
	snprintf(path, tamanio, "%s%s", tabla_Path, nombre_upper);
	free(nombre_upper);
	//log_info(logger, "[ARMAR PATH TABLA] Path armado %s", path);
	return path;
}

char *armarPathMetadataTabla(char *nombre_tabla) {
	char *nombre_upper = malloc(strlen(nombre_tabla) + 1);
	strcpy(nombre_upper, nombre_tabla);
	string_to_upper(nombre_upper);
	int tamanio = strlen(tabla_Path) + strlen(nombre_upper)+ strlen("/metadata") + 1;
	char *path = malloc(tamanio);
	snprintf(path, tamanio, "%s%s%s", tabla_Path, nombre_upper, "/metadata");
	//log_info(logger, "[ARMAR PATH METADATA TABLA] Path armado %s", path);
	free(nombre_upper);
	return path;
}

int verificarTabla(char *tabla) {
	char *path = armarPathTabla(tabla);
	FILE *file;
	//log_info(logger,"[VERIFICADO] La direccion de la tabla que se quiere verificar es: %s",path);
	file = fopen(path, "r");
	free(path);

	if (file == NULL) {

		log_error(logger, "[VERIFICAR-TABLA] No existe la tabla");
		return -1;

	} else {

		//log_info(logger, "[VERIFICAR-TABLA] La tabla ya existe.");
		fclose(file);
		return 0;
	}
}


t_metadata_tabla* obtenerMetadataTabla(char* tabla) {

	t_metadata_tabla* metadataTabla;

	char *path_tabla_metadata = armarPathMetadataTabla(tabla);
	//log_info(logger, "[obtenerMetadata] (+) metadata a abrir : %s",	path_tabla_metadata);

	int result = 0;
	metadataTabla = malloc(sizeof(t_metadata_tabla)); // Vatiable global.--->la hago local para soportar varios procesos concurrentes
	t_config* metadataFile;

	metadataFile = config_create(path_tabla_metadata);

	if (metadataFile != NULL) {

		log_info(logger, "LFS: Leyendo metadata...");

		if (config_has_property(metadataFile, "CONSISTENCY")) {


			// Si no hago esto, al destruir la configuración genero un segmentation fault
			char *auxStr = config_get_string_value(metadataFile, "CONSISTENCY");
			metadataTabla->consistency = malloc(strlen(auxStr) + 1);
			strcpy(metadataTabla->consistency, auxStr);

			log_info(logger, "La consistencia  es: %s",
					metadataTabla->consistency);

		} else {

			log_error(logger, "El metadata no contiene la consistencia");

		} // if (config_has_property(metadataFile, "CONSISTENCY"))

		if (config_has_property(metadataFile, "PARTITIONS")) {


			metadataTabla->particiones = config_get_int_value(metadataFile,
					"PARTITIONS");

			log_info(logger, "Las particiones son : %d",
					metadataTabla->particiones);

		} else {

			log_error(logger, "El metadata no contiene particiones");

		} // if (config_has_property(metadataFile, "PARTITIONS"))

		if (config_has_property(metadataFile, "COMPACTION_TIME")) {

			metadataTabla->compaction_time = config_get_int_value(metadataFile,
					"COMPACTION_TIME");

			log_info(logger, "el tiempo de compactacion es: %d",
					metadataTabla->compaction_time);

		} else {

			log_error(logger,
					"El metadata no contiene el tiempo de compactacion");

		} // if (config_has_property(metadataFile, "COMPACTION_TIME"))

	} else {

		log_error(logger,
				"[ERROR] Archivo metadata de particion no encontrado.");

		result = -1;

	} // if (metadataFile != NULL)

	//log_info(logger,"[FREE] variable metadataFile utlizada para navegar el metadata.");

	if (metadataFile != NULL)
		config_destroy(metadataFile);

	free(path_tabla_metadata);

	if (result == -1) {
		free(metadataTabla);
		return NULL;
	}

	return metadataTabla;

}

int obtenerMetadata() {

	log_info(logger, "levantando metadata del File System");

	int result = 0;

	metadataLFS = malloc(sizeof(t_metadata_LFS)); // Vatiable global.

	int tamanio = strlen(configFile->punto_montaje)
			+ strlen(PATH_LFILESYSTEM_METADATA) + 1;
	char* metadataPath = malloc(tamanio);

	snprintf(metadataPath, tamanio, "%s%s", configFile->punto_montaje,
	PATH_LFILESYSTEM_METADATA);


	t_config* metadataFile;
	metadataFile = config_create(metadataPath);

	if (metadataFile != NULL) {

		log_info(logger, "LFS: Leyendo metadata...");

		if (config_has_property(metadataFile, "BLOCK_SIZE")) {


			metadataLFS->block_size = config_get_int_value(metadataFile,
					"BLOCK_SIZE");

			log_info(logger, "el tamanio del bloque es: %d",
					metadataLFS->block_size);

		} else {

			log_error(logger,
					"El metadata no contiene el tamanio de bloque [BLOCK_SIZE]");

		} // if (config_has_property(metadataFile, "CONSISTENCY"))

		if (config_has_property(metadataFile, "BLOCKS")) {


			metadataLFS->blocks = config_get_int_value(metadataFile, "BLOCKS");

			log_info(logger, "La cantidad de bloques es: %d",
					metadataLFS->blocks);

		} else {

			log_error(logger,
					"El metadata no contiene cantidad de bloques [BLOCKS]");

		} // if (config_has_property(metadataFile, "PARTITIONS"))

		if (config_has_property(metadataFile, "MAGIC_NUMBER")) {

			char* aux = config_get_string_value(metadataFile,
					"MAGIC_NUMBER");
			metadataLFS->magic_number = malloc(strlen(aux)+1);
			strcpy(metadataLFS->magic_number,aux);

			log_info(logger, "el magic number es: %s",
					metadataLFS->magic_number);

		} else {

			log_error(logger,
					"El metadata no contiene el magic number [MAGIC_NUMBER]");

		} // if (config_has_property(metadataFile, "COMPACTION_TIME"))

	} else {

		log_error(logger,"[ERROR] Archivo metadata de file system no encontrado.");

		result = -1;

	} // if (metadataFile != NULL)

	//log_info(logger,"[FREE] variable metadataFile utlizada para navegar el metadata.");

	free(metadataPath);
	config_destroy(metadataFile);


	return result;
}

char* retornarValores(char* tabla, t_metadata_tabla* metadata) {

	//printf("\nValores de la %s \n\n", tabla);
	//printf("CONSISTENCY=%s\nPARTITIONS=%d\nCOMPACTION_TIME=%d\n", metadata->consistency,metadata->particiones,metadata->compaction_time);

	char* particiones = malloc(4);
	char* tiempoCompactacion = malloc(7);

	sprintf(particiones, "%d", metadata->particiones);
	sprintf(tiempoCompactacion, "%d", metadata->compaction_time);

	int tamanio = strlen(tabla) + strlen(metadata->consistency)
			+ strlen(particiones) + strlen(tiempoCompactacion) + strlen("|||")
			+ 3;

	char* valorDescribe = malloc(tamanio);

	snprintf(valorDescribe, tamanio, "%s|%s|%s|%s", tabla,
			metadata->consistency, particiones, tiempoCompactacion);

	free(particiones);
	free(tiempoCompactacion);
	return valorDescribe;

}

//todo: @martin revisar esta función
char* retornarValoresDirectorio() {
	DIR *dir;
	struct dirent *ent;
	int tamanio = strlen(configFile->punto_montaje) + strlen(TABLE_PATH) + 1;
	char* pathTabla = malloc(tamanio);
	char* resultado = NULL;
	int memoriaParaMalloc = 0;
	bool encontreAlgoEnDirectorio = false;
	t_list* lista_describes;
	lista_describes = list_create();

	snprintf(pathTabla, tamanio, "%s%s", configFile->punto_montaje, TABLE_PATH);

	dir = opendir(pathTabla);


	if (dir == NULL) {
		log_error(logger, "No puedo abrir el directorio");
		perror("No puedo abrir el directorio");

	}

	while ((ent = readdir(dir)) != NULL) {

		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			log_info(logger, "Tabla analizada= %s", ent->d_name);
			verificarTabla(ent->d_name);
			t_metadata_tabla* metadata = obtenerMetadataTabla(ent->d_name);
			if (metadata != NULL) {
				resultado = retornarValores(ent->d_name, metadata);
				//log_info(logger, "el resultado es %s", resultado);
				memoriaParaMalloc += strlen(resultado) + 1; // el 1 es por el | para separar cada describe
				//log_info(logger, "tamanio malloc es %d", memoriaParaMalloc);
				list_add(lista_describes, resultado);
				encontreAlgoEnDirectorio = true;
				free(metadata->consistency);
				free(metadata);
//				free(resultado); //NO AGREGAR XFA
			}

		}
	}

	//log_info(logger, "[DEBUG] Fin de lectura directorio");
	char* resultadoFinal = NULL;
	if (encontreAlgoEnDirectorio) {
		resultadoFinal = string_new();
		for (int i = 0; i < list_size(lista_describes); i++) {
			char* elemento = list_get(lista_describes, i);
			string_append(&resultadoFinal, elemento);
			if(i < list_size(lista_describes)-1)
				string_append(&resultadoFinal, "|");
		}
		//char *aux = stringTomarDesdeInicio(resultadoFinal,strlen(resultadoFinal) - 1);
		//free(resultadoFinal);
		//resultadoFinal = aux;

	}

	if (resultado != NULL)
		free(resultado);

	free(pathTabla);
	closedir(dir);
	return resultadoFinal;
}

void INThandler(int sig) {


	signal(sig, SIG_IGN);
	printf("\n");
	log_info(logger,"[CATCHING SIGNAL] Cierro sockets clientes antes de salir");
	cerrar_todos_clientes();
	exit(0);

}

//DUMP

void esperarTiempoDump() {

	while (true) {

		usleep(configFile->tiempo_dump * 1000);
		log_info(logger, "Es tiempo de dump, verificando si hay cosas en la memtable");
		mutexBloquear(&listaTablasInsertadas_mx);
		int tam = list_size(listaTablasInsertadas);
		mutexDesbloquear(&listaTablasInsertadas_mx);
		if (tam > 0) {
	        log_info(logger, "Se encontraron cosas en la memtable, se hace el dump");
			realizarDump();
			printf("\n****Se realizó un DUMP****\n>");

			cantidad_de_dumps++;
		} else {
			log_info(logger, "La memtable esta vacia");
		}

	}

}

void realizarDump() {
	rwLockEscribir(&sem_rw_memtable);
	mutexBloquear(&listaTablasInsertadas_mx);
	int tam = list_size(listaTablasInsertadas);
	mutexDesbloquear(&listaTablasInsertadas_mx);
	//mutexBloquear(&memtable_mx); //@martin @sincro lo descomento
	for (int i = 0; i < tam; i++) {
//		mutexBloquear(&listaTablasInsertadas_mx);
		char* tabla = list_get(listaTablasInsertadas, i);
//		mutexDesbloquear(&listaTablasInsertadas_mx);
		indiceTablaParaTamanio = i;
		log_info(logger, "la tabla insertada en la memtable es %s", tabla);
		char* path = armarPathTablaParaDump(tabla, cantidad_de_dumps);
		crearArchivoTemporal(path, tabla);
		free(path); //  @VALGRIND @MARTIN


	}
	log_info(logger, "Se limpia diccionario y la listaTablasInsertadas");
	vaciarMemtable();
	log_info(logger, "Se vacio la memtable");
//	mutexDesbloquear(&memtable_mx); //@martin @sincro lo descomento
//	mutexBloquear(&listaTablasInsertadas_mx);
	list_clean_and_destroy_elements(listaTablasInsertadas,free);//@martin @revisar
	rwLockDesbloquear(&sem_rw_memtable);
	log_info(logger, "Se vacio la lista de tablas insertadas");
//	mutexDesbloquear(&listaTablasInsertadas_mx);
}

char* armarPathTablaParaDump(char* tabla, int dumps) {
	char *pathTabla = armarPathTabla(tabla);
	int tam = strlen(pathTabla) + 10 + strlen(".tmp") + 1;
	char *pathTemp = malloc(tam);
	snprintf(pathTemp, tam, "%s/%d%s", pathTabla, dumps, ".tmp");
	free(pathTabla);
	//log_info(logger, "la ruta es %s", pathTemp);
	return pathTemp;



}


int crearArchivoTemporal(char* path, char* tabla) {

	// path objetivo: /home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Tables/TABLA/cantidad_de_dumps.tmp

	//mutexBloquear(&memtable_mx); @martin @sincro comentado
	t_list* listaRegistrosTabla = dictionary_get(memtable, tabla);
//	t_list *listaRegistrosTabla = list_duplicate(dictionary_get(memtable, tabla)); //@martin @debug
	// mutexDesbloquear(&memtable_mx); @martin @sincro comentado


	t_sems_tabla* semsTabla = dictionary_get(dicSemTablas,tabla);

	rwLockEscribir(&(semsTabla->rwLockTabla));


	t_list* bloquesUsados = list_create();
	int tam_total_registros = tamTotalListaRegistros(listaRegistrosTabla);
	int cantidad_bloques = cuantosBloquesNecesito(tam_total_registros);

	int bloqueAux;
	int *bloqueLista;
	log_info(logger, "[DEBUG] Tengo %d bloques libres y necesito %d",
			cantBloquesLibresBitmap(), cantidad_bloques);
	if (cantBloquesLibresBitmap() >= cantidad_bloques) {
		for (int i = 0; i < cantidad_bloques; i++) {
			bloqueAux = obtenerPrimerBloqueLibreBitmap();
			bloqueLista = malloc(sizeof(int));
			*bloqueLista = bloqueAux;
			if (bloqueAux != -1) {
				ocuparBloqueLibreBitmap(bloqueAux);
				//list_add(bloquesUsadcrearArchivoTemporalos, (void*)bloqueAux);
				list_add(bloquesUsados, bloqueLista);
			} else {
				//liberar los bloques de la lista
				return -1;
			}
		}
	} else {
		log_error(logger,
				"[DUMP] no hay bloques disponibles para hacer el dump");
	}

	//void* bufferRegistros = malloc(tam_total_registros);
	void* bufferRegistros = armarBufferConRegistros(listaRegistrosTabla,tam_total_registros);
	int resultadoEscritura = escribirVariosBloques(bloquesUsados,tam_total_registros, bufferRegistros);

	if (resultadoEscritura != -1) {
		FILE* temporal;
		temporal = fopen(path, "w");
		log_info(logger, "[DUMP] Creamos el archivo: %s", path);

		if (temporal != NULL) {
			char *contenido;
			/*
			 * -MARTIN: SI SE USAN LAS FUNCIONES DE LAS COMMONS PARA STRINGS NO HACE FALTA HACER UN MALLOC,
			 * YA LO HACE EL STRING NEW, Y VA REALOCANDO EL STRING SEGUN NECESITA
			 */
			/*char* contenido = malloc(
			 string_length("SIZE=") + sizeof(char) * 2
			 + string_length("BLOCKS=[]")
			 + sizeof(char) * 2 * list_size(bloquesUsados) - 1);*/ //arreglar para bloques con mas de un numero (string, no char)
			contenido = string_new();
			string_append(&contenido, "SIZE=");
			char* size = malloc(10);
			sprintf(size, "%d", tam_total_registros);
			string_append(&contenido, size);
			string_append(&contenido, "\n");
			string_append(&contenido, "BLOCKS=[");
			char* bloque = malloc(10);
			for (int i = 0; i < list_size(bloquesUsados); i++) {
				int* nroBloque = list_get(bloquesUsados, i);
				sprintf(bloque, "%d", *nroBloque);
				string_append(&contenido, bloque);
				if(i != list_size(bloquesUsados)-1) //@martin
					string_append(&contenido, ",");
			}
//			contenido = stringTomarDesdeInicio(contenido,strlen(contenido) - 1);
			string_append(&contenido, "]");
			fputs(contenido, temporal);
			log_info(logger, "[DUMP] Temporal completado con: %s", contenido);
			free(contenido);
			free(bloque); //@martin
			free(size);//@martin
			fclose(temporal);
		} else {
			//liberar bloques de la lista de bloquesUsados
			log_error(logger,
					"[DUMP] Error al abrir el archivo temporal con path: %s",
					path);
		}
	} else {
		log_error(logger,
				"[DUMP] Hubo un error al escribir los datos en los bloques");
	}
	free(bufferRegistros);

	list_destroy_and_destroy_elements(bloquesUsados, free);

	rwLockDesbloquear(&(semsTabla->rwLockTabla));

	return 0;
}

//OPERACIONES CON BLOQUES

//<value>;<key>;<timestamp>\n
int tamTotalListaRegistros(t_list* listaRegistros) {
	int cantidad_registros = list_size(listaRegistros);
	log_info(logger, "Cantidad de registros de la lista: %d",cantidad_registros);

	int tam_total_registros = 0;
	t_registroMemtable* registro;

	for (int i = 0; i < cantidad_registros; i++) {
		registro = list_get(listaRegistros, i);

		//log_info(logger, "[DEBUG] tam registro (%d) = %d", i,registro->tam_registro);

		tam_total_registros += registro->tam_registro - 1; // El -1 porque no estoy escribiendo el \0 al archivo, si no al leer le sobran bytes
	}

	tam_total_registros += sizeof(char) * 3 * cantidad_registros;

	log_info(logger, "Tamanio total de registros: %d", tam_total_registros);

	return tam_total_registros;
}

int cuantosBloquesNecesito(int tam_total) {
	log_info(logger, "[CUANTOS BLOQUES NECESITO] Entra tamaño total %d",
			tam_total);
	log_info(logger,
			"[CUANTOS BLOQUES NECESITO] El tamaño de los bloques es %d",
			metadataLFS->block_size);
	if (tam_total % metadataLFS->block_size == 0) {
		return tam_total / metadataLFS->block_size;
	}

	return tam_total / metadataLFS->block_size + 1;
}

void* armarBufferConRegistros(t_list* listaRegistros, int tam_total_registros) {

	int offset = 0;
	char punto_y_coma = ';';
	char barra_n = '\n';

	int cantidad_registros = list_size(listaRegistros);
	t_registroMemtable* registro;

	void* bufferConRegistros = malloc(tam_total_registros+20);//@martin @nacho probando

	for (int i = 0; i < cantidad_registros; i++) {

		registro = list_get(listaRegistros, i);
		memcpy(bufferConRegistros + offset, &registro->timestamp,sizeof(u_int64_t));
		offset += sizeof(u_int64_t);
		memcpy(bufferConRegistros + offset, &punto_y_coma, sizeof(char));
		offset += sizeof(char);
		memcpy(bufferConRegistros + offset, &registro->key, sizeof(u_int16_t));
		offset += sizeof(u_int16_t);
		memcpy(bufferConRegistros + offset, &punto_y_coma, sizeof(char));
		offset += sizeof(char);
		memcpy(bufferConRegistros + offset, registro->value,strlen(registro->value));
		offset += strlen(registro->value);
		memcpy(bufferConRegistros + offset, &barra_n, sizeof(char));
		offset += sizeof(char);
	}
	//log_info(logger,"[DEBUG555] Tamanio total escrito %d. Tamanio total del buffer %d",offset,tam_total_registros);
	return bufferConRegistros;
}

int escribirVariosBloques(t_list* bloques, int tam_total_registros,
		void* buffer) {

	int resultado = 1;
	int offset = 0;

	for (int i = 0; i < list_size(bloques); i++) {

		int* auxnroBloque = list_get(bloques, i);
		int nroBloque = *auxnroBloque;
		log_info(logger, "bloque seleccionado para guardar el tmp %d",
				nroBloque);
		if (tam_total_registros <= metadataLFS->block_size) {
			//log_info(logger, "tam registros menor a block size");
			resultado = escribirBloque(nroBloque, tam_total_registros, offset,
					buffer);
			offset += tam_total_registros;
			tam_total_registros -= tam_total_registros;
		} else {
			//log_info(logger, "tam registros mayor a block size");
			resultado = escribirBloque(nroBloque, metadataLFS->block_size,
					offset, buffer);
			tam_total_registros -= metadataLFS->block_size;
			offset += metadataLFS->block_size;
		}

		if (resultado == -1) {
			log_info(logger, "resultado -1");
			//liberar bloques en el bitmap
			break;
		}
	}

	log_info(logger, "[BLOQUE] Se terminaron de escribir los bloques");

	//@VALGRIND
	//t_list* registrosLeidos = leerBloquesConsecutivos(bloques, tam_total);
	//list_destroy_and_destroy_elements(registrosLeidos,(void*)borrarRegistro);
	return resultado;
}

int escribirBloque(int bloque, int size, int offset, void* buffer) {

	char* path = crearPathBloque(bloque);
//	log_info(logger, "path de bloque a escribir %s", path);
	FILE* bloqueFile = fopen(path, "wb");

	if (bloqueFile != NULL) {


		fwrite(buffer + offset, size, 1, bloqueFile);
		fclose(bloqueFile);
		free(path);
		return 0;
	}

//	log_info(logger, "[BLOQUE] bloque %d escrito con exito", bloque);
	free(path);
	return -1;
}

void crearBloques() {

	int tamanio = strlen(configFile->punto_montaje) + strlen(PATH_BLOQUES) + 30;
	char *pathBloqueDir = malloc(tamanio);
	snprintf(pathBloqueDir, tamanio, "%s%s",configFile->punto_montaje, PATH_BLOQUES);
	if(!existeDirectorio(pathBloqueDir)){
		if(mkdir(pathBloqueDir,0777)==-1){
			log_error(logger,"ERROR AL CREAR EL DIRECTORIO DE BLOQUES %s",pathBloqueDir);
			free(pathBloqueDir);
			exit(-1);
		}
	}
	free(pathBloqueDir);
	char* pathBloque = malloc(tamanio);
	log_info(logger, "Voy a crear los bloques");
	for (int i = 0; i < metadataLFS->blocks; i++) {
		snprintf(pathBloque, tamanio, "%s%sblock%d.bin",
				configFile->punto_montaje, PATH_BLOQUES, i);
		FILE* bloque;
		bloque = fopen(pathBloque, "a");
		fclose(bloque);

	}
	log_info(logger, "los bloques fueron creados");
	free(pathBloque);
}

char* crearPathBloque(int bloque) {

	int tamanio = strlen(configFile->punto_montaje) + strlen(PATH_BLOQUES) + 30;
	char* pathBloque = malloc(tamanio);
	snprintf(pathBloque, tamanio, "%s%sblock%d.bin", configFile->punto_montaje,
	PATH_BLOQUES, bloque);

	return pathBloque;
}

t_list* leerBloque(char* path) {
	t_list *registros_leidos = list_create();
	FILE* bloque;
	int tam_bloque;
	bloque = fopen(path, "rb");
	t_registroMemtable* registro;

	fseek(bloque, 0, SEEK_END);
	tam_bloque = ftell(bloque);

	log_info(logger, "[BLOQUE] Tamanio del bloque: %d", tam_bloque);

	rewind(bloque);

	void* registros_bloque = malloc(tam_bloque);

	if (fread(registros_bloque, tam_bloque, 1, bloque)) {

		int offset = 0;
		char *aux = malloc(configFile->tamanio_value + 1);
		while (offset < tam_bloque) {
			registro = malloc(sizeof(t_registroMemtable));

			//Guardo timestamp
			memcpy(&registro->timestamp, registros_bloque + offset,
					sizeof(u_int64_t));
			offset += sizeof(u_int64_t);
			offset += sizeof(char); // ";"

			//Guardo key
			memcpy(&registro->key, registros_bloque + offset, sizeof(uint16_t));
			offset += sizeof(uint16_t);
			offset += sizeof(char); // ";"

			//Guardo en el aux el máximo tamaño del value
			if (configFile->tamanio_value + 1 <= tam_bloque - offset)
				memcpy(aux, registros_bloque + offset,
						configFile->tamanio_value + 1);
			else
				memcpy(aux, registros_bloque + offset, tam_bloque - offset);

			//Busco el \n que indica el fin del valor
			char **aux_split = string_split(aux, "\n");
			registro->value = malloc(strlen(aux_split[0]) + 1);
			strcpy(registro->value, aux_split[0]);

			//Libero toda la memoria que genera el string_split
			int i = 0;
			while (aux_split[i] != NULL) {
				free(aux_split[i]);
				i++;
			}
			free(aux_split);

			//Avanzo el offset solo el tamaño realmente leído del valor
			offset += strlen(registro->value) + sizeof(char); //value + \n

			//Calculo tamaño
			registro->tam_registro = strlen(registro->value) + 1
					+ sizeof(u_int64_t) + sizeof(uint16_t);

			log_info(logger, "timestamp leido: %llu", registro->timestamp);
			log_info(logger, "key leida: %d", registro->key);
			log_info(logger, "value leido: %s", registro->value);
			log_info(logger, "tamaño registro: %d", registro->tam_registro);

			//Agrego el registro a la lista que voy a retornar
			list_add(registros_leidos, registro);

			//log_info(logger, "Hasta ahora lei %d bytes de %d", offset,tam_bloque);
		}
		free(aux);
		fclose(bloque);
	} else {
		log_error(logger, "[BLOQUE] No se pudo leer el archivo con el path: %s",
				path);
	}

	return registros_leidos;
}

int abrirArchivoBloque(FILE **fp, int nroBloque, char *modo) {
	char *pathBloque = crearPathBloque(nroBloque);
	*fp = fopen(pathBloque, modo);
	if (*fp == NULL) {
		log_warning(logger, "[abrirArchivoBloque] Error al abrir el archivo %s",
				pathBloque);
		return -1;
	}
	int tam_bloque;
	fseek(*fp, 0, SEEK_END);
	tam_bloque = ftell(*fp);
	rewind(*fp);
//	log_info(logger, "[abrirArchivoBloque] Archivo %s abierto correctamente",pathBloque);
	free(pathBloque);
	return tam_bloque;
}

t_list* leerBloquesConsecutivos(t_list *nroBloques, int tam_total) {

	t_list *registros_leidos = list_create(); //@VALGRIND VER DONDE LIBERAR ESTO
	FILE *bloque;

	if(tam_total == 0){
		return registros_leidos;
	}

	char *aux_value = malloc(configFile->tamanio_value + 1);

	estadoLecturaBloque_t estado = EST_LEER;
	estadoLecturaBloque_t anterior = EST_TIMESTAMP;

	void *aux_campo_leyendo;
	if (configFile->tamanio_value + 1 > sizeof(u_int64_t))
		aux_campo_leyendo = malloc(configFile->tamanio_value + 1);
	else
		aux_campo_leyendo = malloc(sizeof(u_int64_t));
	int offset_bloque = 0, tam_bloque = 0, offset_campo = 0, offset_total = 0,
			bloques_leidos = 0, num_separador = 0;
	bool leiValueEntero;
	t_registroMemtable* registro = malloc(sizeof(t_registroMemtable));
	void* registros_bloque = NULL;

	log_info(logger,
			"[OBTENIENDO TODO BLOQUES] Voy a obtener todos los registros de los bloques indicados");

	while (offset_total < tam_total && estado != EST_FIN) {
//		log_info(logger, "Hasta ahora leí %d bytes de %d", offset_total,
//				tam_total);me
		switch (estado) {
		case EST_LEER:
			if (bloques_leidos == list_size(nroBloques)) {
				estado = EST_FIN;
				break;
			}
			offset_bloque = 0;
			int *nBloque = list_get(nroBloques, bloques_leidos);
//			log_info(logger,"[OBTENIENDO TODO BLOQUES] Leyendo el bloque nro %d, que es el bloque %d",bloques_leidos, *nBloque);
			tam_bloque = abrirArchivoBloque(&bloque, *nBloque, "rb");
//			log_info(logger,"[OBTENIENDO TODO BLOQUES] El tamaño del bloque es: %d",tam_bloque);
			if (registros_bloque != NULL)
				free(registros_bloque);
			registros_bloque = malloc(tam_bloque);
			if (fread(registros_bloque, tam_bloque, 1, bloque) == 0) {
				log_info(logger,"[OBTENIENDO TODO BLOQUES] Error al leer el bloque %d",	*nBloque);
				return NULL;
			}
			fclose(bloque);
//			log_info(logger,"[OBTENIENDO TODO BLOQUES] Bloque leído correctamente");
			bloques_leidos++;
			estado = anterior;
			break;

		case EST_TIMESTAMP:

			//Si con los bytes que le quedan al bloque me alcanza para completar el campo, los copio y avanzo al siguiente estado
			if (offset_bloque + sizeof(u_int64_t) - offset_campo
					<= tam_bloque) {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						sizeof(u_int64_t) - offset_campo);
				memcpy(&(registro->timestamp), aux_campo_leyendo,
						sizeof(u_int64_t));


				//Avanzo los offset los bytes que acabo de leer
				offset_bloque += sizeof(u_int64_t) - offset_campo;
				offset_total += sizeof(u_int64_t) - offset_campo;

				//Avanzo al siguiente estado que es buscar un separador
				anterior = estado;
				estado = EST_SEP;
			}
			//Si no me alcanza, copio todo lo que puedo y voy a leer un nuevo bloque
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque);

				//Avanzo los offset los bytes que acabo de leer
				offset_campo += tam_bloque - offset_bloque;
				offset_total += tam_bloque - offset_bloque;
				offset_bloque += tam_bloque - offset_bloque;


				//Voy a leer un nuevo bloque
				anterior = estado;
				estado = EST_LEER;
			}
			break;

		case EST_KEY:


			//Si con los bytes que le quedan al bloque me alcanza para completar el campo, los copio y avanzo al siguiente estado
			if (offset_bloque + sizeof(uint16_t) - offset_campo <= tam_bloque) {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						sizeof(uint16_t) - offset_campo);
				memcpy(&(registro->key), aux_campo_leyendo, sizeof(uint16_t));


				//Avanzo los offset los bytes que acabo de leer
				offset_bloque += sizeof(uint16_t) - offset_campo;
				offset_total += sizeof(uint16_t) - offset_campo;

				//Avanzo al siguiente estado que es buscar un separador
				anterior = estado;
				estado = EST_SEP;
			}
			//Si no me alcanza, copio todo lo que puedo y voy a leer un nuevo bloque
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque); //@martin @valgrind invalid read of size

				//Avanzo los offset los bytes que acabo de leer
				offset_campo += tam_bloque - offset_bloque;
				offset_total += tam_bloque - offset_bloque;
				offset_bloque += tam_bloque - offset_bloque;


				//Voy a leer un nuevo bloque
				anterior = estado;
				estado = EST_LEER;
			}
			break;

		case EST_VALUE:

			leiValueEntero = false;

			//Si con los bytes que le quedan al bloque me alcanza para completar el tamaño máximo para un value, los copio y avanzo al siguiente estado
			if (offset_bloque + configFile->tamanio_value - offset_campo
					<= tam_bloque) {
				//log_info(logger, "Voy a copiar a aux %d bytes",configFile->tamanio_value - offset_campo);
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						configFile->tamanio_value - offset_campo);
				memcpy(aux_value, aux_campo_leyendo, configFile->tamanio_value);

				//Como al escribir no se escribe el caracter nulo, al leer lo agrego
				aux_value[configFile->tamanio_value] = '\0';
				leiValueEntero = true;
			}
			//Si no me alcanza, leo todo lo que tenga y me fijo si el value está completo (puede tener menos bytes que el máximo)
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque);
				memcpy(aux_value, aux_campo_leyendo,
						tam_bloque - offset_bloque);

				//Como al escribir no se escribe el caracter nulo, al leer lo agrego
				aux_value[offset_campo + tam_bloque - offset_bloque] = '\0';

				//Si encuentro el \n, encontré el valor entero y no tengo que leer otro bloque para tener este campo
				if (string_contains(aux_value, "\n"))
					leiValueEntero = true;
				//Si no lo encuentro, sí tengo que leer otro bloque
				else {
					//Avanzo los offsets los bytes que acabo de leer
					offset_campo += tam_bloque - offset_bloque;
					offset_total += tam_bloque - offset_bloque;
					offset_bloque += tam_bloque - offset_bloque;

					//Voy a leer un nuevo bloque
					anterior = estado;
					estado = EST_LEER;
//					log_info(logger,
//							"No encontré el '\\n' y me faltan %d bytes para el máximo del value",
//							configFile->tamanio_value - offset_campo);
				}
			}
			//Si de cualquiera de las 2 formas pude completar el value, lo guardo y avanzo
			if (leiValueEntero) {
				//Corto el string en el \n y lo guardo en el campo value del registro
				char **aux_split = string_split(aux_value, "\n");
				registro->value = malloc(strlen(aux_split[0]) + 1);
				strcpy(registro->value, aux_split[0]);

				//Borro toda la memoria que aloca la función string_split
				int i = 0;
				while (aux_split[i] != NULL) {
					free(aux_split[i]);
					i++;
				}
				free(aux_split);

				//Avanzo los offsets los bytes que acabo de leer
				offset_bloque += strlen(registro->value) - offset_campo;
				offset_total += strlen(registro->value) - offset_campo;

				//Calculo el tamaño
				registro->tam_registro = sizeof(u_int64_t) + sizeof(uint16_t)
						+ strlen(registro->value) + 1;

//				log_info(logger, "[DEBUG] Tamaño de registro %d",	registro->tam_registro);

				//Agrego el nuevo registro a la lista que voy a retornar
				list_add(registros_leidos, registro);
//				log_info(logger,"[OBTENIENDO TODO BLOQUES] Registro <%llu;%d;%s> agregado",	registro->timestamp, registro->key, registro->value);

				//Avanzo al siguiente estado que es buscar un separador
				anterior = estado;
				estado = EST_SEP;
			}
			break;

		case EST_SEP:
			//log_info(logger, "Buscando un separador");
			//Si no tengo bytes para leer, voy a leer otro bloque
			if (offset_bloque == tam_bloque) {
				anterior = estado;
				estado = EST_LEER;
			}
			//Si tengo un byte, leo y avanzo
			else {
				//No me guardo los separadores porque no los necesito, simplemente avanzo los offsets
				offset_bloque += sizeof(char);
				offset_total += sizeof(char);

				//Voy al siguiente estado, para lo que necesito saber que número de separador estoy leyendo
				anterior = estado;
				switch (num_separador) {
				case 0:
					estado = EST_KEY;
					offset_campo = 0;
					num_separador++;
					break;
				case 1:
					estado = EST_VALUE;
					offset_campo = 0;
					num_separador++;
					break;
				case 2:
					estado = EST_TIMESTAMP;
					offset_campo = 0;
					num_separador = 0;

					//Si voy a leer un timestamp es porque es nuevo registro, por lo que tengo que alocar memoria para guardarlo
					if (offset_total < tam_total)
						registro = malloc(sizeof(t_registroMemtable));
					break;
				}
				//log_info(logger, "Separador leído");
			}
			break;

		}
	}
	log_info(logger, "[OBTENIENDO TODO BLOQUES] Leí todos los bloques");

	//Libero la memoria auxiliar que usé
	free(registros_bloque);
	free(aux_value);
	free(aux_campo_leyendo);

	return registros_leidos;
}

void *imprimirRegistro(t_registroMemtable *reg) {
	log_info(logger, "Registro: <%llu;%d;%s>", reg->timestamp, reg->key,reg->value);
	return reg;
}

void borrarRegistro(t_registroMemtable *reg) {
	if (reg->value != NULL)
		free(reg->value);
	free(reg);
}

t_registroMemtable *crearCopiaRegistro(t_registroMemtable *origen) {
	t_registroMemtable *copia = malloc(sizeof(t_registroMemtable));
	if (origen->value != NULL) {
		copia->value = malloc(strlen(origen->value) + 1);
		strcpy(copia->value, origen->value);
	} else {
		copia->value = NULL;
	}
	copia->key = origen->key;
	copia->tam_registro = origen->tam_registro;
	copia->timestamp = origen->timestamp;
	return copia;
}


//OPERACIONES CON BLOQUES

//DUMP
int determinarParticion(int key, int particiones) {

	int retornar = key % particiones;

	log_info(logger, "La key %d debe almacenarse en la particion %d",key,retornar);

	return retornar;

}

char* rutaParticion(char* tabla, int particion) {
	char *path = armarPathTabla(tabla);
	char * stringParticion = malloc(sizeof(char) * 3);

	sprintf(stringParticion, "%d", particion);

	int tamanio = string_length(path) + string_length(stringParticion)
			+ string_length(PATH_BIN) + 2;

	char* archivoParticion = malloc(tamanio);

	snprintf(archivoParticion, tamanio, "%s%s%d%s", path, "/", particion,
	PATH_BIN);
	log_info(logger, "La ruta de la particion es: %s", archivoParticion);

	free(path);
	free(stringParticion);
	return archivoParticion;
}

void eliminarTablaCompleta(char* tabla) {

	t_metadata_tabla *metadata = obtenerMetadataTabla(tabla);

	if (metadata != NULL) {

		for (int i = 0; i < metadata->particiones; i++) {

			char* archivoParticion = rutaParticion(tabla, i);

			log_info(logger,
					"Vamos a eliminar el archivo binario  de la tabla: %s",
					archivoParticion);

			int retParticion = remove(archivoParticion);

			if (retParticion == 0) { // Eliminamos el archivo
				log_info(logger,
						"El archivo fue eliminado satisfactoriamente\n");
			} else {
				log_info(logger, "No se pudo eliminar el archivo\n");

			}
			free(archivoParticion); //@martin
		}
//		free(metadata->consistency);
		free(metadata);

	}

	char *path = armarPathTabla(tabla);
	t_list *lista_tmp = obtenerArchivosDirectorio(path, ".tmp");
	for (int i = 0; i < list_size(lista_tmp); i++) {
		char *aux = list_get(lista_tmp, i);
		log_info(logger, "[BORRANDO] Borrando archivo %s", aux);
		if (remove(aux) == 0) {
			log_info(logger, "[BORRANDO] El archivo fue borrado correctamente");
		} else {
			log_info(logger, "[BORRANDO] El archivo no pudo borrarse");
		}
	}
	list_destroy_and_destroy_elements(lista_tmp, free);

	t_list *lista_tmpc = obtenerArchivosDirectorio(path, ".tmpc");
	for (int i = 0; i < list_size(lista_tmpc); i++) {
		char *aux = list_get(lista_tmpc, i);
		log_info(logger, "[BORRANDO] Borrando archivo %s", aux);
		if (remove(aux) == 0) {
			log_info(logger, "[BORRANDO] El archivo fue borrado correctamente");
		} else {
			log_info(logger, "[BORRANDO] El archivo no pudo borrarse");
		}
	}
	list_destroy_and_destroy_elements(lista_tmpc, free);


	char *path_tabla_metadata = armarPathMetadataTabla(tabla);
	log_info(logger, "Vamos a eliminar el metadata de la tabla: %s",
			path_tabla_metadata);

	int retMet = remove(path_tabla_metadata);

	//log_info(logger, "Resultado de remove del metadata de la tabla %d", retMet);

	if (retMet == 0) { // Eliminamos el archivo
		log_info(logger, "El archivo fue eliminado satisfactoriamente\n");
	} else {
		log_info(logger, "No se pudo eliminar el archivo\n");
	}

	log_info(logger, "Vamos a eliminar el directorio: %s", path);

	int retTab = remove(path);

	//log_info(logger, "Resultado de remove de la tabla %d", retTab);

	if (retTab == 0) { // Eliminamos el archivo
		log_info(logger, "El archivo fue eliminado satisfactoriamente\n");
	} else {
		log_info(logger, "No se pudo eliminar el archivo\n");
	}

	free(path);
	free(path_tabla_metadata);

}

bool validarValue(char* value) {

	bool contienePuntoYcoma = stringContiene(value, ";");

	if (contienePuntoYcoma) {

		log_info(logger, "el value contiene ; por lo tanto no se agrega");

	}

	return contienePuntoYcoma;

}

bool validarKey(char* key) {

	bool contienePuntoYcoma = stringContiene(key, ";");

	if (contienePuntoYcoma) {

		log_info(logger, "la key contiene ; por lo tanto no se agrega");

	}

	return contienePuntoYcoma;

}

char* desenmascararValue(char* value) {

	char* valueDesenmascarado = malloc(strlen(value)-1);

	for(int i =1;i<strlen(value)-1;i++){
		valueDesenmascarado[i-1] = value[i];

	}
	valueDesenmascarado[strlen(value)-2] = '\0';
	//char* valueDesenmascarado = stringTomarCantidad(value,1,strlen(value)-2);

//	log_info(logger, "el value desenmascarado es %s", valueDesenmascarado);

	return valueDesenmascarado;

}

t_registroMemtable* armarEstructura(char* value, char* key, char* timestamp) {

	t_registroMemtable* registroMemtable;
	registroMemtable = malloc(sizeof(t_registroMemtable));

	int tam_registro = strlen(value) + 1 + sizeof(u_int16_t) + sizeof(u_int64_t); //es un long no un u_int64_t
	registroMemtable->tam_registro = tam_registro;
	registroMemtable->value = malloc(strlen(value) + 1);
	strcpy(registroMemtable->value, value);
	u_int64_t timestampRegistro = strtoull(timestamp, NULL, 10);
	registroMemtable->timestamp = timestampRegistro;
	u_int16_t keyRegistro = strtol(key, NULL, 10);
	registroMemtable->key = keyRegistro;

	//log_info(logger, "Tamaño de registro agregado = %d",registroMemtable->tam_registro);
	//log_info(logger,"El registro quedo conformado por: \n");
	//log_info(logger,"Value = %s ",registroMemtable->value);
	//log_info(logger, "Timestamp = %llu ", registroMemtable->timestamp);
	//log_info(logger,"Key = %x ",registroMemtable->key);
	//log_info(logger,"Se procede a agregar el registro a la memtable");

	return registroMemtable;

}

t_registroMemtable* armarRegistroNulo() {
	t_registroMemtable* aux = malloc(sizeof(t_registroMemtable));
	aux->key = 0;
	aux->tam_registro = 0;
	aux->timestamp = 0;
	aux->value = NULL;

	return aux;
}

t_bloquesUsados* leerTemporaloParticion(char* path) {

	t_config* tempFile;
	tempFile = config_create(path);
	t_bloquesUsados* retVal;

	if (tempFile == NULL) {
		retVal = malloc(sizeof(t_bloquesUsados));
		log_info(logger, "No se ha encontrado el archivo de configuracion");
		retVal->bloques = NULL;
		retVal->size = 0;
	}

	else {
		retVal = malloc(sizeof(t_bloquesUsados));
		log_info(logger, "Se ha encontrado el archivo de configuracion");

		int size = config_get_int_value(tempFile, "SIZE");

		//log_info(logger, "Size encontrado del temp es: %d", size);

		char** bloques = config_get_array_value(tempFile, "BLOCKS");

		char* aux = bloques[0];
		int i = 0;
		t_list* NroBloques = list_create();
		int* auxNroBloque;

		//log_info(logger, "llego al while");


		while (aux != NULL) {

			auxNroBloque = malloc(sizeof(int));
			*auxNroBloque = atoi(aux);
			list_add(NroBloques, auxNroBloque);
			free(aux);
			i++;
			aux = bloques[i];
		}

		//log_info(logger, "salgo de	l while");

		retVal->bloques = NroBloques;
		retVal->size = size;

		free(bloques);
//		free(path);//@nacho agregado
		config_destroy(tempFile);  //@nacho agregado


	}
	return retVal;
}

t_registroMemtable* obtenerRegistroMayor(char* tabla, int key,	t_list* listaSegunLugar) {

	t_registroMemtable* registro = NULL;
	t_registroMemtable* retval = malloc(sizeof(t_registroMemtable));
	for (int i = 0; i < list_size(listaSegunLugar); i++) {
		if (registro == NULL) {

			registro = list_get(listaSegunLugar, i);
			//log_info(logger, "key del registro encontrado %d", registro->key);
			if (registro->key != key) {
				//log_info(logger,"Como es distinta, el registro vuelve a ser NULL");
				registro = NULL;
			} else {
				log_info(logger, "[Primer registro obtenido] <%llu;%d;%s>", registro->timestamp, registro->key,registro->value);

				//imprimirRegistro(registro);
			}

		} else {
			t_registroMemtable* aux = list_get(listaSegunLugar, i);
			if (aux->key == key) {
				log_info(logger, "[Ya habia un registro] <%llu;%d;%s>", registro->timestamp, registro->key,registro->value);
				//imprimirRegistro(aux);

				if (aux->timestamp > registro->timestamp) {
					registro = aux;
					log_info(logger, "[Registro mas reciente] <%llu;%d;%s>", registro->timestamp, registro->key,registro->value);
					//log_info(logger,"[FIN]");
					//imprimirRegistro(registro);

				}
			}
		}
	}
	if (registro != NULL) {
		retval->value = malloc(strlen(registro->value) + 1);
		retval->key = registro->key;
		retval->tam_registro = registro->tam_registro;
		retval->timestamp = registro->timestamp;
		strcpy(retval->value, registro->value);
	} else {
		free(retval);
		retval = armarRegistroNulo();
	}

	return retval;
}

t_registroMemtable* registroMayorMemtable(char* tabla, u_int16_t key) {

	t_registroMemtable* registro;
	//pthread_mutex_lock(&mutex_dump);

	if (dictionary_has_key(memtable, tabla)) {
		t_list* tableRegisters = dictionary_get(memtable, tabla);
		//t_list* aux = list_duplicate(tableRegisters);
		//registro = obtenerRegistroMayor(tabla, key, aux);
		registro = obtenerRegistroMayor(tabla, key, tableRegisters);//@martin @nacho


	} else {

		registro = armarRegistroNulo();

	}
	//pthread_mutex_unlock(&mutex_dump);
	return registro;
}

t_registroMemtable* registroMayorTemporal(char* tabla, u_int16_t key,
		char* terminacion) { //Terminacion va .tmp o .tmpc

	char *path = armarPathTabla(tabla);
	t_list* temporalesTabla = obtenerArchivosDirectorio(path, terminacion);

	free(path);

	t_registroMemtable* registro;
	t_registroMemtable* aux;
	registro = NULL;

	log_info(logger,
			"Entro a función de búsqueda de registro con mayor timestamp en temporal");
	log_info(logger, "Busco la key %d en la tabla %s", key, tabla);

	int tamanioLista = list_size(temporalesTabla);
	log_info(logger, "Tengo %d temporales", tamanioLista);
	if (tamanioLista > 0) {
		for (int i = 0; i < tamanioLista; i++) {
//			char* pathTemp = armarPathTablaParaDump(tabla, i);
			char *pathTemp = list_get(temporalesTabla, i);
			log_info(logger, "Buscando en el temporal %d que es %s", i,
					pathTemp);

			t_bloquesUsados* lecturaTMP = leerTemporaloParticion(pathTemp);
			if (lecturaTMP->bloques == NULL) { // La idea es que nunca entre aca, si tengo temporal es xq un bloque tengo que tener
				registro = armarRegistroNulo();
			} else {
				if (registro == NULL) {
					aux = leerBloquesConsecutivosUnaKey(lecturaTMP->bloques,lecturaTMP->size, key, false);
					if(aux == NULL)
						aux = armarRegistroNulo();
					if (aux->value != NULL) {
						//registro = aux;
						registro = crearCopiaRegistro(aux);	//@nacho agregado
						log_info(logger, "Registro encontrado: <%llu;%d;%s>",registro->timestamp, registro->key,registro->value);
						borrarRegistro(aux); //@nacho agregado
					}
				} else {
					aux = leerBloquesConsecutivosUnaKey(lecturaTMP->bloques,
							lecturaTMP->size, key, false);
					if(aux == NULL)
						aux = armarRegistroNulo();
					if (aux->value != NULL) {
						log_info(logger,
								"Nuevo registro encontrado: <%llu;%d;%s>",
								aux->timestamp, aux->key, aux->value);
						if (aux->timestamp > registro->timestamp) {
							borrarRegistro(registro);
							registro = crearCopiaRegistro(aux); //@nacho agregado
							//borrarRegistro(aux);
							log_info(logger,
									"Pisa al anterior por tener timestamp más grande");
						}
					}
					borrarRegistro(aux);
				}

				list_destroy_and_destroy_elements(lecturaTMP->bloques,free);
			}
			free(lecturaTMP);
		}
	} else { // La idea es que nunca entre aca, si tengo temporal es xq un bloque tengo que tener
		registro = armarRegistroNulo();
	}
	if (registro == NULL) { //Va a entrar acá si no encuentra la key en ningún temporal
		registro = armarRegistroNulo();
	}
	//log_info(logger, "Salgo de la función");
	list_destroy_and_destroy_elements(temporalesTabla,free); //@VALGRIND - PRUEBA DOBLE FREE
	return registro;
}

t_registroMemtable* registroMayorParticion(char* tabla, u_int16_t key,
		int particiones) {

	t_registroMemtable* registro = malloc(sizeof(t_registroMemtable));
	char* pathParticion = rutaParticion(tabla, particiones);

	//log_info(logger, "ruta %s", pathParticion);
	t_bloquesUsados* ListaBloques = leerTemporaloParticion(pathParticion);
	if (ListaBloques->bloques == NULL) {
		registro = armarRegistroNulo();
	} else {
		registro = leerBloquesConsecutivosUnaKey(ListaBloques->bloques,	ListaBloques->size, key, true);
		if(registro == NULL){
			registro = armarRegistroNulo();
		}
		list_destroy_and_destroy_elements(ListaBloques->bloques,free);
	}
	free(ListaBloques);
	free(pathParticion);
	return registro;

}

t_registroMemtable* tomarMayorRegistro(t_registroMemtable* reg1,
	t_registroMemtable* reg2, t_registroMemtable* reg3,
	t_registroMemtable* reg4) {
	t_registroMemtable *auxMayorPuntero;
	t_registroMemtable *retval;

	if (reg1->timestamp > reg2->timestamp) {
		auxMayorPuntero = reg1;
	} else {
		auxMayorPuntero = reg2;
	}

	if (reg3->timestamp > reg4->timestamp) {
		if (reg3->timestamp > auxMayorPuntero->timestamp) {
			auxMayorPuntero = reg3;
		}
	} else {
		if (reg4->timestamp > auxMayorPuntero->timestamp) {
			auxMayorPuntero = reg4;
		}
	}

	retval = crearCopiaRegistro(auxMayorPuntero); //@VALGRIND ESTE SE PUEDE LIBERAR? ES EL RETURN DEL SELECT

	return retval;
}

void iniciarSemaforosCompactacion(void)
{
	log_info(logger,"Busco tablas en disco");
	t_list *tablas_lista = obtenerTablas();
	if(tablas_lista == NULL){
		log_info(logger,"No hay tablas en disco");
		return;
	}
	for(int i=0; i<list_size(tablas_lista);i++){
		char *nombre = list_get(tablas_lista,i);
		log_info(logger,"Iniciando semáforos tabla %s",nombre);
		t_metadata_tabla *metadata = obtenerMetadataTabla(nombre);
		int tiempoCompactacion = metadata->compaction_time;

		RWlock rw_lock;
		Mutex drop_mx;
		rwLockIniciar(&rw_lock);
		mutexIniciar(&drop_mx);
		t_sems_tabla* sems_tabla;
		sems_tabla = malloc(sizeof(t_sems_tabla));
		sems_tabla->rwLockTabla = rw_lock;
		sems_tabla->drop_mx = drop_mx;
		char *copia = malloc(strlen(nombre)+1);
		strcpy(copia,nombre);
		dictionary_put(dicSemTablas,copia,sems_tabla);

		log_info(logger,"Iniciando hilo compactación tabla %s",nombre);
		pthread_t hilo;// = malloc(sizeof(pthread_t));
		args_compactacion_t *args = malloc(sizeof(args_compactacion_t));
		args->retardo = tiempoCompactacion;
		args->tabla = malloc(strlen(nombre)+1);
		strcpy(args->tabla,nombre);
		pthread_create(&hilo,NULL,(void*)correrCompactacion,args);
		pthread_t *hilo_copia = malloc(sizeof(pthread_t));
		memcpy(hilo_copia,&hilo, sizeof(pthread_t));
		pthread_detach(hilo);
		char *copia2 = malloc(strlen(nombre)+1);
		strcpy(copia2,nombre);
		dictionary_put(dicHilosCompactacion,copia2,hilo_copia);

		free(metadata->consistency);
		free(metadata);
	}
	list_destroy_and_destroy_elements(tablas_lista,free);
}

t_registroMemtable* comandoSelect(char* tabla, char* key) {

	t_registroMemtable* registroMayor;
	t_sems_tabla* semsTabla;

	if (verificarTabla(tabla) == -1) {
		registroMayor = armarRegistroNulo();
		registroMayor->tam_registro = -1;
		return registroMayor;
	} else { // archivo de tabla encontrado

		semsTabla = dictionary_get(dicSemTablas, tabla);
		if(semsTabla == NULL){
			log_info(logger,"es null");
		}

		//LEER
		//log_info(logger, "[DEBUG] Voy a bloquear tabla %s",tabla);
		rwLockLeer(&(semsTabla->rwLockTabla));

		t_metadata_tabla *metadata = obtenerMetadataTabla(tabla);

		//rwLockDesbloquear(&(semsTabla->rwLockTabla));

		//@SCRIP -> SI PONGO EL DESBLOQUEAR ACA EN VEZ DE ABAJO, NO SE CUELGA MAS

		if (metadata == NULL) {
			registroMayor = armarRegistroNulo();
			registroMayor->tam_registro = -2;
			//log_info(logger, "[DEBUG] Voy a desbloquear tabla %s",tabla);
			rwLockDesbloquear(&(semsTabla->rwLockTabla)); //@martin si voy a retornar tengo que desbloquear no?
			return registroMayor;
		}; // 0: OK. -1: ERROR. // frenar en caso de error

		int valorKey = atoi(key);


		u_int16_t valorKeyU = strtol(key, NULL, 10);

		int particiones = determinarParticion(valorKey, metadata->particiones);

				t_registroMemtable* registroParticion;

		registroParticion = registroMayorParticion(tabla, valorKeyU,
				particiones);

		t_registroMemtable* registroTemporal = registroMayorTemporal(tabla,
				valorKeyU, ".tmp");

		t_registroMemtable* registroTemporalC = registroMayorTemporal(tabla,
				valorKeyU, ".tmpc");

		//log_info(logger, "[DEBUG] Voy a desbloquear tabla %s",tabla);
		rwLockDesbloquear(&(semsTabla->rwLockTabla));

		//log_info(logger, "[DEBUG] Voy a bloquear memtable");
		rwLockLeer(&sem_rw_memtable);
		t_registroMemtable* registroMemtable = registroMayorMemtable(tabla,	valorKeyU);
		//log_info(logger, "[DEBUG] Voy a desbloquear memtable");
		rwLockDesbloquear(&sem_rw_memtable);

		registroMayor = tomarMayorRegistro(registroMemtable, registroParticion,
				registroTemporal, registroTemporalC);


		//Como la función tomarMayorRegistro crea una copia del mayor, puedo borrar todos
		borrarRegistro(registroMemtable);
		borrarRegistro(registroParticion);
		borrarRegistro(registroTemporal);
		borrarRegistro(registroTemporalC);

		log_info(logger, "el timestamp mas grande es %llu",registroMayor->timestamp);

		if (registroMayor->timestamp > 0) {
			log_info(logger, "Registro obtenido: <%llu;%d;%s>",registroMayor->timestamp, registroMayor->key,registroMayor->value);
			//printf("Registro obtenido: <%llu;%d;%s>\n", registroMayor->timestamp,registroMayor->key, registroMayor->value);
		} else {
			log_info(logger, "No se encontró ningún registro con la key %d",
					valorKey);
		//	printf("No se encontró ningún registro con la key %d\n", valorKey);
		}

//		free(metadata);

		// NACHO
		free(metadata->consistency); //@nacho agregado
		free(metadata); //@nacho agregado

		return registroMayor;

	}
}

int comandoDrop(char* tabla) {

	t_sems_tabla* semsTabla;

	log_info(logger, "Por verificar tabla");

	int retornoVerificar = verificarTabla(tabla);
	if (retornoVerificar == 0) {


	//	log_info(logger,"[DEBUG] tabla %s",tabla);
		semsTabla = dictionary_get(dicSemTablas, tabla);

		if(semsTabla == NULL){
			log_info(logger,"[DEBUG] es null");
		}

		mutexBloquear(&(semsTabla->drop_mx));
		rwLockEscribir(&(semsTabla->rwLockTabla));

		pthread_t *hiloCompactacion = dictionary_get(dicHilosCompactacion,tabla);


//		pthread_kill(*hiloCompactacion, SIGKILL);

//		dictionary_remove_and_destroy(dicHilosCompactacion,tabla,free);
		dictionary_remove(dicHilosCompactacion,tabla);


		char*path = armarPathTabla(tabla);
		log_info(logger, "Vamos a eliminar la tabla: %s", path);
		free(path);

//		mutexBloquear(&memtable_mx);
		rwLockEscribir(&sem_rw_memtable);
		t_list *lista_reg = dictionary_get(memtable,tabla);
		dictionary_remove(memtable,tabla);
//		mutexDesbloquear(&memtable_mx);
		if (lista_reg != NULL)
			list_destroy_and_destroy_elements(lista_reg,(void*)borrarRegistro);

		mutexBloquear(&listaTablasInsertadas_mx);
		for(int i=0; i<list_size(listaTablasInsertadas);i++){
			char *tablaBorrar = list_get(listaTablasInsertadas,i);
			if(!strcmp(tablaBorrar,tabla)){
				list_remove_and_destroy_element(listaTablasInsertadas,i,free);
				break;
			}
		}
		mutexDesbloquear(&listaTablasInsertadas_mx);
		rwLockDesbloquear(&sem_rw_memtable); //@sincro revisar si no caemos en interbloqueo

		eliminarTablaCompleta(tabla);

		pthread_cancel(*hiloCompactacion);

		dictionary_remove(dicSemTablas,tabla);

		rwLockDesbloquear(&(semsTabla->rwLockTabla));
		mutexDesbloquear(&(semsTabla->drop_mx));

		free(semsTabla);

		return retornoVerificar;
	} else {
		return -1;
	}

}

int comandoCreate(char* tabla, char* consistencia, char* particiones,
		char* tiempoCompactacion) {

	int retornoVerificar = verificarTabla(tabla);
	if (retornoVerificar == -1) {     // La tabla no existe, se crea


//		log_info(logger,"%s---%s",tabla,tablaAverificar);
		char *path = armarPathTabla(tabla);

		mkdir(path, 0777);

		log_info(logger, "Se crea la tabla y su direccion es %s ", path);


		free(path);

		FILE* archivoMetadata;

		char *path_tabla_metadata = armarPathMetadataTabla(tabla);

		archivoMetadata = fopen(path_tabla_metadata, "w");

		free(path_tabla_metadata);

		if (archivoMetadata != NULL) {
			log_info(logger,"El archivo metadata se creo satisfactoriamente");

			log_info(logger, "Se crean los semaforos de la tabla");

			RWlock rw_lock;
			Mutex drop_mx;
			rwLockIniciar(&rw_lock);
			mutexIniciar(&drop_mx);

			t_sems_tabla* sems_tabla;
			sems_tabla = malloc(sizeof(t_sems_tabla));

			sems_tabla->rwLockTabla = rw_lock;
			sems_tabla->drop_mx = drop_mx;

			char *copia = malloc(strlen(tabla)+1);
			strcpy(copia,tabla);

			//log_info(logger,"create tabla %s",copia);
			dictionary_put(dicSemTablas, copia, sems_tabla);

			int tamanioConsistencia = strlen(consistencia)
					+ sizeof("CONSISTENCY=") + 4;
			char *lineaConsistencia = malloc(tamanioConsistencia);

			snprintf(lineaConsistencia, tamanioConsistencia, "CONSISTENCY=%s\n",
					consistencia);

			//log_info(logger, "Se agrego la consistencia %s", lineaConsistencia);

			int tamanioParticiones = strlen(particiones) + sizeof("PARTITIONS=")
					+ 4;
			char *lineaParticiones = malloc(tamanioParticiones);

			snprintf(lineaParticiones, tamanioParticiones, "PARTITIONS=%s\n",
					particiones);

			//log_info(logger, "Se agregaron las particiones %s",lineaParticiones);

			int tamanioTiempoC = strlen(tiempoCompactacion)
					+ strlen("COMPACTION_TIME=") + 1;
			char *lineaTiempoCompactacion = malloc(tamanioTiempoC);

			snprintf(lineaTiempoCompactacion, tamanioTiempoC,"COMPACTION_TIME=%s", tiempoCompactacion);

			//log_info(logger, "Se agrego el tiempo de compactacion %s",lineaTiempoCompactacion);

			log_info(logger, "Metadata de la tabla creada CONSISTENCY=%s,PARTITIONS=%s,COMPACTION_TIME=%s",consistencia,particiones,tiempoCompactacion);

			fputs(lineaConsistencia, archivoMetadata);
			fputs(lineaParticiones, archivoMetadata);
			fputs(lineaTiempoCompactacion, archivoMetadata);


			fclose(archivoMetadata);
			free(lineaConsistencia);
			free(lineaParticiones);
			free(lineaTiempoCompactacion);



			int aux = atoi(particiones);

			//log_info(logger, "aux=%d", aux);
			log_info(logger, "Por crear particiones, cantidad %d ", aux-1);
			for (int i = 0; i < aux; i++) {
				char* archivoParticion = rutaParticion(tabla, i);
				FILE* particion;
				particion = fopen(archivoParticion, "w");

				int tamanio = string_length("SIZE=") + sizeof(int)
						+ string_length("BLOCKS=[]") + sizeof(int) + 4;
				int bloqueLibre = obtenerPrimerBloqueLibreBitmap();
				ocuparBloqueLibreBitmap(bloqueLibre);

				char* lineaParticion = malloc(tamanio);

				snprintf(lineaParticion, tamanio, "SIZE=0\nBLOCKS=[%d]",
						bloqueLibre);

				fputs(lineaParticion, particion);
				free(lineaParticion);
				//log_info(logger, "Particion creada: %s", archivoParticion);
				free(archivoParticion);
				fclose(particion);

			}
			pthread_t hilo;// = malloc(sizeof(pthread_t));
			args_compactacion_t *args = malloc(sizeof(args_compactacion_t));
			args->retardo = strtoull(tiempoCompactacion,NULL,10);
			args->tabla = malloc(strlen(tabla)+1);
			strcpy(args->tabla,tabla);
			pthread_create(&hilo,NULL,(void*)correrCompactacion,args);
			pthread_t *hilo_copia = malloc(sizeof(pthread_t));
			memcpy(hilo_copia,&hilo, sizeof(pthread_t));
			pthread_detach(hilo);
			char *copia2 = malloc(strlen(tabla)+1);
			strcpy(copia2,tabla);
			dictionary_put(dicHilosCompactacion,copia2,hilo_copia);
			return 0;
		} else {
			log_info(logger, "No se pudo crear el archivo metadata");
			fclose(archivoMetadata);
			return -1;
		}
	} else {
		log_info(logger, "La tabla ya existe");
		return -2;

	}

}

int comandoInsertSinTimestamp(char* tabla, char* key, char* value) {

	u_int64_t aux = timestamp();

	char timestamp_s[40];
	sprintf(timestamp_s, "%llu", aux);

	//log_info(logger, "el timestamp a agregar es: %s", timestamp_s);

	return comandoInsert(tabla, key, value, timestamp_s);

}

int comandoInsert(char* tabla, char* key, char* valueOriginal, char* timestamp) {
	int retornoVerificar = verificarTabla(tabla);

	if (retornoVerificar == 0) {
		char* value = malloc(strlen(valueOriginal) + 1);
		strcpy(value, valueOriginal);


		bool verificarValue = validarValue(value);
		bool verificarKey = validarKey(key);
		bool algunoContiene = (verificarValue || verificarKey);

		if (!algunoContiene) {

			char* valueDesenmascarado = desenmascararValue(value);

			t_registroMemtable* registroPorAgregarE = armarEstructura(valueDesenmascarado, key, timestamp);
			free(valueDesenmascarado);
			// Verifico que la key ya exista en el memtable, aca se hace el dump
//			mutexBloquear(&memtable_mx);
			rwLockEscribir(&sem_rw_memtable);
			bool tablaRepetida = dictionary_has_key(memtable, tabla);
//			mutexDesbloquear(&memtable_mx);
		//	log_info(logger, "valor tablaRepetida %d", tablaRepetida);

			if (tablaRepetida) {
				log_info(logger, "Encontre una tabla repetida");
//				mutexBloquear(&memtable_mx);

				t_list* tableRegisters = dictionary_get(memtable, tabla);
				list_add(tableRegisters, registroPorAgregarE);

				log_info(logger,"Agregué registro a la memtable <%llu,%d;%s>",registroPorAgregarE->timestamp,registroPorAgregarE->key, registroPorAgregarE->value);
				//log_info(logger, "[DEBUG] esta tabla en la memtable tiene: ");
//				t_list* tableRegisters2 = dictionary_get(memtable, tabla);
			//	log_info(logger, "[DEBUG] tamaño tabla registros %d : ", list_size(tableRegisters2));
//				for (int i = 0; i < list_size(tableRegisters2); i++) {
//					t_registroMemtable * regAux = list_get(tableRegisters2, i);
//					//log_info(logger, "[DEBUG] %d <%llu;%d;%s>", i,regAux->timestamp, regAux->key, regAux->value);
//				}

//				mutexDesbloquear(&memtable_mx);

			}
			else {
				log_info(logger, "La tabla no esta repetida");
				t_list* listaAux = list_create();
				list_add(listaAux, registroPorAgregarE);
				char* aux = malloc(strlen(tabla) + 1);
//				char* aux2 = malloc(strlen(tabla) + 1); //@martin @revisar
				strcpy(aux, tabla);
//				strcpy(aux2, tabla);//@MARTIN
//				mutexBloquear(&memtable_mx);
				dictionary_put(memtable, aux, listaAux);
//				mutexDesbloquear(&memtable_mx);
				mutexBloquear(&listaTablasInsertadas_mx);
				list_add(listaTablasInsertadas, aux); //puede llegar a romper, agregar un aux
				mutexDesbloquear(&listaTablasInsertadas_mx);

			}

			rwLockDesbloquear(&sem_rw_memtable);
			 //free(value);

		}
		//@VALGRIND
		free(value);
	} else {
		retornoVerificar = -1;
	}

	return retornoVerificar;
}

char* comandoDescribeEspecifico(char* tabla) {

	t_metadata_tabla *metadata;
	t_sems_tabla* semsTabla;

	if (verificarTabla(tabla) == 0) {
		semsTabla = dictionary_get(dicSemTablas, tabla);
		rwLockLeer(&(semsTabla->rwLockTabla));
		metadata = obtenerMetadataTabla(tabla);
		if (metadata != NULL) {
			char* resultado = retornarValores(tabla, metadata);
			free(metadata->consistency);
			free(metadata);
			log_info(logger, "Resultado describe %s", resultado);
			rwLockDesbloquear(&(semsTabla->rwLockTabla));
			return resultado;
		} else {
			rwLockDesbloquear(&(semsTabla->rwLockTabla));
			return NULL;
		}
	} else {
		return NULL;
	}
}

char* comandoDescribe() {

	/*for(int i=0; i<dictionary_size(dicSemTablas); i++) {
		semsTabla = dictionary_get(dicSemTablas, i);
		//rwLockLeer(semsTabla->rwLockTabla);
	}*///@gian
	char* resultado = retornarValoresDirectorio();
	if (resultado != NULL)
		log_info(logger, "Resultado describe de todas las tablas %s",
				resultado);

	/*for(int i=0; i<dictionary_size(dicSemTablas); i++) {
		t_sems_tabla* semsTabla = dictionary_get(i);
		//rwLockDesbloquear(semsTabla->rwLockTabla);
	}*/
	return resultado;
}

t_list *obtenerArchivosDirectorio(char *path, char *terminacion) {
	t_list *retval = list_create();

	struct dirent *de;  // Pointer for directory entry
	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir(path);
	;

	if (dr == NULL)  // opendir returns NULL if couldn't open directory
	{
		return NULL;
	}
	char *path_completo;
	int size;
	while ((de = readdir(dr)) != NULL) {
		if (string_ends_with(de->d_name, terminacion)) {
			size = strlen(path) + strlen(de->d_name) + 2;
			path_completo = malloc(size);
			snprintf(path_completo, size, "%s/%s", path, de->d_name);
			list_add(retval, path_completo);
		}
	}
	closedir(dr);
	return retval;
}

t_list *obtenerTablas(void)
{
	t_list *retval = list_create();

	struct dirent *de;  // Pointer for directory entry
	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir(tabla_Path);
	;

	if (dr == NULL)  // opendir returns NULL if couldn't open directory
	{
		return NULL;
	}
	char *path;
	int size;
	while ((de = readdir(dr)) != NULL) {
		if(!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
			continue;
		size = strlen(de->d_name) + 2;
		path = malloc(size);
		snprintf(path, size, "%s", de->d_name);
		list_add(retval, path);
	}
	closedir(dr);
	return retval;
}

t_registroMemtable *leerBloquesConsecutivosUnaKey(t_list *nroBloques,
		int tam_total, uint16_t key_buscada, bool es_unica) {
	FILE *bloque;

	t_registroMemtable *registro = malloc(sizeof(t_registroMemtable));
	registro->value = malloc(configFile->tamanio_value + 1);
	strcpy(registro->value, "");
	t_registroMemtable *retval = malloc(sizeof(t_registroMemtable));
	retval->value = malloc(configFile->tamanio_value + 1);
	retval->timestamp = 0;
	strcpy(retval->value, "");

	char *aux_value = malloc(configFile->tamanio_value + 1);

	estadoLecturaBloque_t estado = EST_LEER;
	estadoLecturaBloque_t anterior = EST_TIMESTAMP;

	void *aux_campo_leyendo;
	if (configFile->tamanio_value + 1 > sizeof(u_int64_t))
		aux_campo_leyendo = malloc(configFile->tamanio_value + 1);
	else
		aux_campo_leyendo = malloc(sizeof(u_int64_t));
	int offset_bloque = 0, tam_bloque = 0, offset_campo = 0, offset_total = 0,
			bloques_leidos = 0, num_separador = 0;
	bool leiValueEntero;
	log_info(logger, "[OBTENIENDO KEY BLOQUES] Voy a buscar la key %d",
			key_buscada);
	void* registros_bloque = NULL;
	while (offset_total < tam_total && estado != EST_FIN) {
//		log_info(logger,"Hasta ahora leí %d bytes de %d",offset_total, tam_total);
		switch (estado) {
		case EST_LEER:
			if (bloques_leidos == list_size(nroBloques)) {
				estado = EST_FIN;
				break;
			}
			offset_bloque = 0;
			int *nBloque = list_get(nroBloques, bloques_leidos);
//			log_info(logger,"[OBTENIENDO KEY BLOQUES] Leyendo el bloque nro %d, que es el bloque %d",	bloques_leidos, *nBloque);
			tam_bloque = abrirArchivoBloque(&bloque, *nBloque, "rb");
//			log_info(logger,"[OBTENIENDO KEY BLOQUES] El tamaño del bloque es: %d",	tam_bloque);
			if (registros_bloque != NULL)
				free(registros_bloque);
			registros_bloque = malloc(tam_bloque);
			if (fread(registros_bloque, tam_bloque, 1, bloque) == 0) {
				log_info(logger,"[OBTENIENDO KEY BLOQUES] Error al leer el bloque %d",*nBloque);
				return NULL;
			}
			fclose(bloque);
//			log_info(logger,"[OBTENIENDO KEY BLOQUES] Bloque leído correctamente");
			bloques_leidos++;
			estado = anterior;
			break;

		case EST_TIMESTAMP:
//				log_info(logger, "Buscando el timestamp");
//				log_info(logger, "Al bloque le quedan %d bytes y yo necesito %d",tam_bloque-offset_bloque, sizeof(u_int64_t)-offset_campo);

			//Si con los bytes que le quedan al bloque me alcanza para completar el campo, los copio y avanzo al siguiente estado
			if (offset_bloque + sizeof(u_int64_t) - offset_campo <= tam_bloque) {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						sizeof(u_int64_t) - offset_campo);
				memcpy(&(registro->timestamp), aux_campo_leyendo,sizeof(u_int64_t));
//					log_info(logger, "Timestamp leido: %d",registro->timestamp);

				//Avanzo los offset los bytes que acabo de leer
				offset_bloque += sizeof(u_int64_t) - offset_campo;
				offset_total += sizeof(u_int64_t) - offset_campo;

				//Avanzo al siguiente estado que es buscar un separador
				anterior = estado;
				estado = EST_SEP;
			}
			//Si no me alcanza, copio todo lo que puedo y voy a leer un nuevo bloque
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque);

				//Avanzo los offset los bytes que acabo de leer
				offset_campo += tam_bloque - offset_bloque;
				offset_total += tam_bloque - offset_bloque;
				offset_bloque += tam_bloque - offset_bloque;
//					log_info(logger, "Me faltan %d bytes para leer el timestamp",sizeof(u_int64_t)-offset_campo);

				//Voy a leer un nuevo bloque
				anterior = estado;
				estado = EST_LEER;
			}
			break;

		case EST_KEY:
//				log_info(logger, "Buscando key");
//				log_info(logger, "Al bloque le quedan %d bytes y yo necesito %d",tam_bloque-offset_bloque, sizeof(uint16_t)-offset_campo);

			//Si con los bytes que le quedan al bloque me alcanza para completar el campo, los copio y avanzo al siguiente estado
			if (offset_bloque + sizeof(uint16_t) - offset_campo <= tam_bloque) {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						sizeof(uint16_t) - offset_campo);
				memcpy(&(registro->key), aux_campo_leyendo, sizeof(uint16_t));
//					log_info(logger, "Key leída: %d",registro->key);

				//Avanzo los offset los bytes que acabo de leer
				offset_bloque += sizeof(uint16_t) - offset_campo;
				offset_total += sizeof(uint16_t) - offset_campo;

				//Avanzo al siguiente estado que es buscar un separador
				anterior = estado;
				estado = EST_SEP;
			}
			//Si no me alcanza, copio todo lo que puedo y voy a leer un nuevo bloque
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque);

				//Avanzo los offset los bytes que acabo de leer
				offset_campo += tam_bloque - offset_bloque;
				offset_total += tam_bloque - offset_bloque;
				offset_bloque += tam_bloque - offset_bloque;
//					log_info(logger, "Me faltan %d bytes para leer la key",sizeof(uint16_t)-offset_campo);

				//Voy a leer un nuevo bloque
				anterior = estado;
				estado = EST_LEER;
			}
			break;

		case EST_VALUE:
//				log_info(logger, "Buscando value");
			leiValueEntero = false;

			//Si con los bytes que le quedan al bloque me alcanza para completar el tamaño máximo para un value, los copio y avanzo al siguiente estado
			if (offset_bloque + configFile->tamanio_value - offset_campo
					<= tam_bloque) {
				//log_info(logger, "Voy a copiar a aux %d bytes",configFile->tamanio_value - offset_campo);
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						configFile->tamanio_value - offset_campo);
				memcpy(aux_value, aux_campo_leyendo, configFile->tamanio_value);

				//Como al escribir no se escribe el caracter nulo, al leer lo agrego
				aux_value[configFile->tamanio_value] = '\0';
				leiValueEntero = true;
			}
			//Si no me alcanza, leo todo lo que tenga y me fijo si el value está completo (puede tener menos bytes que el máximo)
			else {
				memcpy(aux_campo_leyendo + offset_campo,
						registros_bloque + offset_bloque,
						tam_bloque - offset_bloque);
				memcpy(aux_value, aux_campo_leyendo, tam_bloque - offset_bloque);

				//Como al escribir no se escribe el caracter nulo, al leer lo agrego
				aux_value[offset_campo + tam_bloque - offset_bloque] = '\0';

				//Si encuentro el \n, encontré el valor entero y no tengo que leer otro bloque para tener este campo
				if (string_contains(aux_value, "\n"))
					leiValueEntero = true;
				//Si no lo encuentro, sí tengo que leer otro bloque
				else {
					//Avanzo los offsets los bytes que acabo de leer
					offset_campo += tam_bloque - offset_bloque;
					offset_total += tam_bloque - offset_bloque;
					offset_bloque += tam_bloque - offset_bloque;

					//Voy a leer un nuevo bloque
					anterior = estado;
					estado = EST_LEER;
//						log_info(logger, "No encontré el '\\n' y me faltan %d bytes para el máximo del value",configFile->tamanio_value-offset_campo);
				}
			}
			//Si de cualquiera de las 2 formas pude completar el value, lo guardo y avanzo
			if (leiValueEntero) {
				//Corto el string en el \n y lo guardo en el campo value del registro
				char **aux_split = string_split(aux_value, "\n");
				strcpy(registro->value, aux_split[0]);

				//Borro toda la memoria que aloca la función string_split
				int i = 0;
				while (aux_split[i] != NULL) {
					free(aux_split[i]);
					i++;
				}
				free(aux_split);

				//Avanzo los offsets los bytes que acabo de leer
				offset_bloque += strlen(registro->value) - offset_campo;
				offset_total += strlen(registro->value) - offset_campo;

				//Calculo el tamaño
				registro->tam_registro = sizeof(u_int64_t) + sizeof(uint16_t) + strlen(registro->value) + 1;

				if (registro->key == key_buscada) {
					//log_info(logger, "[OBTENIENDO KEY BLOQUES] Encontre un registro con la key %d: <%d;%d;%s>", key_buscada, registro->timestamp,registro->key,registro->value);
					if (es_unica) {
//							log_info(logger, "[OBTENIENDO KEY BLOQUES] Al ser único, no sigo buscando");

						retval->key = registro->key;
						retval->tam_registro = registro->tam_registro;
						retval->timestamp = registro->timestamp;
						strcpy(retval->value, registro->value);

						anterior = estado;
						estado = EST_FIN;
					} else {
						if (registro->timestamp > retval->timestamp) {
							retval->key = registro->key;
							retval->tam_registro = registro->tam_registro;
							retval->timestamp = registro->timestamp;
							strcpy(retval->value, registro->value);
						}
						anterior = estado;
						estado = EST_SEP;
					}
				} else {
					anterior = estado;
					estado = EST_SEP;
				}
			}
			break;

		case EST_SEP:
			//log_info(logger, "Buscando un separador");
			//Si no tengo bytes para leer, voy a leer otro bloque
			if (offset_bloque == tam_bloque) {
				anterior = estado;
				estado = EST_LEER;
			}
			//Si tengo un byte, leo y avanzo
			else {
				//No me guardo los separadores porque no los necesito, simplemente avanzo los offsets
				offset_bloque += sizeof(char);
				offset_total += sizeof(char);

				//Voy al siguiente estado, para lo que necesito saber que número de separador estoy leyendo
				anterior = estado;
				switch (num_separador) {
				case 0:
					estado = EST_KEY;
					offset_campo = 0;
					num_separador++;
					break;
				case 1:
					estado = EST_VALUE;
					offset_campo = 0;
					num_separador++;
					break;
				case 2:
					estado = EST_TIMESTAMP;
					offset_campo = 0;
					num_separador = 0;
					break;
				}
				//log_info(logger, "Separador leído");
			}
			break;

		}
	}
	log_info(logger, "[OBTENIENDO KEY BLOQUES] Terminé de leer los bloques");

	//Libero la memoria auxiliar que usé
	if (registros_bloque != NULL)
		free(registros_bloque);
	free(aux_value);
	free(aux_campo_leyendo);

	//log_info(logger, "libere bloque no nulo");

	if (registro != NULL) {
		if (registro->value != NULL)
			free(registro->value);
		free(registro);
	}
//	log_info(logger, "por imprimir el retval");
	if (retval->timestamp != 0) {

		log_info(logger,"[OBTENIENDO KEY BLOQUES] El registro con mayor timestamp es <%llu;%d;%s>",retval->timestamp, retval->key, retval->value);
	} else {
		log_info(logger,"[OBTENIENDO KEY BLOQUES] No encontré un registro con la key indicada");
	}
	return retval;
}

void cerrarTodo() {
	//rwLockEscribir();
	liberarTodosLosRecursosGlobalesQueNoSeCerraron();
	sem_destroy(&semaforoQueries);
	if(dictionary_size(memtable)>0){
		rwLockEscribir(&sem_rw_memtable);
//		mutexBloquear(&memtable_mx); //@martin @sincro agrego
		vaciarMemtable();
//		mutexDesbloquear(&memtable_mx); //@martin @sincro agrego
		rwLockDesbloquear(&sem_rw_memtable);
	}
	dictionary_destroy(memtable);
	free(metadataLFS->magic_number);
	free(metadataLFS);
	list_destroy_and_destroy_elements(listaTablasInsertadas,free);
	dictionary_destroy_and_destroy_elements(dicSemTablas,free);
	dictionary_destroy_and_destroy_elements(dicHilosCompactacion,(void*)matarYBorrarHilos);
	//rwLockDesbloquear();

	/*for(int i=0; i<dictionary_size(dicSemTablas); i++) {
			semsTabla = dictionary_get(dicSemTablas, i);
			//rwLockLeer(semsTabla->rwLockTabla);
		}

		*///@gian por aqui

		/*for(int i=0; i<dictionary_size(dicSemTablas); i++) {
			t_sems_tabla* semsTabla = dictionary_get(i);
			//rwLockDesbloquear(semsTabla->rwLockTabla);
		}*/
}

void matarYBorrarHilos(pthread_t *thread)
{
	//pthread_kill(*thread,SIGKILL);
	pthread_cancel(*thread);
	free(thread);
}

void vaciarMemtable(void)
{
	//mutexBloquear(&memtable_mx); @martin @sincro comento
	dictionary_clean_and_destroy_elements(memtable,(void *)borrarListaMemtable);
	//mutexDesbloquear(&memtable_mx); @martin @sincro comento
}

void borrarListaMemtable(t_list *lista)
{
	list_destroy_and_destroy_elements(lista,(void *)borrarRegistro);
}

void liberarTodosLosRecursosGlobalesQueNoSeCerraron() {
	free(metadataLFS->magic_number);
	free(metadataLFS);
	free(bitmapPath);
	free(tabla_Path);

	free(configFile->ip);
	free(configFile->punto_montaje);
	free(configFile);
}

t_datos_particion *obtenerDatosParticion(char *path_particion) {
	bool error = false;
	char **bloques;
	t_datos_particion *retval = malloc(sizeof(t_datos_particion));

	log_info(logger, "[DATOS PARTICION] Cargando datos de particion %s",
			path_particion);

	t_config *config_particion = config_create(path_particion);

	if (config_particion == NULL) {
		log_warning(logger,
				"[OBTENER DATOS PARTICION] Error al leer la metadata de la partición");
		free(retval);
		return NULL;
	}

	if (config_has_property(config_particion, "SIZE")) {
		retval->size = config_get_int_value(config_particion, "SIZE");
	} else {
		error = true;
	}
	if (config_has_property(config_particion, "BLOCKS")) {
		bloques = config_get_array_value(config_particion, "BLOCKS");
	} else {
		error = true;
	}

	if (error) {
		log_warning(logger,
				"[OBTENER DATOS PARTICION] Error al obtener los datos de la partición");
		free(retval);
		return NULL;
	}
	retval->bloques = list_create();
	for (int i = 0; bloques[i] != NULL; i++) {
		int aux = atoi(bloques[i]);
		int *num = malloc(sizeof(int));
		memcpy(num, &aux, sizeof(int));
		list_add(retval->bloques, num);
	}
	config_destroy(config_particion);
	borrar_array_config(bloques);
	log_info(logger, "[OBTENER DATOS PARTICION] Datos obtenidos");
	return retval;
}

void borrar_array_config(char **array)
{
	if(array != NULL){
		int i = 0;
		while(array[i]!=NULL){
			free(array[i]);
			i++;
		}
		free(array);
	}
	array = NULL;
}


/* COMPACTACIÓN */

void *correrCompactacion(args_compactacion_t *args)
{

	//char *tabla = malloc(strlen(args->tabla)+1);
	char* tabla = args->tabla;
	//strcpy(tabla,args->tabla);
	uint64_t retardo = args->retardo;
	free(args);
	log_info(logger,"[COMPACTACION] Iniciando hilo tabla %s. Retardo %llu milisegundos",tabla,retardo);
	usleep(retardo*1000);
	while(1){
		log_info(logger,"[COMPACTACION] Se lanzara compactacion tabla %s",tabla);
		if(compactarTabla(tabla)>0)
			printf("***Se compacto tabla %s***\n>",tabla);
		usleep(retardo*1000);
	}
}

int compactarTabla(char *tabla) {

	t_sems_tabla* semsTabla;

	//Renombro los temporales
	log_info(logger,
			"[COMPACTACION] Entrando a proceso de compactación de tabla %s",
			tabla);

	char *path_tabla = armarPathTabla(tabla);
	t_list * temporales_list = obtenerArchivosDirectorio(path_tabla, ".tmp");

	if (list_size(temporales_list) == 0) {
		log_info(logger,"[COMPACTACION] No se encontraron temporales de la tabla %s. No se hace compactación",	tabla);
		list_destroy_and_destroy_elements(temporales_list, free);
		free(path_tabla);
		return 0;
	}

	semsTabla = dictionary_get(dicSemTablas, tabla);

	mutexBloquear(&(semsTabla->drop_mx));
	rwLockEscribir(&(semsTabla->rwLockTabla));

	char *path_tmp, *path_tmpc;
	for (int i = 0; i < list_size(temporales_list); i++) {
		path_tmp = list_get(temporales_list, i);
		path_tmpc = malloc(strlen(path_tmp) + 2);
		sprintf(path_tmpc, "%s%s", path_tmp, "c"); //Le agrego la "c" al final
		log_info(logger, "[COMPACTACION] Renombrando %s a %s", path_tmp,
				path_tmpc);
		if (rename(path_tmp, path_tmpc) == 0) {
			log_info(logger, "[COMPACTACION] Archivo renombrado correctamente");
		} else {
			log_error(logger, "[COMPACTACION] No se pudo renombrar el archivo");
			exit(-1);
		}
		free(path_tmpc);
	}

	rwLockDesbloquear(&(semsTabla->rwLockTabla));

	list_destroy_and_destroy_elements(temporales_list, free);

	log_info(logger,
			"[COMPACTACION] Se renombraron todos los temporales de %s correctamente",
			tabla);

	//Obtengo lo que hay en las particiones

	log_info(logger,
			"[COMPACTACION] Obtengo todos los archivos de particiones");

//	t_list *particiones_list = obtenerArchivosDirectorio(path_tabla,".bin");

	t_metadata_tabla *metadataTabla = obtenerMetadataTabla(tabla);
	if (metadataTabla == NULL) {
		log_error(logger,
				"[COMPACTACION] Error al leer la metadata de la tabla");
		free(path_tabla);
		return -1;
	}

	t_list *particiones_list = list_create();
	for (int i = 0; i < metadataTabla->particiones; i++) {
		char *aux_path_particion = malloc(strlen(path_tabla) + 20);
		sprintf(aux_path_particion, "%s/%d.bin", path_tabla, i);
		list_add(particiones_list, aux_path_particion);
	}
	free(metadataTabla->consistency); //@martin
	free(metadataTabla); //@martin

	if (list_size(particiones_list) == 0) {
		log_error(logger,
				"[COMPACTACION] No se encontraron particiones de la tabla %s",
				tabla);
		list_destroy_and_destroy_elements(particiones_list, free);
		free(path_tabla);
		return -1;
	}

	//Armo una lista indexada por numero de partición en la que voy a poner las listas de registros de todas las particiones
	t_list *registros_tabla = list_create();

	for (int i = 0; i < list_size(particiones_list); i++) {
		char *path_particion = list_get(particiones_list, i);
		t_datos_particion * datos_particion = obtenerDatosParticion(
				path_particion);
		if (datos_particion != NULL) {
			log_info(logger,
					"[COMPACTACION] Se obtuvieron los datos de la particion %s",
					path_particion);
			log_info(logger, "[COMPACTACION] Tamaño: %d",
					datos_particion->size);
			for (int j = 0; j < list_size(datos_particion->bloques); j++) {
				int *num_aux = list_get(datos_particion->bloques, j);
				log_info(logger, "[COMPACTACION] (%d) Bloque: %d", j, *num_aux);
			}
			log_info(logger,
					"[COMPACTACION] Voy a leer todo lo que hay en la partición");

			t_list *registros_particion = leerBloquesConsecutivos(datos_particion->bloques, datos_particion->size);
			list_add(registros_tabla, registros_particion);
			log_info(logger,
					"[COMPACTACION] %d registros de la particion %d agregados a la lista",
					list_size(registros_particion), i);
			list_destroy_and_destroy_elements(datos_particion->bloques, free);
			free(datos_particion);

		} else {
			log_info(logger,
					"[COMPACTACION] Error al obtener los datos de la partición %d",
					i);
		}
	}

	list_destroy_and_destroy_elements(particiones_list, free);

	log_info(logger,
			"[COMPACTACION] Todas las particiones fueron agregadas a la lista");

	/* Obtengo lo que hay en el temporal
	 * Unifico lo de las participaciones y lo del temporal
	 * Repito para cada temporal
	 */
	t_list *tmpc_list = obtenerArchivosDirectorio(path_tabla, ".tmpc");
	for (int i = 0; i < list_size(tmpc_list); i++) {
		char *path_tmpc = list_get(tmpc_list, i);
		t_datos_particion *datos_particion = obtenerDatosParticion(path_tmpc);
		if (datos_particion != NULL) {
			log_info(logger,
					"[COMPACTACION] Se obtuvieron los datos del temporal %s",
					path_tmpc);
			log_info(logger, "[COMPACTACION] Tamaño: %d",
					datos_particion->size);
			for (int j = 0; j < list_size(datos_particion->bloques); j++) {
				int *num_aux = list_get(datos_particion->bloques, j);
				log_info(logger, "[COMPACTACION] (%d) Bloque: %d", j, *num_aux);
			}
			log_info(logger,
					"[COMPACTACION] Voy a leer todo lo que hay en el temporal");
			t_list *registros_temporal = leerBloquesConsecutivos(
					datos_particion->bloques, datos_particion->size);

			actualizarListaRegistros(registros_tabla, registros_temporal);

			log_info(logger,
					"[COMPACTACION] Registros del temporal %s agregados a las particiones",
					path_tmpc);

			list_destroy_and_destroy_elements(registros_temporal,
					(void*) borrarRegistro);
			list_destroy_and_destroy_elements(datos_particion->bloques,free); //@VALGRIND
			free(datos_particion);
		} else {
			log_info(logger,
					"[COMPACTACION] Error al obtener los datos de la partición %d",
					i);
		}
	}

	list_destroy_and_destroy_elements(tmpc_list, free);

	rwLockEscribir(&(semsTabla->rwLockTabla));

	u_int64_t comienzoBloqueo = timestamp();

	//Borro temporales de compactación y libero sus bloques
	liberarBloquesDeArchivo(path_tabla, ".tmpc");

	//Libero bloques particiones​
	liberarBloquesDeArchivo(path_tabla, ".bin");

	//Llevar registro del tiempo que estuvo bloqueada

	//Pido nuevos bloques y escribo
	for (int i = 0; i < list_size(registros_tabla); i++) {
		t_list* registros_particion = list_get(registros_tabla, i);
		log_info(logger,
				"[COMPACTACION] Guardando los registros de la partición %d", i);
		guardarRegistrosParticion(path_tabla, i, registros_particion);
	}

	u_int64_t finBloqueo = timestamp();

	rwLockDesbloquear(&(semsTabla->rwLockTabla));
	mutexDesbloquear(&(semsTabla->drop_mx));

	u_int64_t tiempoQueEstuvoBloqueada = finBloqueo - comienzoBloqueo;

	log_info(logger, "[COMPACTACION] Tabla %s estuvo bloqueada %d milisegundos", tabla, tiempoQueEstuvoBloqueada);
	//Libero la memoria
	for (int i = 0; i < list_size(registros_tabla); i++) {
		t_list *aux = list_get(registros_tabla, i);
		list_destroy_and_destroy_elements(aux, (void*) borrarRegistro);
	}
	list_destroy(registros_tabla);

	log_info(logger,
			"[COMPACTACION] Compactacion de la tabla %s realizada con exito",
			tabla);

	free(path_tabla);
	return 1;
}

void actualizarListaRegistros(t_list *listas_registros, t_list *nuevos) {
	int particion;
	int cant_particiones = list_size(listas_registros);
	for (int i = 0; i < list_size(nuevos); i++) {
		t_registroMemtable *aux_registro = list_get(nuevos, i);
		particion = determinarParticion(aux_registro->key, cant_particiones);
		t_list *registros_particion = list_get(listas_registros, particion);
		incorporarRegistro(registros_particion, aux_registro);
	}
}

void incorporarRegistro(t_list *registros, t_registroMemtable *nuevo) {
	//log_debug(logger, "[INCORPORANDO REGISTRO]");
	log_debug(logger, "[INCORPORANDO REGISTRO] El que quiero agregar es: <%llu;%d;%s>", nuevo->timestamp, nuevo->key,nuevo->value);
	//mprimirRegistro(nuevo);
	for (int i = 0; i < list_size(registros); i++) {
		t_registroMemtable *aux = list_get(registros, i);
		if (aux->key == nuevo->key) {
			log_debug(logger, "[INCORPORANDO REGISTRO] El registro que habia antes es: <%llu;%d;%s>", aux->timestamp, aux->key,aux->value);
			imprimirRegistro(aux);
			if (aux->timestamp < nuevo->timestamp) {
				free(aux->value);
				aux->value = malloc(strlen(nuevo->value) + 1);
				strcpy(aux->value, nuevo->value);
				aux->timestamp = nuevo->timestamp;
				aux->tam_registro = sizeof(uint16_t) + sizeof(uint64_t)
						+ strlen(aux->value) + 1; //@martin, @gian: se calcula así?
				log_debug(logger,"[INCORPORANDO REGISTRO] Actualicé el registro anterior");
			} else {
				log_debug(logger,"[INCORPORANDO REGISTRO] El registro anterior era más reciente");
			}
			return;
		}
	}
	//Si llego hastá acá, no teníamos la key
	t_registroMemtable *copia = crearCopiaRegistro(nuevo);
	list_add(registros, copia);
	log_debug(logger,
			"[INCORPORANDO REGISTRO] No estaba esta key. Registro agregado");
}

void liberarBloquesDeArchivo(char *path_tabla, char *extension) {
	log_info(logger, "[COMPACTACION] Borrando temporales de compactacion");
	t_list *tmpc_list = obtenerArchivosDirectorio(path_tabla, extension);
	for (int i = 0; i < list_size(tmpc_list); i++) {
		char *path = list_get(tmpc_list, i);
		t_datos_particion *datos_tmpc = obtenerDatosParticion(path);
		for (int j = 0; j < list_size(datos_tmpc->bloques); j++) {
			int *bloque = list_get(datos_tmpc->bloques, j);
			liberarBloqueBitmap(*bloque);
//			log_info(logger, "[COMPACTACION] Bloque %d liberado", *bloque);
		}
		list_destroy_and_destroy_elements(datos_tmpc->bloques, free);
		free(datos_tmpc); //@martin
		remove(path);
	}
	list_destroy_and_destroy_elements(tmpc_list, free);
}

int guardarRegistrosParticion(char *path_tabla, int particion,
		t_list *registros_list) {
	t_list* bloquesUsados = list_create();
	int tam_total_registros = tamTotalListaRegistros(registros_list);
	int cantidad_bloques = cuantosBloquesNecesito(tam_total_registros);
	if (cantidad_bloques == 0) {
		cantidad_bloques = 1;
	}
	int bloqueAux;
	int *bloqueLista;
	log_info(logger, "[COMPACTACION] Tengo %d bloques libres y necesito %d",
			cantBloquesLibresBitmap(), cantidad_bloques);
	if (cantBloquesLibresBitmap() >= cantidad_bloques) {
		for (int i = 0; i < cantidad_bloques; i++) {
			bloqueAux = obtenerPrimerBloqueLibreBitmap();
			bloqueLista = malloc(sizeof(int));
			*bloqueLista = bloqueAux;
			if (bloqueAux != -1) {
				ocuparBloqueLibreBitmap(bloqueAux);
				//list_add(bloquesUsados, (void*)bloqueAux);
				list_add(bloquesUsados, bloqueLista);
			} else {

				//liberar los bloques de la lista
				return -1;
			}
		}
	} else {
		log_error(logger,
				"[COMPACTACION] no hay bloques disponibles para hacer la compactación");
	}

	//void* bufferRegistros = malloc(tam_total_registros);
	void* bufferRegistros = armarBufferConRegistros(registros_list,
			tam_total_registros);
	int resultadoEscritura = escribirVariosBloques(bloquesUsados,
			tam_total_registros, bufferRegistros);

	char *path_particion = malloc(strlen(path_tabla) + 20);
	sprintf(path_particion, "%s/%d.bin", path_tabla, particion);

	if (resultadoEscritura != -1) {
		FILE* fp;
		fp = fopen(path_particion, "w");
		log_info(logger,
				"[COMPACTACION] creamos el archivo, ahora lo llenamos");
		log_info(logger, "[COMPACTACION] path del archivo %s", path_particion);

		if (fp != NULL) {
			char *contenido;
			contenido = string_new();
			string_append(&contenido, "SIZE=");
			char* size = malloc(10);
			sprintf(size, "%d", tam_total_registros);
			string_append(&contenido, size);
			string_append(&contenido, "\n");
			string_append(&contenido, "BLOCKS=[");
			char* bloque = malloc(10);
			for (int i = 0; i < list_size(bloquesUsados); i++) {
				int* nroBloque = list_get(bloquesUsados, i);
				sprintf(bloque, "%d", *nroBloque);
				string_append(&contenido, bloque);
				if(i!= list_size(bloquesUsados)-1) //@martin
					string_append(&contenido, ",");
			}
			/*contenido = stringTomarDesdeInicio(contenido,
					strlen(contenido) - 1);*/
			string_append(&contenido, "]");
			fputs(contenido, fp);
			log_info(logger, "[COMPACTACION] Temporal completado con: %s",
					contenido);
			free(contenido);
			free(size); //@martin
			free(bloque); //@martin
			fclose(fp);
		} else {
			//liberar bloques de la lista de bloquesUsados
			log_error(logger,
					"[COMPACTACION] Error al abrir el archivo con path: %s",
					path_particion);
		}
	} else {
		log_error(logger,
				"[COMPACTACION] Hubo un error al escribir los datos en los bloques");
	}

	free(path_particion);
	free(bufferRegistros);
	//list_clean_and_destroy_elements(bloquesUsados, free);
	list_destroy_and_destroy_elements(bloquesUsados, free);

	return 0;
}
