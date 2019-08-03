#include "memoria.h"
#include "gestionMemoria.h"
//#include "../Biblioteca/src/Biblioteca.c"
#include "../Biblioteca/src/Biblioteca.h"
#include "main_memoria.h"
//void terminar_memoria(t_log* g_log);


#define LOGGEAR_EN_CONSOLA

//#define MAIN_1
#ifdef MAIN_1
int main() {

	// LOGGING
	printf("INICIANDO EL MODULO MEMORIA \n COMINEZA EL TP\n***************************************\n");
	inicioLogYConfig();

//	crearConexionesConOtrosProcesos(); // conecta con LFS y puede que con kernel.

	printf("HACIENDO MEMORIA");

//	aux_tabla_paginas=malloc(sizeof(tabla_pagina));

	arc_config->max_value_key = 20;
	max_valor_key = arc_config->max_value_key;
	//ESTE ES SOLO TEST, DESPUES SE BORRA A LA MIERDA
		   max_valor_key=6;

	armarMemoriaPrincipal();

	iniciarSemaforosYMutex();

	//ESTE ES SOLO TEST, DESPUES SE BORRA A LA MIERDA
	   max_valor_key=6;
//    infoPagina* infoPag = malloc(sizeof(infoPag));

//	imprimirPorPantallaTodosLosComandosDisponibles();
	ejecutarHiloConsola();
//	consola_prueba();
/*
	   void* info = malloc(50);
	   insertHardcodeado(1, 1, info, "sdf", "TABLA1");
	   insertHardcodeado(2, 1, info, "xxx", "TABLA1");
	   insertHardcodeado(3, 1, info, "www", "TABLA1");
	   char* cadenapasada = malloc(10);
	   memcpy(cadenapasada, "TABLA1", 7);
	   selectHardcodeado(cadenapasada, 1, info);
	   selectHardcodeado(cadenapasada, 2, info);
	   selectHardcodeado(cadenapasada,3, info);
	   insertHardcodeado(4, 1, info, "aaa", "TABLA1");
	   selectHardcodeado(cadenapasada,4, info);
	   selectHardcodeado(cadenapasada,1, info);
	   selectHardcodeado(cadenapasada,3, info);
	   selectHardcodeado(cadenapasada,2, info);

	   insertHardcodeado(8, 1, info, "trd", "TABLA4");
	   selectHardcodeado(cadenapasada,2, info);

	   insertHardcodeado(4, 1, info, "xsa", "TABLA2");
	   insertHardcodeado(1, 1, info, "4fd", "TABLA3");
	   selectHardcodeado(cadenapasada,4, info);
	   insertHardcodeado(4, 1, info, "rrr", "TABLA1");
	   selectHardcodeado(cadenapasada,4, info);
	   memcpy(cadenapasada, "TABLA2", 7);
	   selectHardcodeado(cadenapasada,4, info);
	   memcpy(cadenapasada, "TABLA4", 7);
	   selectHardcodeado(cadenapasada,8, info);

	free(info);
	  */


    printf("TERMINADO\n");

    //    ejecutarHiloConsola();

    	liberar_todo_por_cierre_de_modulo();
    	return 0;
}


void consola_prueba() {
	int fin = 0;
	request_t req;
	int retardo;
	char *linea;
	datosRequest* datosDeRequest = malloc(sizeof(datosRequest));
		datosDeRequest->req1 = malloc(datosDeRequest->tamanioReq1);
		datosDeRequest->req2 = malloc(datosDeRequest->tamanioReq2);
		datosDeRequest->req3 = malloc(datosDeRequest->tamanioReq3);
	printf("\n\n");
	imprimirPorPantallaTodosLosComandosDisponibles();
	int i;
	while(!fin){
		retardo_memoria(arc_config->retardo_mem);

		pthread_mutex_lock(&mutex_info_request);
		linea = readline(">>");
		req = parser(linea);
		free(linea);

		switch(req.command){
			case INSERT:
				if(req.cant_args == 3){
					printf("\nInsertando: <%s><%d><%s>\n\n",req.args[0],atoi(req.args[1]),req.args[2]);
					Hilo hiloInsertNuevo;
					hiloCrear(&hiloInsertNuevo, hiloInsert, &req);
					hiloEsperar(hiloInsertNuevo);
				} else {
					imprimirError(log_memoria, "[COMANDO INSERT]\nEl comando INSERT esta mal ingresado");
				}
				mutexDesbloquear(&mutex_info_request);
				break;
			case SELECT:
				if(req.cant_args == 2){
					printf("\nObteniendo: <%s><%d>\n\n",req.args[0],atoi(req.args[1]));
					Hilo hiloSelectNuevo;
					hiloCrear(&hiloSelectNuevo, hiloSelect, &req);
					hiloEsperar(hiloSelectNuevo);
				} else {
					imprimirError(log_memoria, "[COMANDO SELECT]\nEl comando SELECT esta mal ingresado");
				}
				mutexDesbloquear(&mutex_info_request);
				//HAY UN TEMA CON EL TEMA DEL REQ.ARGS 0 Y ES QUE SI NO PONGO NOMBRE
				//MANDA SARASA, REVISALO LUEGO Y CORREGILO
				break;
			case DESCRIBE:
				//FALLA SI PONGO OSLO DESCRIBE, ES ALGO DE LA REQ QUE NO ANDA
				if(req.cant_args == 0) {
					printf("\nObteniendo la metadata de todos los segmentos\n\n");
				} else {
					printf("\nObteniendo la metadata de: <%s>\n\n",req.args[0]);
				}

				Hilo hiloDescribes;
				hiloCrear(&hiloDescribes, hiloDescribe, &req);
				hiloEsperar(hiloDescribes);

			//	imprimirPorPantallaTodosLosComandosDisponibles();
			//	borrar_request(req);
				break;
			case DROP:
				if(req.cant_args == 1){
					printf("\nDROP de la tabla: <%s>\n\n",req.args[0]);
					Hilo drophilp;
					hiloCrear(&drophilp, hiloDrop, &req);
					hiloEsperar(drophilp);
			//		imprimirPorPantallaTodosLosComandosDisponibles();
				} else {
					imprimirError(log_memoria, "[COMANDO DROP]\nEl comando DROP esta mal ingresado");
				}
				mutexDesbloquear(&mutex_info_request);
		//		borrar_request(req);
				break;
			case RETARDO_MEMORIA:
				if(req.cant_args == 1){
					retardo = atoi(req.args[0]);
					mutexDesbloquear(&mutex_info_request);
					imprimirAviso1(log_memoria, "Cambiando el retardo de acceso a MEMORIA a [%d]",
							retardo);
					modificarTIempoRetardo(retardo, RETARDO_MEMORIA);
				} else {
					imprimirError(log_memoria, "[COMANDO RETARDO]\nEl comando RETARDO esta mal ingresado");
					mutexDesbloquear(&mutex_info_request);
				}

	//			imprimirPorPantallaTodosLosComandosDisponibles();

				break;
			case RETARDO_FS:
				if(req.cant_args == 1){
					retardo = atoi(req.args[0]);
					mutexDesbloquear(&mutex_info_request);
					imprimirAviso1(log_memoria, "Cambiando el retardo de acceso a LISANDRA a [%d]",
											retardo);
					modificarTIempoRetardo(retardo, RETARDO_FS);
				} else {
					imprimirError(log_memoria, "[COMANDO RETARDO]\nEl comando RETARDO esta mal ingresado");
					mutexDesbloquear(&mutex_info_request);
				}

		//		imprimirPorPantallaTodosLosComandosDisponibles();
				break;
			case RETARDO_GOSSIPING:
				if(req.cant_args == 1){
					retardo = atoi(req.args[0]);
					mutexDesbloquear(&mutex_info_request);
					imprimirAviso1(log_memoria, "Cambiando el retardo de GOSSIPING a [%d]",
							retardo);
					modificarTIempoRetardo(retardo, RETARDO_GOSSIPING);
				} else {
					imprimirError(log_memoria, "[COMANDO RETARDO]\nEl comando RETARDO esta mal ingresado");
					mutexDesbloquear(&mutex_info_request);
				}

		//		imprimirPorPantallaTodosLosComandosDisponibles();
				break;
			case RETARDO_JOURNAL:
				if(req.cant_args == 1){
					retardo = atoi(req.args[0]);
					mutexDesbloquear(&mutex_info_request);
					imprimirAviso1(log_memoria, "Cambiando el retardo de JOURNAL a [%d]",
									retardo);
					modificarTIempoRetardo(retardo, RETARDO_JOURNAL);
				} else {
					imprimirError(log_memoria, "[COMANDO RETARDO]\nEl comando RETARDO esta mal ingresado");
					mutexDesbloquear(&mutex_info_request);
				}

		//		imprimirPorPantallaTodosLosComandosDisponibles();
				break;
			case SALIR:
			//	borrar_request(req);
				mutexDesbloquear(&mutex_info_request);
				imprimirAviso(log_memoria, "\nEMPIEZA A CERRARSE TODO EL MODULO DE MEMORIA\n\n");
				fin = 1;
				break;
			default:
				imprimirAviso(log_memoria, "\n\nERROR AL REALIZAR LA CONSULTA, PRUEBE DE NUEVO\n\n");
			//	borrar_request(req);
				mutexDesbloquear(&mutex_info_request);

				break;
		}
		borrar_request(req);

	}
	free(datosDeRequest);
}
#endif

//void hiloDrop(request_t* req){
//	/*if(stringEstaVacio(req->args[0])){
//		imprimirError(log_memoria, "NO SE HA INGRESADO 1 NOMBRE CORRECTO\n");
//		return;
//	}*/
//	char* nombre = malloc(strlen(req->args[0]));
//	memcpy(nombre, req->args[0], strlen(req->args[0])+1);
//	if(funcionDrop(nombre)==-1){
//		imprimirError1(log_memoria, "\nERROR, La tabla ya fue eliminada o no existe: <%s>\n", nombre);
//	}
//	free(nombre);
//}
//
//void hiloDescribe(request_t* req){
//	char* nombre;
//	if(req->cant_args == 0){
//		mutexDesbloquear(&mutex_info_request);
//		//ES DESCRIBE DE TODAS LAS COSAS EN MEMORIA
//	//	printf("\n\nENTRA AQUI Y AGRADEZCO\n");
//		nombre = "";
//	//	sleep(5);
//		if(funcionDescribe(nombre)==-1){
//			printf("\nERROR, NO existe la METADATA de los segmentos\n");
//		}
//	} else {
//		nombre = malloc(strlen(req->args[0]));
//		memcpy(nombre, req->args[0], strlen(req->args[0])+1);
//		mutexDesbloquear(&mutex_info_request);
//		if(funcionDescribe(nombre)==-1){
//			printf("\nERROR, NO existe la METADATA de <%s>\n", nombre);
//		} else {
//			free(nombre);
//		}
//	}
//}
/*
void hiloInsert(request_t* req){
	char* nombreTabla = malloc(strlen(req->args[0]));
	char* valorAPoner = malloc(strlen(req->args[2]));
	memcpy(nombreTabla, req->args[0], strlen(req->args[0])+1);
	u_int16_t keyBuscada = atoi(req->args[1]);
	memcpy(valorAPoner, req->args[2], strlen(req->args[2])+1);

	//printf("PRUEBAS: \nVALUE A PONER: [%s] [%s]\n\n", nombreTabla, valorAPoner);
	if(funcionInsert(nombreTabla, keyBuscada, valorAPoner, true, -1)== -1){
		imprimirError(log_memoria, "[FUNCION INSERT]\n\nERROR: Mayor al pasar max value\n\n");
	}
	free(valorAPoner);
	free(nombreTabla);
//	imprimirPorPantallaTodosLosComandosDisponibles();
}*/

//void hiloSelect(request_t* req){
//	char* nombreTablaABuscar =malloc(strlen(req->args[0]));
//	u_int16_t keyBuscado = atoi(req->args[1]);
//	pagina_a_devolver* pagina_y_valor = malloc(sizeof(pagina_a_devolver));
//	memcpy(nombreTablaABuscar, req->args[0], strlen(req->args[0])+1);
//
////	void* informacion = malloc(sizeof(pagina)+max_valor_key);
//
//	char* valorABuscar = malloc(max_valor_key);
//	pagina_y_valor->value = malloc(max_valor_key);
//
//	if(funcionSelect(nombreTablaABuscar, keyBuscado, &pagina_y_valor, &valorABuscar)!=-1){
//		printf("\nSALI DE FUNCION SELECT\n");
//	//	memcpy(pagina_y_valor->value, valorABuscar, max_valor_key);
//	/*	pag = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(
//				nombreTablaABuscar,keyBuscado,&seg,&aux);
//		printf("\nAQUI 2\n");
//		pagina = selectPaginaPorPosicion(pag,informacion);
//		*/
//
//		printf("\n******************"
//				"DATOS DEL SELECT"
//				"******************\n"
//				"SEGMENTO [%s]\nKEY [%d]\nVALUE: [%s]\n", nombreTablaABuscar,
//				pagina_y_valor->key,pagina_y_valor->value);
//
//	} else {
//		printf("\nERROR <%s><%d>\n", nombreTablaABuscar, keyBuscado);
//	}
//	free(valorABuscar);
//	free(nombreTablaABuscar);
//	free(pagina_y_valor->value);
//	free(pagina_y_valor);
//
//}

void imprimirPorPantallaTodosLosComandosDisponibles(){
	printf("\n--------------------------------------------------------\n"
			"             COMANDOS QUE SE PUEDEN UTILIZAR\n"
			"	  USAR MAYUSCULAS PARA LOS COMANDOS Y SUS PARAMETROS\n"
			"--------------------------------------------------------\n\n"
			"SELECT <nombre Tabla> <Key a buscar>\n"
			"INSERT <nombre Tabla> <Key a poner> <Valor a ingresar>\n"
			"JOURNAL\n"
			"DESCRIBE [nombre tabla - OPCIONAL]\n"
			"CREATE <nombre tabla> <tipo> <particiones> <dump>\n"
			"DROP <nombre tabla>\n"
			/*"RETARDO_MEMORIA <milisegundos>\n"
			"RETARDO_FS <milisegundos>\n"
			"RETARDO_GOSSIPING <milisegundos>\n"
			"RETARDO_JOURNAL <milisegundos>\n"*/
			"SALIR\n");
}


/*void insertHardcodeado(int cant, int inicio, void* info, char* valorNuevo, char* nombreTabla){
	int i=0;
	log_info(log_memoria, "\n\n[X Insertar datos] Insertando datos en '%s'\n\nValor a poner ['%s']\n", nombreTabla, valorNuevo);
	// for(i=inicio; i<cant+inicio; i++){
	     printf("\nComienzo el insert NRO %d\n", i);
	     if(funcionInsert(nombreTabla, cant, valorNuevo, true, -1)!=-1){
	        printf("Se hizo el insert NRO %d\n", i);

	    } else {
	    	printf("ERROR CON %s\n", valorNuevo);
	    }

}*/

//void selectHardcodeado(char* nombreTablaAIr, u_int16_t keyBuscada, void* dato){
//	pagina_a_devolver* pag = malloc(sizeof(pagina_a_devolver));
////	funcionSelect(nombreTablaAIr, keyBuscada, &pag);
//	free(pag);
//}

void cerrarTodosLosHilosPendientes() {
	hiloCancelar(hiloConsolaMemoria);
}


void inicioLogYConfig(int numMemoria, bool loggearEnConsola) {
	tamanioPredefinidoParaNombreTabla = 50;
//	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
	int tam_path = 150;
	char *path_log = malloc(tam_path);
	char *path_config = malloc(tam_path);
	snprintf(path_log,tam_path, "../Log/LOG_MEMORIA_%d.txt",numMemoria);
	snprintf(path_config,tam_path, "../Config/MEMORIA_%d.txt",numMemoria);
	archivoLogValidar(path_log);
	log_memoria = log_create(path_log, "Proceso Memoria", loggearEnConsola, LOG_LEVEL_INFO);
	log_info(log_memoria," ========== Iniciación de Pool de Memoria ========== ");

	tablas_fp = fopen(LOG_TABLAS_PATH, "w");
	if(tablas_fp == NULL){
		log_info(log_memoria, "[LOGYCONFIG] No se pudo crear el archivo de seguimiento de tablas");
	}
	else{
		fprintf(tablas_fp,"ARCHIVO PARA SEGUIMIENTO DE TABLAS\n\n\n");
		log_info(log_memoria, "[LOGYCONFIG] Logger de seguimiento de tablas creado en %s",LOG_TABLAS_PATH);
	}
	cargarConfiguracion(path_config);

	printf("\n*Se cargo el archivo de config <%s>*\n\n",path_config);
	printf("\n*Se creo el log en <%s>*\n\n",path_log);
	log_info(log_memoria,"[LOGYCONFIG] *** CONFIGURACIÓN DE MEMORIA CARGADA. *** ");
	free(path_log);
	free(path_config);
}

/*-----------------------------------------------------------------------------
 * MEMORIA PRINCIPAL
 *--------------------------------------------------verificarSiBitmapLleno---------------------------*/

void iniciarSemaforosYMutex() {
	log_info(log_memoria, "[SEMAFOROS] Iniciando semaforos y mutexs");
	mutexIniciar(&memoria_mutex_paginas_disponibles);
	mutexIniciar(&mutex_tabla_pagina_en_modificacion);
	mutexIniciar(&mutex_segmento_en_modificacion);
	mutexIniciar(&mutex_limpiando_memoria);
	mutexIniciar(&mutex_pagina_auxiliar);
	mutexIniciar(&LRUMutex);
	mutexIniciar(&mutex_pagina_referenciada_aux);
	mutexIniciar(&mutex_pagina_referenciada_aux2);
	mutexIniciar(&mutex_crear_pagina_nueva);
	mutexIniciar(&ACCIONLRU);
	mutexIniciar(&JOURNALHecho);
	mutexIniciar(&mutex_memoria);
	mutexIniciar(&mutex_bitmap);
	mutexIniciar(&mutex_bloque_LRU_modificando);
	mutexIniciar(&mutex_info_request);
//	semaforoIniciar(&paginasSinUsar, cantPaginasTotales);

	mutexIniciar(&mutex_retardo_memoria);
	mutexIniciar(&mutex_retardo_gossiping);
	mutexIniciar(&mutex_retardo_fs);
	mutexIniciar(&mutex_retardo_journal);

	mutexIniciar(&mutex_bloquear_select_por_limpieza);

	mutexIniciar(&verificarSiBitmapLleno);
	mutexIniciar(&mutex_retardos_memoria);
//	iniciarSemaforosRetados();

//	log_info(log_memoria, "[SEMAFOROS] Semaforos y mutex inicializados");
//	log_info(log_memoria,	"[SEMAFOROS] Semaforo paginasSinUsar iniciada con valor '%d'",	cantPaginasTotales);
}

/*-----------------------------------------------------------------------------
 * MODIFICAR CONFIGURACION
 *-----------------------------------------------------------------------------*/

/*
 * BIRRA GRATIS:
 * Ahora que tengo tu atencion, debo cambiar el numero de retardo pero falla aqui
 */

void modificarTiempoRetardoMemoria(int nuevoCampo) {
//	config_set_value(configFile, "RETARDO_MEM", nuevoCampo);
	arc_config->retardo_mem = nuevoCampo;
}

void modificarTiempoRetardoFileSystem(int nuevoCampo) {
//	config_set_value(configFile, "RETARDO_FS", nuevoCampo);
	arc_config->retardo_fs = nuevoCampo;
}

void modificarTiempoRetardoGossiping(int nuevoCampo) {
//	config_set_value(configFile, "RETARDO_GOSSIPING", nuevoCampo);
	arc_config->retardo_gossiping = nuevoCampo;
}

void modificarTiempoRetardoJournal(int nuevoCampo) {
//	config_set_value(configFile, "RETARDO_JOURNAL", nuevoCampo);
	arc_config->retardo_journal = nuevoCampo;
}

void modificarTIempoRetardo(int nuevoCampo, int campoAModificar) {
//	t_config* configFile;
//	configFile = config_create(PATH_MEMORIA_CONFIG);
	if (arc_config == NULL) {
		log_error(log_memoria,
				"[MODIFICAR TIEMPO RETARDO]NO se abrio el archivo de configuracion");
		imprimirError(log_memoria,
				"[MODIFICAR TIEMPO RETARDO]NO se abrio el archivo de configuracion para MODIFICAR TIEMPO RETARDO");
		return;
	} else {
			switch(campoAModificar){
				case RETARDO_MEMORIA:
					modificarTiempoRetardoMemoria(nuevoCampo);
					log_info(log_memoria,
					"[MODIFICAR TIEMPO RETARDO]Se ha guardado el nuevo dato: RETARDO MEMORIA-> %d",
							nuevoCampo);
					break;
				case RETARDO_FS:
					modificarTiempoRetardoFileSystem(nuevoCampo);
					log_info(log_memoria,
					"[MODIFICAR TIEMPO RETARDO]Se ha guardado el nuevo dato: RETARDO FILESYSTEM-> %d",
						nuevoCampo);
					break;
				case RETARDO_GOSSIPING:
					log_info(log_memoria,
					"[MODIFICAR TIEMPO RETARDO]Se ha guardado el nuevo dato: RETARDO GOSSIPING-> %d",
						nuevoCampo);
					modificarTiempoRetardoGossiping(nuevoCampo);
					break;
				case RETARDO_JOURNAL:
					log_info(log_memoria,
					"[MODIFICAR TIEMPO RETARDO]Se ha guardado el nuevo dato: RETARDO JOURNAL-> %d",
						nuevoCampo);
					modificarTiempoRetardoJournal(nuevoCampo);
					break;
				default:
					imprimirError(log_memoria,
		"[MODIFICAR TIEMPO RETARDO] NO existe ese campo al que se quiere modificar algo");
					break;
			}
	}
}



/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CONEXION ENTRE PROCESOS
 *-----------------------------------------------------------------------------*/

//void crearConexionesConOtrosProcesos() {
//
//	log_info(log_memoria, "[HILOS] (+)");
//
//	pthread_t hiloClienteLFS;
//	pthread_create(&hiloClienteLFS, NULL,
//			(void*) conectarConServidorLisandraFileSystem, NULL);
//
//	pthread_detach(hiloClienteLFS);
//	log_info(log_memoria, "[HILOS] LANZADO CLIENTE LFS");
//
//	pthread_t hiloServidorKernel;
//	pthread_create(&hiloServidorKernel, NULL, (void*) levantarServidor, NULL);
//
//	pthread_detach(hiloServidorKernel);
//
//	log_info(log_memoria, "[HILOS] LANZADO  SERVIDOR KERNEL");
//
//	conectarConServidorLisandraFileSystem();
//	levantarServidor();
//	log_info(log_memoria, "[HILOS] (-)");
//	while (1)
//		;
//}

/*void conectarConServidorLisandraFileSystem() {
	imprimirAviso(log_memoria, "[CONEXION LSF]INICIAMOS LA CONEXION CON LFS");
	sockeConexionLF = nuevoSocket(log_memoria);

	if (sockeConexionLF == ERROR) {

		imprimirError(log_memoria, "[CONEXION LSF]ERROR al crear un socket");
		abortarProcesoPorUnErrorImportante(log_memoria,
				"NO se creo el socket con LisandraFS. Salimos del Proceso");
	}

	imprimirVerde(log_memoria,
			"[CONEXION LSF]Se ha creado un socket correctamente");

	log_info(log_memoria, "[CONEXION LSF]El Socket creado es: %d .",
			sockeConexionLF);

	log_info(log_memoria,
			"[CONEXION LSF]por llamar a la funcion connectarSocket() para conectarnos con Memoria");

	log_info(log_memoria, "[CONEXION LSF]PUERTO A CONECTAR: %d ",
			arc_config->puerto_fs);
	log_info(log_memoria, "[CONEXION LSF]PRUEBA: %d ", arc_config->puerto_fs);
//	char* ipLFS = "127.0.0.1";
	int resultado_Conectar = conectarSocket(sockeConexionLF, "0",
			arc_config->puerto_fs, log_memoria);
	// ## Acá IP de LFS Hardcodeada (#001#)

	if (resultado_Conectar == ERROR) {
		liberar_todo_por_cierre_de_modulo();
		abortarProcesoPorUnErrorImportante(log_memoria,
				"[CONEXION LSF]Hubo un problema al querer Conectarnos con LFS. Salimos del proceso");

	} else {

		imprimirVerde1(log_memoria,
				"[CONEXION LSF]Nos conectamos con exito, el resultado fue %d",
				resultado_Conectar);

		char * mensaje = "hola Lisandra";

		resultado_sendMsj = socketEnviar(sockeConexionLF, mensaje,
				strlen(mensaje) + 1, log_memoria);

		log_info(log_memoria,
				"[CONEXION LSF]Se ha intentado mandar un mensaje al server");

		if (resultado_sendMsj == ERROR) {
			liberar_todo_por_cierre_de_modulo();
			abortarProcesoPorUnErrorImportante(log_memoria,
					"[CONEXION LSF]Error al enviar mensaje a LSF. Salimos");
		}

		imprimirVerde1(log_memoria,
				"[CONEXION LSF]El mensaje se envio correctamente\n\nMENSAJE ENVIADO: %s",
				mensaje);

		buffer = malloc(sizeof(char));



//		 recibiendoMensaje = socketRecibir(sockeConexionLF, buffer, 13,  log_memoria);
//
//		 if(resultado_sendMsj == ERROR){
//		 imprimirError(log_memoria, "Error al recibir mensaje de LSF. salimos");
//		 return;
//		 }
//
//
//		 imprimirVerde1(log_memoria,"Se ha recibido un mensaje de LISANDRA\n\nMENSAJE RECIBIDO: %s", buffer);


		recibiendoMensaje = socketRecibir(sockeConexionLF, buffer, 3,
				log_memoria);

		if (resultado_sendMsj == ERROR) {
			imprimirError(log_memoria,
					"[CONEXION LSF]Error al recibir mensaje de LSF. salimos");
			return;
		}

		imprimirVerde1(log_memoria,
				"[CONEXION LSF]Se ha recibido el mensaje MAX VALUE de LISANDRA\n\nMENSAJE RECIBIDO: %s",
				buffer);

		//GUARDO ESTE VALOR QUE VA A SER IMPORTANTE PARA ARMAR LAS TABLAS
		log_info(log_memoria,
				"[CONEXION LSF]Guardando el valor MAX_VALUE_KEY: %s", buffer);

		arc_config->max_value_key = atoi(buffer);
		max_valor_key = arc_config->max_value_key;

		log_info(log_memoria, "[CONEXION LSF]max_value_key guardado: %i",
				arc_config->max_value_key);

		log_info(log_memoria, "[CONEXION LSF]Por liberar BUFFER");
		free(buffer);
		log_info(log_memoria, "[CONEXION LSF]BUFFER liberado");

		while (1) {

			mensaje = readline(">");

		} // while (1)
		  //	return;

//		char* msj = malloc(10*sizeof(char));
//		 msj = "PruebaK\n";
//		 resultado_sendMsj = socketEnviar(socket_CMemoria,msj,strlen(msj),log_kernel);
//		 if(resultado_sendMsj == ERROR){
//		 log_error(log_kernel,"Error al enviar mensaje a memoria. Salimos");
//		 return;
//		 }
//		 log_info(log_kernel,"El mensaje se envio correctamente");
	}
}*/

//void levantarServidor() {
//
//	str_com_t string;
//
//	msg_com_t recibido;
//
//	// SOCKET
//	socketEscuchaKernel = nuevoSocket(log_memoria);  // CREAR SOCKET
//	if (socketEscuchaKernel == ERROR) {                // CASO DE ERROR.
//		log_error(log_memoria,
//				" ¡¡¡ ERROR AL CREAR SOCKET. SE TERMINA EL PROCESO. !!! ");
//		return;
//	}
//	log_info(log_memoria, "SOCKET CREADO.Valor: %d.", socketEscuchaKernel);
//
//	// PUERTO
//	log_info(log_memoria, " *** SE VA A ASOCIAR SOCKET CON PUERTO ... *** ");
//	log_info(log_memoria, "PUERTO A USAR: %d.", arc_config->puerto);
//
//	// ASOCIAR "SOCKET" CON "PUERTO".
//	asociarSocket(socketEscuchaKernel     // SOCKET
//			, arc_config->puerto      // PUERTO
//			, log_memoria); // LOG
//	log_info(log_memoria, " *** PUERTO ASOCIADO A SOCKET EXITOSAMENTE. *** ");
//
//	// ESCUCHAR
//	socketEscuchar(socketEscuchaKernel    // SOCKET
//			, 10, log_memoria); // LOG
//	while (1) {
//		log_info(log_memoria, " +++ esperando conexiones... +++ ");
//		conexionEntrante = aceptarConexionSocket(socketEscuchaKernel,
//				log_memoria);
//		if (conexionEntrante == ERROR) {
//			log_error(log_memoria, "ERROR AL CONECTAR.");
//			return;
//		}
//		/*buffer = malloc(sizeof(t_header));
//		 recibiendoMensaje = socketRecibir(conexionEntrante, buffer,
//		 sizeof(t_header), log_memoria);
//
//		 printf("Recibimos por socket el comando: %d\n", buffer->comando);
//		 log_info(log_memoria, "El mensaje que se recibio fue con el comando %d",
//		 buffer->comando);
//
//		 printf("Recibimos por socket el tamanio que vendra en el body: %d\n",
//		 buffer->tamanio);
//		 log_info(log_memoria,
//		 "Recibimos un tamanio que vendra en el body de: %d",
//		 buffer->tamanio);
//
//		 printf(
//		 "Recibimos por socket la cantidad de argumentos que vendran en el body: %d\n",
//		 buffer->cantArgumentos);
//		 log_info(log_memoria,
//		 "Recibimos la cantidad de argumentos que vendran en el body de: %d",
//		 buffer->cantArgumentos);
//
//		 log_info(log_memoria,
//		 "El valor de retorno de la funcion que recibio el mensaje fue: %d",
//		 recibiendoMensaje);
//		 log_info(log_memoria, "El tamanio de la estructura t_header es: %d",
//		 sizeof(t_header));
//		 if (recibiendoMensaje == sizeof(t_header)) {
//
//		 log_info(log_memoria,
//		 "Por enviar confirmacion a Kernel de que recibimos correctamente");
//
//		 log_info(log_memoria,
//		 "El tamanio de la confirmacion que enviamos es de: %d",
//		 sizeof(recibiendoMensaje));
//		 int resultadoEnvio = socketEnviar(conexionEntrante,
//		 &recibiendoMensaje, sizeof(recibiendoMensaje), log_memoria);
//
//		 log_info(log_memoria,
//		 "Por hacer un malloc de: %d para guardar el body. ",
//		 buffer->tamanio);
//		 argumentosComando = malloc(buffer->tamanio);
//
//		 memset(argumentosComando, '\0', buffer->tamanio);
//
//		 recibiendoMensaje = socketRecibir(conexionEntrante,
//		 argumentosComando, buffer->tamanio, log_memoria);
//
//		 log_info(log_memoria, "Recibimos el/los argumentos: %s",
//		 argumentosComando);
//		 printf("Recibimos el/los argumentos: %s \n", argumentosComando);
//
//		 log_info(log_memoria, "Por parsear los argumentos.");
//
//		 argumentosParseados = string_split(argumentosComando, SEPARADOR);
//
//		 for (int i = 0; argumentosParseados[i] != NULL; i++) {
//
//		 log_info(log_memoria,
//		 "Parseando queda en la posicion %i: el valor: %s", i,
//		 argumentosParseados[i]);
//		 printf("Parseando queda en la posicion %i: el valor: %s \n", i,
//		 argumentosParseados[i]);
//
//		 }*/
//
//		/*
//		log_info(log_memoria, "Por llamar a recibir mensaje");
//		recibido = recibir_mensaje(conexionEntrante);
//
//		printf("\n\n***Me llego un request***");
//		log_info(log_memoria, "***Me llego un request***");
//
//		string = procesar_request(recibido);
//		borrar_mensaje(recibido);
//		printf("\n\nRequest: %s", string.str);
//		log_info(log_memoria, "Request: %s", string.str);
//*/
//
//		log_info(log_memoria, "Fin de parseo");
//		printf("Fin de parseo. \n");
//
////	}
//	}
//}

void crearHIloEscucharKernel() {
	pthread_t hiloEscucharKernel;
	log_info(log_memoria,
		"[HILO SERVER] *** HILO CREADO PARA ESCUCHA PERMANENTE *** ");
	hiloCrear(&hiloEscucharKernel, (void*) escucharConexionKernel, NULL);

// NO SE SI DEBE ESTAR ASI, LO DEJO POR SI ACASO
	pthread_detach(hiloEscucharKernel);
		log_info(log_memoria, "[HILO SERVER] ESCUCHANDO A LOS CLIENTES");
}

void escucharConexionKernel() {
	socketEscuchar(socketEscuchaKernel    // SOCKET
						, 10, log_memoria); // LOG
	str_com_t string;
	msg_com_t recibido;
	while (1) {
		log_info(log_memoria, " +++ esperando conexiones... +++ ");
		conexionEntrante = aceptarConexionSocket(socketEscuchaKernel, log_memoria);
			if (conexionEntrante == ERROR) {
		log_error(log_memoria, "ERROR AL CONECTAR.");
		abortarProcesoPorUnErrorImportante(log_memoria,
				"El socket que escucha a Kernel no se conecto");

			}
			buffer = malloc( 10 * sizeof(char) );

			recibiendoMensaje = socketRecibir(conexionEntrante, buffer, 10, log_memoria);
				log_info(log_memoria, "Recibimos por socket %s",buffer);
				log_info(log_memoria,"El mensaje que se recibio fue %s", buffer);

				log_info(log_memoria, "Por llamar a recibir mensaje");
				recibido = recibir_mensaje(conexionEntrante);

				printf("\n\n***Me llego un request***");
				log_info(log_memoria, "***Me llego un request***");
				string = procesar_request(recibido);
				borrar_mensaje(recibido);
			printf("\n\nRequest: %s", string.str);
			log_info(log_memoria, "Request: %s", string.str);

	}
}

/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CONSOLA
 *-----------------------------------------------------------------------------*/
/*
//void ejecutarHiloConsola() {
//	log_info(log_memoria, "[HILO CONSOLA]Inicializando HILO CONSOLA");
//
//	hiloCrear(&hiloConsolaMemoria, (void*)consola_prueba, NULL);
////	pthread_create(&hiloConsolaMemoria, NULL, consola_prueba(), NULL);
//	log_info(log_memoria, "[HILO CONSOLA]Se crea HILO CONSOLA");
//	//DUDAS RESPECTO A ESTE HILO, SI PONGO ESTO EMPIEZA A EJECUTAR Y NO PERMITIRA QUE OTROS ENTREN O QUE?
//	pthread_join(hiloConsolaMemoria, NULL);
//	log_info(log_memoria, "[HILO CONSOLA]HILO CONSOLA en ejecucion");
//}
//*/
//char* lectura_consola() {
//char* linea = (char*) readline(">");
//return linea;
//}
//
//
//void menu() {
//
//printf("Los comandos que se pueden ingresar son: \n"
//		"COMANDOS \n"
//		"Insert \n"
//		"Select \n"
//		"Create \n"
//		"Describe \n"
//		"Drop \n"
//		"Journal  \n"
//		"SALIR \n"
//		"\n");
//
//}
//
//
//int enviarComando(char** comando, t_log* logger) {
//
//log_info(logger, "En funcion enviarComando");
//
//char* msj = malloc(7 * sizeof(char));
//
//msj = comando;
//
//log_info(logger, "El mensaje que vamos a enviar es: %s", msj);
//
////AQUI TENGO UNA DUDA, LA CONEXION:
//
////LA CONEXION SE INICIA EN
////conectarConServidorLisandraFileSystem
////POR LO TANTO YA TENGO EL INT DE ESO ASI QUE NO TENGO QUE VOLVER A
////INICIARLA
////	sockeConexionLF = conexionKernel();
//
//log_info("Vamos a enviar a memoria por el socket %d", sockeConexionLF);
//
//resultado_sendMsj = socketEnviar(sockeConexionLF, msj, strlen(msj),
//		log_memoria);
//
//if (resultado_sendMsj == ERROR) {
//
//	log_error(log_memoria, "Error al enviar mensaje a memoria. Salimos");
//
//	return ERROR;
//}
//
//log_info(log_memoria, "El mensaje se envio correctamente: %s", msj);
//
//return 0;
//
//}

/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CARGA DE LA CONFIGURACION Y EL LOG
 *-----------------------------------------------------------------------------*/
void cargarConfiguracion(char *path_config) {
//	printf("\n\n**********CARGANDO CONFIGURACION**********\n\n");
	log_info(log_memoria, "[CONFIGURANDO MODULO] RESERVAR MEMORIA.");
//	if(arc_config != NULL){
//		//FUE CARGADA PREVIAMENTE POR LO TANTO DEBO LIMPIARLO PARA RECARGARLA DE NUEVO
//		free(arc_config->ip);
//	}
	arc_config = malloc(sizeof(t_memoria_config));

	// Lo inicializo así no generó segmentation fault al salir (puedo saber cuáles aloqué)
	arc_config->ip = NULL;
	arc_config->ip_fs = NULL;
	arc_config->ip_seeds = NULL;
	arc_config->puerto_seeds = NULL;

	log_info(log_memoria, "[CONFIGURANDO MODULO] BUSCANDO CONFIGURACION.");

	configFile = config_create(path_config);

	if (configFile != NULL) {

		log_info(log_memoria, "[CONFIGURANDO MODULO] LEYENDO CONFIGURACION...");

		if (config_has_property(configFile, "PUERTO")) {

			arc_config->puerto = config_get_int_value(configFile, "PUERTO");
			log_info(log_memoria, "PUERTO PARA MODULO MEMORIA: %d",
					arc_config->puerto);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY PUERTO CONFIGURADO");
		} // PUERTO

		if (config_has_property(configFile, "IP")) {

				arc_config->ip = config_get_string_value(configFile, "IP");
				log_info(log_memoria, "[CONFIGURANDO MODULO] IP DE ESCUCHA: %s",
						arc_config->ip);

			} else {
				log_error(log_memoria, "[ERROR] NO HAY IP CONFIGURADA");
			} // IP


		if (config_has_property(configFile, "IP_FS")) {

			arc_config->ip_fs = config_get_string_value(configFile, "IP_FS");
			log_info(log_memoria, "[CONFIGURANDO MODULO] IP DE FILESYSTEM: %s",
					arc_config->ip_fs);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY IP CONFIGURADA");
		} // IP FS

		if (config_has_property(configFile, "PUERTO_FS")) {

			arc_config->puerto_fs = config_get_int_value(configFile, "PUERTO_FS");
			log_info(log_memoria, "[CONFIGURANDO MODULO] PUERTO DE FILESYSTEM: %d",
					arc_config->puerto_fs);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY PUERTO PARA MODULO FS");
		} // PUERTO FS

		if (config_has_property(configFile, "IP_SEEDS")) {

			arc_config->ip_seeds = config_get_array_value(configFile, "IP_SEEDS");
			log_info(log_memoria, "[CONFIGURANDO MODULO] IPs DE SEEDS LEIDOS");

		} else {
			log_error(log_memoria, "[ERROR] NO HAY IPS PARA SEEDS");
		} // IP SEEDS

		if (config_has_property(configFile, "PUERTO_SEEDS")) {
			arc_config->puerto_seeds = config_get_array_value(configFile,
					"PUERTO_SEEDS");
			log_info(log_memoria, "[CONFIGURANDO MODULO] PUERTOS PARA SEEDS LEIDOS");

		} else {
			log_error(log_memoria,
					"[ERROR] NO SE ENCONTRARON LOS PUERTOS DE SEEDS");
		} // PUERTOS SEEDS

		if (config_has_property(configFile, "RETARDO_MEM")) {

			arc_config->retardo_mem = config_get_int_value(configFile,
					"RETARDO_MEM");
			log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO MEMORIA: %d",
					arc_config->retardo_mem);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY RETARDO CONFIGURADO");
		} // RETARDO DE MEMORIA

		if (config_has_property(configFile, "RETARDO_FS")) {

			arc_config->retardo_fs = config_get_int_value(configFile, "RETARDO_FS");
			log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO DEL FS: %d",
					arc_config->retardo_fs);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY RETARDO DE FS CONFIGURADO");
		} // RETARDO FS

		if (config_has_property(configFile, "TAM_MEM")) {

			arc_config->tam_mem = config_get_int_value(configFile, "TAM_MEM");
			log_info(log_memoria, "[CONFIGURANDO MODULO] TAMAÑO DE MEMORIA: %d",
					arc_config->tam_mem);

		} else {
			log_error(log_memoria, "[ERROR]NO HAY TAMAÑO DE MEMORIA CONFIGURADO");
		} // TAMAÑO DE MEMORIA

		if (config_has_property(configFile, "RETARDO_JOURNAL")) {

			arc_config->retardo_journal = config_get_int_value(configFile,
					"RETARDO_JOURNAL");
			log_info(log_memoria,
					"[CONFIGURANDO MODULO] RETARDO DEL JOURNALING: %d",
					arc_config->retardo_journal);

		} else {
			log_error(log_memoria,
					"[ERROR] NO HAY RETARDO DE JOURNALING CONFIGURADO");
		} // RETARDO JOURNALING

		if (config_has_property(configFile, "RETARDO_GOSSIPING")) {

			arc_config->retardo_gossiping = config_get_int_value(configFile,
					"RETARDO_GOSSIPING");
			log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO DE GOSSIPING: %d",
					arc_config->retardo_gossiping);

		} else {
			log_error(log_memoria,
					"[ERROR] NO HAY RETARDO DE GOSSIPING CONFIGURADO");
		} // RETARDO GOSSIPING

		if (config_has_property(configFile, "MEMORY_NUMBER")) {

			arc_config->memory_number = config_get_int_value(configFile,
					"MEMORY_NUMBER");
			log_info(log_memoria, "[CONFIGURANDO MODULO] NUMERO DE MEMORIA: %d",
					arc_config->memory_number);

		} else {
			log_error(log_memoria, "[ERROR] NO HAY NUMERO DE MEMORIA CONFIGURADO");
		} // MEMORY NUMBER

	} else {

		log_error(log_memoria,"[WARNING] NO EXISTE EL ARCHIVO DE CONFIGURACION <%s>",path_config); // ERROR: SIN ARCHIVO CONFIGURACION
		exit(1);

	}
}


/*
 * Lo hago en otra función para sólo modificar retardos
 * Al ser variables globales, se puede romper la memoria si modifican, por ej, el tam maximo del value
 * Obvio que depende de como se esté usando, pero mejor ahorremonos ese problema
 */

void recargarConfiguracion(char *path_config){

	log_info(log_memoria, "[ACTUALIZANDO RETARDOS] Voy a actualizar retardos");

	mutexBloquear(&mutex_retardos_memoria);

	t_config* auxConfigFile = config_create(path_config);

	if (auxConfigFile != NULL) {

		log_info(log_memoria, "[ACTUALIZANDO RETARDOS] LEYENDO CONFIGURACION...");

		if (config_has_property(auxConfigFile, "RETARDO_MEM")) {

			arc_config->retardo_mem = config_get_int_value(auxConfigFile,
					"RETARDO_MEM");
			log_info(log_memoria, "[ACTUALIZANDO RETARDOS] RETARTDO MEMORIA: %d",
					arc_config->retardo_mem);

		} else {
			log_error(log_memoria, "[ACTUALIZANDO RETARDOS] NO HAY RETARDO CONFIGURADO");
		} // RETARDO DE MEMORIA

		if (config_has_property(auxConfigFile, "RETARDO_FS")) {

			arc_config->retardo_fs = config_get_int_value(auxConfigFile, "RETARDO_FS");
			log_info(log_memoria, "[ACTUALIZANDO RETARDOS] RETARDO DEL FS: %d",
					arc_config->retardo_fs);

		} else {
			log_error(log_memoria, "[ACTUALIZANDO RETARDOS] NO HAY RETARDO DE FS CONFIGURADO");
		} // RETARDO FS

		if (config_has_property(auxConfigFile, "RETARDO_JOURNAL")) {

			arc_config->retardo_journal = config_get_int_value(auxConfigFile,
					"RETARDO_JOURNAL");
			log_info(log_memoria,
					"[ACTUALIZANDO RETARDOS] RETARDO DEL JOURNALING: %d",
					arc_config->retardo_journal);

		} else {
			log_error(log_memoria,
					"[ACTUALIZANDO RETARDOS] NO HAY RETARDO DE JOURNALING CONFIGURADO");
		} // RETARDO JOURNALING

		if (config_has_property(auxConfigFile, "RETARDO_GOSSIPING")) {

			arc_config->retardo_gossiping = config_get_int_value(auxConfigFile,
					"RETARDO_GOSSIPING");
			log_info(log_memoria, "[ACTUALIZANDO RETARDOS] RETARDO DE GOSSIPING: %d",
					arc_config->retardo_gossiping);

		} else {
			log_error(log_memoria,
					"[ACTUALIZANDO RETARDOS] NO HAY RETARDO DE GOSSIPING CONFIGURADO");
		} // RETARDO GOSSIPING

	} else {
		log_error(log_memoria,
				"[ACTUALIZANDO RETARDOS] NO HAY ARCHIVO DE CONFIGURACION DE MODULO MEMORIA"); // ERROR: SIN ARCHIVO CONFIGURACION
	}

	config_destroy(auxConfigFile);

	actualizar_retardo_gossiping(arc_config->retardo_gossiping);

	log_info(log_memoria, "[ACTUALIZANDO RETARDOS] RETARDOS ACTUALIZADOS CORRECTAMENTE");

	mutexDesbloquear(&mutex_retardos_memoria);
}


/*-----------------------------------------------------
 * FUNCIONES PARA LA ADMINISTRACION DE MEMORIA
 *-----------------------------------------------------*/

int JOURNAL(int socket_lfs) {
//	log_info(log_memoria, "[JOURNAL] EN JOURNAL");
	imprimirAviso(log_memoria, "[JOURNAL] ENTRANDO");
//	char* datosAPasar=NULL;
	datosJournal* journalAPasar;

	journalAPasar = obtener_todos_journal();
	log_info(log_memoria, "[JOURNAL] PROCEDO A ENVIAR LA INFORMACION A LISANDRA");

	log_info(log_memoria, "[JOURNAL] ENVIO EL MENSAJE A LISANDRA");

	int cant_pasados = pasarValoresALisandra(journalAPasar,socket_lfs);

	if(cant_pasados == 0){
		log_error(log_memoria,"[JOURNAL] No se pudo enviar ningun registro. No se hace el journal");
	//@gian @martin @nacho  Retornamos -1 aca?
	}
	log_info(log_memoria, "[JOURNAL] JOURNAL HECHO, LISANDRA HA RECIBIDO BIEN %d REGISTROS, LIMPIO TODO",cant_pasados);

	liberarDatosJournal(journalAPasar);
	mutexBloquear(&mutex_bloquear_select_por_limpieza);
	limpiezaGlobalDeMemoriaYSegmentos();
	mutexDesbloquear(&mutex_bloquear_select_por_limpieza);
	if(activo_retardo_journal){
		activo_retardo_journal=false;
		pthread_mutex_unlock(&JOURNALHecho);

//		pthread_cancel(journalHilo);
//		pthread_cancel(journalHilo);
	//	pthread_create(&journalHilo, NULL, retardo_journal, arc_config->retardo_journal);
//		pthread_detach(journalHilo);
	}
	return cant_pasados;
}

int pasarValoresALisandra(datosJournal* datos,int socket_lfs)
{
	bool debo_cerrar_socket = false;
	if(socket_lfs == -1){
		imprimirMensaje(log_memoria,"[ENVIANDO DATOS A LFS] Voy a crear conexión con LFS");
		socket_lfs = conectar_a_lfs(false,NULL);
		if(socket_lfs == -1){
			imprimirError(log_memoria,"[ENVIANDO DATOS A LFS] No fue posible conectarse al LFS");
			return -1;
		}
		else
			debo_cerrar_socket = true;
	}
	imprimirMensaje(log_memoria,"[ENVIANDO DATOS A LFS] Voy a enviarles los datos modificados");
	datosJournal *enviar = datos;
	req_com_t insert;
	char aux[100];
	int cont = 0;
	bool conexion_caida = false;
	while(enviar != NULL && !conexion_caida){
		snprintf(aux,100,"INSERT %s %d %s %llu",enviar->nombreTabla,enviar->key,enviar->value, enviar->timestamp);
		insert.tam = strlen(aux)+1;
		insert.str = malloc(insert.tam);
		strcpy(insert.str,aux);
		retardo_fs();
		imprimirMensaje1(log_memoria,"[ENVIANDO DATOS A LFS] Enviando <%s>",insert.str);
		if(enviar_request(socket_lfs,insert) == -1){
			imprimirError(log_memoria, "[ENVIANDO DATOS A LFS] No se puedo enviar el insert al filesystem");
			borrar_request_com(insert);
			conexion_caida = true;
			socket_lfs = -1;
			break;
		}
		borrar_request_com(insert);

		//Espero su respuesta
		msg_com_t msg = recibir_mensaje(socket_lfs);
//		retardo_fs();
		if(msg.tipo == RESPUESTA){
			resp_com_t recibido = procesar_respuesta(msg);
			borrar_mensaje(msg);
			if(recibido.tipo == RESP_OK){
				imprimirMensaje(log_memoria, "[ENVIANDO DATOS A LFS] El filesystem realizó el INSERT con éxito");
				cont++;
			}
			else{
				imprimirError(log_memoria, "[ENVIANDO DATOS A LFS] El filesystem no pudo realizar el INSERT");
				borrar_respuesta(recibido);
			}
			if(recibido.msg.tam>0)
				imprimirMensaje1(log_memoria,"[ENVIANDO DATOS A LFS] El filesystem contestó con %s",recibido.msg.str);
			borrar_respuesta(recibido);
		}
		else if(msg.tipo == DESCONECTADO){
			conexion_caida = true;
			socket_lfs = -1;
			borrar_mensaje(msg);
			break;
		}
		else{
			borrar_mensaje(msg);
		}
		enviar = enviar->sig;
	}
	if(debo_cerrar_socket)
		close(socket_lfs);
	return cont;
}

int procesoJournal(int socket_lfs){
	int cant_pasados = 0;
	mutexBloquear(&JOURNALHecho);
	hiloCancelar(journalHilo);
//	log_info(log_memoria, "[procesoJournal] Memoria esta full, procedo a hacer Journal");
//	printf("EN PROCESO JOURNAL\n\n");
//	retardo_journal(arc_config->retardo_journal);
	cant_pasados = JOURNAL(socket_lfs);
//	log_info(log_memoria, "[JOURNAL] JOURNAL REALIZADO, PROCEDO A REINICIAR EL HILO JOURNAL");
	pthread_create(&journalHilo, NULL, retardo_journal, arc_config->retardo_journal);
	hiloDetach(journalHilo);
	mutexDesbloquear(&JOURNALHecho);
	return cant_pasados;
}
