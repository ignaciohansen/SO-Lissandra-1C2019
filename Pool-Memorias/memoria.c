#include "memoria.h"
#include "gestionMemoria.h"
//#include "../Biblioteca/src/Biblioteca.c"

void terminar_memoria(t_log* g_log);
int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(char* nombreTabla, u_int16_t keyBuscada,
		segmento** segmentoBuscado, int* nroDePagina);

int main() {

	// LOGGING
	printf("INICIANDO EL MODULO MEMORIA \n COMINEZA EL TP\n***************************************\n");
	inicioLogYConfig();

//	crearConexionesConOtrosProcesos(); // conecta con LFS y puede que con kernel.

	printf("HACIENDO MEMORIA");
	aux_crear_pagina = malloc(sizeof(pagina));
	aux_devolver_pagina = malloc(sizeof(pagina_a_devolver));
	aux_segmento = malloc(sizeof(segmento));
	aux_tabla_paginas = malloc(sizeof(pagina_referenciada));
	aux_tabla_paginas2 = malloc(sizeof(pagina_referenciada));
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
					printf("\DROP de la tabla: <%s>\n\n",req.args[0]);
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

void hiloDrop(request_t* req){
	/*if(stringEstaVacio(req->args[0])){
		imprimirError(log_memoria, "NO SE HA INGRESADO 1 NOMBRE CORRECTO\n");
		return;
	}*/
	char* nombre = malloc(strlen(req->args[0]));
	memcpy(nombre, req->args[0], strlen(req->args[0])+1);
	if(funcionDrop(nombre)==-1){
		imprimirError1(log_memoria, "\nERROR, La tabla ya fue eliminada o no existe: <%s>\n", nombre);
	}
	free(nombre);
}

void hiloDescribe(request_t* req){
	char* nombre;
	if(req->cant_args == 0){
		mutexDesbloquear(&mutex_info_request);
		//ES DESCRIBE DE TODAS LAS COSAS EN MEMORIA
	//	printf("\n\nENTRA AQUI Y AGRADEZCO\n");
		nombre = "";
	//	sleep(5);
		if(funcionDescribe(nombre)==-1){
			printf("\nERROR, NO existe la METADATA de los segmentos\n");
		}
	} else {
		nombre = malloc(strlen(req->args[0]));
		memcpy(nombre, req->args[0], strlen(req->args[0])+1);
		mutexDesbloquear(&mutex_info_request);
		if(funcionDescribe(nombre)==-1){
			printf("\nERROR, NO existe la METADATA de <%s>\n", nombre);
		} else {
			free(nombre);
		}
	}
}

void hiloInsert(request_t* req){
	char* nombreTabla = malloc(strlen(req->args[0]));
	char* valorAPoner = malloc(strlen(req->args[2]));
	memcpy(nombreTabla, req->args[0], strlen(req->args[0])+1);
	u_int16_t keyBuscada = atoi(req->args[1]);
	memcpy(valorAPoner, req->args[2], strlen(req->args[2])+1);

//	printf("PRUEBAS: \nVALUE A PONER: [%s] [%s]\n\n", nombreTabla, valorAPoner);
	if(funcionInsert(nombreTabla, keyBuscada, valorAPoner, true)== -1){
		imprimirError(log_memoria, "[FUNCION INSERT]\n\nERROR: Mayor al pasar max value\n\n");
	}
	free(valorAPoner);
	free(nombreTabla);
//	imprimirPorPantallaTodosLosComandosDisponibles();
}

void hiloSelect(request_t* req){
	char* nombreTablaABuscar =malloc(strlen(req->args[0]));
	u_int16_t keyBuscado = atoi(req->args[1]);
	pagina_a_devolver* pagina_y_valor = malloc(sizeof(pagina_a_devolver));
	memcpy(nombreTablaABuscar, req->args[0], strlen(req->args[0])+1);

//	void* informacion = malloc(sizeof(pagina)+max_valor_key);

	char* valorABuscar = malloc(max_valor_key);
	pagina_y_valor->value = malloc(max_valor_key);

	if(funcionSelect(nombreTablaABuscar, keyBuscado, &pagina_y_valor, &valorABuscar)!=-1){
		printf("\SALI DE FUNCION SELECT\n");
	//	memcpy(pagina_y_valor->value, valorABuscar, max_valor_key);
	/*	pag = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(
				nombreTablaABuscar,keyBuscado,&seg,&aux);
		printf("\nAQUI 2\n");
		pagina = selectPaginaPorPosicion(pag,informacion);
		*/

		printf("\n******************"
				"DATOS DEL SELECT"
				"******************\n"
				"SEGMENTO [%s]\nKEY [%d]\nVALUE: [%s]\n", nombreTablaABuscar,
				pagina_y_valor->key,pagina_y_valor->value);

	} else {
		printf("\nERROR <%s><%d>\n", nombreTablaABuscar, keyBuscado);
	}
	free(valorABuscar);
	free(nombreTablaABuscar);
	free(pagina_y_valor->value);
	free(pagina_y_valor);

}

void imprimirPorPantallaTodosLosComandosDisponibles(){
	printf("\n--------------------------------------------------------\n"
			"             COMANDOS QUE SE PUEDEN UTILIZAR\n"
			"	  USAR MAYUSCULAS PARA LOS COMANDOS Y SUS PARAMETROS\n"
			"--------------------------------------------------------\n\n"
			"SELECT <nombre Tabla> <Key a buscar>\n"
			"INSERT <nombre Tabla> <Key a poner> <Valor a ingresar>\n"
			"JOURNAL\n"
			"DESCRIBE [nombre tabla - OPCIONAL]\n"
			"DROP <nombre tabla>\n"
			"RETARDO_MEMORIA <milisegundos>\n"
			"RETARDO_FS <milisegundos>\n"
			"RETARDO_GOSSIPING <milisegundos>\n"
			"RETARDO_JOURNAL <milisegundos>\n"
			"SALIR\n");
}

int funcionInsert(char* nombreTabla, u_int16_t keyBuscada, char* valorAPoner, bool estadoAPoner){
	log_info(log_memoria, "[INSERT] EN funcion INSERT");
	if(strlen(valorAPoner)>=max_valor_key){
		log_error(log_memoria, "[INSERT] El valor VALUE '%s' es mayor que el max value KEY\nMOTIVO: %d Mayor que %d",
				valorAPoner, strlen(valorAPoner), max_valor_key);
		return -1;
	}

	//ESTO ES PARA BLOQUEAR CUALQUIER INSERT NUEVO CUANDO SE ESTA REALIZANDO LRU O JOURNAL
	//ADEMAS, SI YA SE ESTA REALIZANDO 1 INSERT ENTONCES

	//mutexBloquear(&mutex_bloque_LRU_modificando);
	mutexBloquear(&ACCIONLRU);
	mutexDesbloquear(&ACCIONLRU);
	int nroPosicion=0;
	log_info(log_memoria, "[INSERT] EN funcion INSERT");
	segmento* segmentoBuscado = NULL;
	pagina_referenciada* ref = malloc(sizeof(pagina_referenciada));
	log_info(log_memoria, "[INSERT] Me pongo a buscar el segmento y la tabla en base a '%s' y '%d'",
			nombreTabla, keyBuscada);
	int posicionAIr =
			buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombreTabla, keyBuscada,
															&segmentoBuscado, &nroPosicion);

	if(posicionAIr==-1){
		log_info(log_memoria, "[INSERT] NO se encontro la posicion a donde debo ir");
		//CASO B, verifico si se encontro el segmento, caso contrario debo tambien crearlo
		if(segmentoBuscado==NULL){
			log_info(log_memoria, "[INSERT] Tampoco se encontro que existe un segmento asociado a la tabla '%s'/nProcedo a crear el segmento, tabla de pagina y alojar la pagina en memoria",
						nombreTabla);
			//NO SE ENCONTRO NINGUN SEGMENTO CON EL NOMBRE DE LA TABLA BUSCADA POR LO TANTO DEBO CREARLA

	//		log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina, el nro de su posicion es '%d'", nroTablaCreada);
			tabla_pagina_crear(keyBuscada, valorAPoner, estadoAPoner,
					&ref, nombreTabla, true, NULL);
			if(estadoAPoner) {
				log_info(log_memoria,
			"[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",
					ref->nropagina, keyBuscada);
			} else {
				log_info(log_memoria,
			"[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|FALSE",
					ref->nropagina, keyBuscada);
			}

			segmentoBuscado = segmento_crear(nombreTabla, ref);
			segmentoBuscado->paginasAsocida = ref;
			log_info(log_memoria, "[INSERT] SEGMENTO Creado para la tabla '%s' /nComo es el primer segmento TABLA SEGMENTOS apuntara este elemento", segmentoBuscado->path_tabla);
			segmentoBuscado->siguienteSegmento = tablaSegmentos;
			tablaSegmentos = segmentoBuscado;
			log_info(log_memoria, "[INSERT] Se ha creado un segmento para la tabla '%s'", nombreTabla);

		} else {
			log_info(log_memoria, "[INSERT] Se encontro un segmento asociado a la tabla '%s'",
					segmentoBuscado->path_tabla);
			//EXISTE EL SEGMENTO, SOLO CREO LA TABLA Y LA PAGINA Y SE LA ASIGNO A LA COLA DE TABLA DE PAGINAS DE SEGMENTO

			tabla_pagina_crear(keyBuscada, valorAPoner, estadoAPoner,
					&ref, nombreTabla, true, segmentoBuscado);

			if(estadoAPoner) {
				printf("[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",
					ref->nropagina, keyBuscada);
				log_info(log_memoria,
		"[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",
					ref->nropagina, keyBuscada);
						} else {
				printf("[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",
					ref->nropagina, keyBuscada);
				log_info(log_memoria,
		"[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|FALSE",
					ref->nropagina, keyBuscada);
				}
		//	printf("\n\n\nPASAMOS INSERT\n\n");
			pagina_referenciada* ref3 = segmentoBuscado->paginasAsocida;
	//		printf("\n\n\n%d\n\n", ref3->nropagina);
			while(ref3->sig!=NULL){
				ref3=ref3->sig;
		//		printf("\n\n\n%d\n\n", ref3->nropagina);
			}
			ref3->sig = ref;
		//	printf("\n\n\nPASAMOS INSERT\n\n");
		//	ref->sig = segmentoBuscado->paginasAsocida->sig;
		//	memcpy(segmentoBuscado->paginasAsocida->sig, ref->sig, sizeof(*pagina_referenciada));
		//	memcpy(segmentoBuscado->paginasAsocida, ref, sizeof(pagina_referenciada));
		//	segmentoBuscado->paginasAsocida = ref;
	//		nroTablaCreada = tabla_pagina_crear(keyBuscada, valorAPoner, true);
			log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina\nNro de su posicion es aux y nueva '%d'/'%d'",
					ref->nropagina, ref3->sig->nropagina);
	//		segmentoAgregarNroTabla(&segmentoBuscado, nroTablaCreada);
			log_info(log_memoria, "[INSERT] Se ha actualizado segmento para la tabla '%s'", nombreTabla);

			log_info(log_memoria, "[INSERT HECHO] Compruebo si se añadio la nueva tabla de paginas de '%s'", nombreTabla);
			pagina_referenciada* ref2 = segmentoBuscado->paginasAsocida;
			int i = 0;
			while(ref2!=NULL){
				i++;
				log_info(log_memoria, "\n[INSERT HECHO] Iteracion '%d'\nNro pagina: '%d'", i, ref2->nropagina);
				ref2 = ref2->sig;
			}
	//		free(ref);
		}
	} else {
		printf("[INSERT A MODIFICAR]\nExiste el segmento '%s' y la pagina que referencia la key (%d) que es NRO '%d'\nProcedo a poner el nuevo valor que es '%s'\n\n",
				segmentoBuscado->path_tabla, keyBuscada, posicionAIr, valorAPoner);
		/*
		 * SE CREO PERFECTAMENTE, CONOSCO EL SEGMENTO Y LA PAGINA A REFERENCIAR, PROCEDO A MODIFICAR Y ACCEDER
		 * A LA MEMORIA PARA LA MODIFICACION DE LOS CAMPOS
		 */
		log_info(log_memoria, "[INSERT A MODIFICAR]\nExiste el segmento '%s' y la pagina que referencia la key (%d) que es NRO '%d'\nProcedo a poner el nuevo valor que es '%s'",
				segmentoBuscado->path_tabla, keyBuscada, posicionAIr, valorAPoner);
		free(ref);

	//	free(segmentoBuscado);
		modificarValoresDeTablaYMemoriaAsociadasAKEY(posicionAIr, valorAPoner, nroPosicion);
	}
//	mutexDesbloquear(&mutex_bloque_LRU_modificando);

	return 1;
}

int funcionSelect(char* nombreTablaAIr, u_int16_t keyBuscada,
		pagina_a_devolver** dato, char** valorADevolver){
	int direccionPagina, nroDePagina;
	segmento *seg;

//	mutexBloquear(&mutex_bloque_LRU_modificando);
//	void* informacion = malloc(sizeof(pagina)+max_valor_key);
	log_info(log_memoria,
"[FUNCION SELECT] ENTRANDO POR NUEVA PETICION\nValor de key de los datos solicitados:\n\nSEGMENTO: % s \nKEY: %d",
			nombreTablaAIr,
			keyBuscada);
	direccionPagina = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey
			(nombreTablaAIr, keyBuscada, &seg, &nroDePagina);

	if(direccionPagina==-1){
		imprimirError2(log_memoria, "[FUNCION SELECT] ERROR, NO SE ENCONTRO NADA\n\nSEGMENTO BUSCADO: <%s>\nKEY BUSCADA: <%d>\n\n DEVUELVO ERROR",
					nombreTablaAIr, keyBuscada);
//			free(informacion);
			return 0;
		}
	log_info(log_memoria, "[FUNCION SELECT] Numero de pagina a donde debo ir: %d\nMe pongo a buscar los datos\n", direccionPagina);
	free((*dato)->value);
	free(*dato);
	*dato = selectPaginaPorPosicion(direccionPagina, true);
	modificar_bloque_LRU(NULL, timestamp(), direccionPagina, true, false);

//	printf("POR AQUI\n\n");
//	mutexDesbloquear(&mutex_bloque_LRU_modificando);

	log_info(log_memoria, "[FUNCION SELECT] Se encontro los datos");

	printf("[FUNCION SELECT] Se encontro los datos\n");
	printf("[FUNCION SELECT] <%s> <%d> \n", (*dato)->value, (*dato)->key);
/*	memcpy(*dato, datos_a_devolver, sizeof(pagina_a_devolver));
	printf("POR AQUI 2 \n");
	memcpy(*valorADevolver, informacion+sizeof(pagina)-1, max_valor_key);
//	memcpy(datos_a_devolver->value, informacion+sizeof(pagina)-1, max_valor_key);
	printf("POR AQUI 3 \n");
//	printf("POR AQUI\n\n");
	//ESTO LUEGO SE PODRIA COMENTAR PERO ES PARA ASEGURARNOS DE QUE RETORNA ALGO
	printf("\nSEGMENTO: <%s>\nKEY: <%d>\nValor: %s\n", nombreTablaAIr,
			datos_a_devolver->key,datos_a_devolver->value);
	free(datos_a_devolver->value);
	free(datos_a_devolver);
	*/
//	free(informacion);
	return 1;
}

int funcionDrop(char* nombre) {
	mutexBloquear(&ACCIONLRU);
	segmento* segmentoBuscado;
	segmento* segmentoAnterior = tablaSegmentos;
	pagina_referenciada* ref;
	log_info(log_memoria, "[FUNCION DROP] EN FUNCION DROP");

	mutexBloquear(&mutex_segmento_en_modificacion);

//	printf("[FUNCION DROP] aqui\n\n");
	segmentoBuscado = buscarSegmentoPorNombreTabla(nombre);
	if(segmentoBuscado==NULL){
		log_info(log_memoria, "[FUNCION DROP] No se encontro <%s> -> DevuelvoError", nombre);
		//NO HAY NADA ASI QUE DEVUELVO ERROR
		mutexDesbloquear(&ACCIONLRU);
		mutexDesbloquear(&mutex_segmento_en_modificacion);
		return -1;
	}
	if(strcmp(nombre, tablaSegmentos->path_tabla)==0){
		printf("[FUNCION DROP] Se encontro <%s> y es el primero de todos\n",
				tablaSegmentos->path_tabla);
		log_info(log_memoria, "[FUNCION DROP] Se encontro <%s> y es el primero de todos",
				tablaSegmentos->path_tabla);
		tablaSegmentos = tablaSegmentos->siguienteSegmento;
		limpiar_todos_los_elementos_de_1_segmento(segmentoAnterior);
		mutexDesbloquear(&ACCIONLRU);
		mutexDesbloquear(&mutex_segmento_en_modificacion);
		return 1;
	}
//	printf("[FUNCION DROP] Empiezo a buscar el anterior de ese segmento");
	log_info(log_memoria, "[FUNCION DROP] Empiezo a buscar el anterior de ese segmento");
	while(segmentoAnterior->siguienteSegmento != segmentoBuscado) {
		segmentoAnterior = segmentoAnterior->siguienteSegmento;
	}

	segmentoAnterior->siguienteSegmento=segmentoBuscado->siguienteSegmento;
	log_info(log_memoria, "[FUNCION DROP] Libero todo el segmento: <%s>", segmentoBuscado->path_tabla);
	limpiar_todos_los_elementos_de_1_segmento(segmentoBuscado);
	mutexDesbloquear(&ACCIONLRU);
	mutexDesbloquear(&mutex_segmento_en_modificacion);

	return 1;
}

int funcionDescribe(char* nombreTablaAIr){
	segmento* segmentoBuscado;
	pagina_referenciada* ref;
	void*info = malloc(sizeof(pagina)+max_valor_key);
	pagina_a_devolver* keyObtenida;
	if(stringEstaVacio(nombreTablaAIr)){
		log_info(log_memoria, "[FUNCION DESCRIBRE] EN DESCRIBE TOTAL");
		segmentoBuscado = tablaSegmentos;
		if(segmentoBuscado==NULL){
			imprimirError(log_memoria,
				"[FUNCION DESCRIBRE] DESCRIBE FALLA, no hay tablas cargadas en memoria");
			return -1;
		}

		while(segmentoBuscado!=NULL){
			printf("Obteniend datos de [%s]", segmentoBuscado->path_tabla);

			ref = segmentoBuscado->paginasAsocida;
			while(ref!=NULL){
				keyObtenida = selectPaginaPorPosicion(ref->nropagina, true);
				imprimirMensaje2(log_memoria, "\nDESCRIBE: Segmento|Key = [%s]-[%d]\n",
						segmentoBuscado->path_tabla, keyObtenida->key);
				ref = ref->sig;
				free(keyObtenida);
		//		sleep(1);
			}
			printf("Obteniend datos de ");
			printf("[%s]", segmentoBuscado->path_tabla);
			segmentoBuscado = segmentoBuscado->siguienteSegmento;
		}

	} else {
		log_info(log_memoria, "[FUNCION DESCRIBRE] En DESCRIBE de <%s>", nombreTablaAIr);
		segmentoBuscado = buscarSegmentoPorNombreTabla(nombreTablaAIr);


		 if(segmentoBuscado==NULL){
			imprimirError1(log_memoria,
					"[FUNCION DESCRIBRE] DESCRIBE FALLA, no existe la tabla <%s>",
					nombreTablaAIr);
			return -1;
		}
		ref = segmentoBuscado->paginasAsocida;
		while(ref!=NULL){
				keyObtenida = selectPaginaPorPosicion(ref->nropagina, info);
				imprimirMensaje2(log_memoria, "\nDESCRIBE: Segmento|Key = [%s]-[%d]\n",
					segmentoBuscado->path_tabla, keyObtenida->key);
				ref = ref->sig;
				free(keyObtenida);
			}

	}
	free(info);
	return 0;
}

void insertHardcodeado(int cant, int inicio, void* info, char* valorNuevo, char* nombreTabla){
	int i=0;
	log_info(log_memoria, "\n\n[X Insertar datos] Insertando datos en '%s'\n\nValor a poner ['%s']\n", nombreTabla, valorNuevo);
	// for(i=inicio; i<cant+inicio; i++){
	     printf("\nComienzo el insert NRO %d\n", i);
	     if(funcionInsert(nombreTabla, cant, valorNuevo, true)!=-1){
	        printf("Se hizo el insert NRO %d\n", i);

	    } else {
	    	printf("ERROR CON %s\n", valorNuevo);
	    }

}

void selectHardcodeado(char* nombreTablaAIr, u_int16_t keyBuscada, void* dato){
	pagina_a_devolver* pag = malloc(sizeof(pagina_a_devolver));
//	funcionSelect(nombreTablaAIr, keyBuscada, &pag);
	free(pag);
}

void cerrarTodosLosHilosPendientes() {
	hiloCancelar(hiloConsolaMemoria);
}

void liberar_todo_por_cierre_de_modulo() {
	//ESTE TIENE 1 ERROR, REVISARLO LUEGO
	//	cerrarTodosLosHilosPendientes();

	log_info(log_memoria,
			"[LIBERAR] Por liberar Segmentos y sus tablas de paginas");
	liberar_todo_segmento();

	//LIBERA LA RAM.
	log_info(log_memoria,
			"[LIBERAR] Empiezo a liberar todos los elementos que se han inicializado");
//	if(aux_crear_pagina!=NULL){
	log_info(log_memoria, "[LIBERAR] Por liberar aux_crear_pagina");
	free(aux_crear_pagina);
	log_info(log_memoria, "[LIBERAR] aux_crear_pagina liberado");
//	}

//	if(aux_devolver_pagina!=NULL){
	log_info(log_memoria, "[LIBERAR] Por liberar aux_devolver_pagina");
	free(aux_devolver_pagina);
	log_info(log_memoria, "[LIBERAR] aux_devolver_pagina liberado");
//	}

//	if(aux_segmento!=NULL){
	log_info(log_memoria, "[LIBERAR] Por liberar aux_segmento");
	free(aux_segmento);
	log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
//	}
//	if(aux_tabla_paginas!=NULL){
	log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas");
	free(aux_tabla_paginas);
	log_info(log_memoria, "[LIBERAR] aux_tabla_paginas");
//		}
//	if(aux_tabla_paginas2!=NULL){
	log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas2");
	free(aux_tabla_paginas2);
	log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
//	}

//	if(memoriaArmada==1){
	log_info(log_memoria, "[LIBERAR] Por liberar memoria");
	free(bloque_memoria);
	log_info(log_memoria, "[LIBERAR] memoria Liberada");
//	}
	log_info(log_memoria, "[LIBERAR] Por liberar BItmap");
	bitmapDestruir(bitmap);
	log_info(log_memoria, "[LIBERAR] BITMAP destruido");
	log_info(log_memoria, "[LIBERAR] Por liberar Struct configuracion");
	free(arc_config);
	log_info(log_memoria, "[LIBERAR] Struct configuracion Liberada");
	if (log_memoria != NULL) {
		log_info(log_memoria, "[LIBERAR] Liberando log_memoria");
		log_info(log_memoria,
				">>>>>>>>>>>>>>>FIN DE PROCESO MEMORIA<<<<<<<<<<<<<<<");
		log_destroy(log_memoria);
		log_memoria = NULL;
	}
}

void inicioLogYConfig() {
	memoriaArmada = 0;
	tamanioPredefinidoParaNombreTabla = 50;
	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
	log_info(log_memoria,
			" \n ========== Iniciación de Pool de Memoria ========== \n \n ");

	log_info(log_memoria, "[LOGYCONFIG](1) LOG CREADO. ");

	cargarConfiguracion();
	log_info(log_memoria,
			"[LOGYCONFIG] *** CONFIGURACIÓN DE MEMORIA CARGADA. *** ");
}

/*-----------------------------------------------------------------------------
 * MEMORIA PRINCIPAL
 *-----------------------------------------------------------------------------*/

void armarMemoriaPrincipal() {
	tablaPaginaArmada = 0;
	memoriaArmada = 0;
	log_info(log_memoria,
			"[ARMAR MEMORIA] Armo el bloque de memoria, guardo su tamaño");

	printf("HACIENDO MEMORIA");

//	bloque_memoria = malloc(arc_config->tam_mem);
	bloque_memoria = malloc(arc_config->tam_mem);

	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria contigua");
	if (bloque_memoria == NULL) {
		log_info(log_memoria, "[ARMAR MEMORIA] NO se ha creado la memoria");
		liberar_todo_por_cierre_de_modulo();
		abortarProcesoPorUnErrorImportante(log_memoria,
				"[ARMAR MEMORIA] NO se ha guardado correctamente el tamaño a memoria");
	}
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria");

	log_info(log_memoria, "[ARMAR MEMORIA] Guardo tamaño de memoria: %d",
			sizeof(int) * arc_config->tam_mem);

	//ESTO ES PARA SABER CUANTA MEMORIA REAL TIENE DISPONIBLE MEMORIA SIN CONTAR LO ADMINISTRATIVO
	//USANDO EL SIZEOF(MEMORIA) QUEDO PARADO EN LA BASE DONDE COMENZARA A HABER PAGINAS SIENDO
	//ESTA LA POSICION 0 o 1
	int tamanioMemoria = arc_config->tam_mem;

	//CON ESTO CONOCENERE CUANTO DEBERIA MOVERME POR BYTE PARA LLEGAR A UNA POSICION DE PAGINA

	//AGREGO EL MAX VALUE UNICAMENTE PORQUE EL CHAR* YA CUENTA 1 BYTE, Y EL MAX VALUE EL TAMAÑO REAL
	//EN BYTE QUE TENDRA EL VALOR EN CHAR* POR LO TANTO EL 1 ORIGINAL PASA A REPRESENTAR EL \0 QUE ES 1
	//BYTE MAS

	//HARDCODEADO PERO BUENO, NO SE QUE LE PASA AL CONFIG DE FILESYSTEM

	int tamanioPagina = sizeof(pagina) + arc_config->max_value_key; //+1 para que incluya el '\0'


	cantPaginasDisponibles = tamanioMemoria/tamanioPagina;
	cantPaginasDisponibles = 4;

//memoria->paginasTotales = cantPaginasDisponibles;
	cantPaginasTotales = cantPaginasDisponibles;
	//AQUI SE HAN INICIALIZADO LOS SEMAFOROS DE PAGINAS DISPONIBLES

	log_info(log_memoria,
			"[ARMAR MEMORIA] Tamaño de memoria guardada, MEMORIA REAL: %d",
			tamanioMemoria);

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de pagina: %d",
			tamanioPagina);

	log_info(log_memoria,
			"[ARMAR MEMORIA] Cantidad maxima de paginas en memoria: %d",
			cantPaginasDisponibles);

	bitmap = bitmapCrear(cantPaginasTotales);
	log_info(log_memoria, "[ARMAR MEMORIA] BITMAP creado con tamaño de %d",
			cantPaginasDisponibles);

	bloque_LRU = malloc(cantPaginasTotales*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla));
	log_info(log_memoria, "[ARMAR MEMORIA] bloque_LRU para administrar el LRU CREADO");
	//log_info(log_memoria, "[ARMAR MEMORIA] Procedo a guardar los datos administrativos de memoria en el bloque de memoria");

	//memcpy(0, sizeof(memoria_principal), memoria);

	imprimirVerde(log_memoria,
			"[ARMAR MEMORIA] Memoria inicializada de forma correcta");

	printf("MEMORIA TERMINADA");

	//PONGO ESTOS SEMAFOROS LISTOS PARA EMPEZAR A OPERAR
	memoriaArmada = 1;
	limpiandoMemoria = 0;
}

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

//	iniciarSemaforosRetados();

	log_info(log_memoria, "[SEMAFOROS] Semaforos y mutex inicializados");
	log_info(log_memoria,
			"[SEMAFOROS] Semaforo paginasSinUsar iniciada con valor '%d'",
			cantPaginasTotales);
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
	t_config* configFile;
//	configFile = config_create(PATH_MEMORIA_CONFIG);
	if (configFile == ERROR) {
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

void crearConexionesConOtrosProcesos() {

	log_info(log_memoria, "[HILOS] (+)");

	pthread_t hiloClienteLFS;
	pthread_create(&hiloClienteLFS, NULL,
			(void*) conectarConServidorLisandraFileSystem, NULL);

	pthread_detach(hiloClienteLFS);
	log_info(log_memoria, "[HILOS] LANZADO CLIENTE LFS");

	pthread_t hiloServidorKernel;
	pthread_create(&hiloServidorKernel, NULL, (void*) levantarServidor, NULL);

	pthread_detach(hiloServidorKernel);

	log_info(log_memoria, "[HILOS] LANZADO  SERVIDOR KERNEL");

	conectarConServidorLisandraFileSystem();
	levantarServidor();
	log_info(log_memoria, "[HILOS] (-)");
	while (1)
		;
}

void conectarConServidorLisandraFileSystem() {
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
	char* ipLFS = "127.0.0.1";
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

		/*

		 recibiendoMensaje = socketRecibir(sockeConexionLF, buffer, 13,  log_memoria);

		 if(resultado_sendMsj == ERROR){
		 imprimirError(log_memoria, "Error al recibir mensaje de LSF. salimos");
		 return;
		 }


		 imprimirVerde1(log_memoria,"Se ha recibido un mensaje de LISANDRA\n\nMENSAJE RECIBIDO: %s", buffer);
		 */

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

		/*char* msj = malloc(10*sizeof(char));
		 msj = "PruebaK\n";
		 resultado_sendMsj = socketEnviar(socket_CMemoria,msj,strlen(msj),log_kernel);
		 if(resultado_sendMsj == ERROR){
		 log_error(log_kernel,"Error al enviar mensaje a memoria. Salimos");
		 return;
		 }
		 log_info(log_kernel,"El mensaje se envio correctamente");*/
	}
}

void levantarServidor() {

	str_com_t string;

	msg_com_t recibido;

	// SOCKET
	socketEscuchaKernel = nuevoSocket(log_memoria);  // CREAR SOCKET
	if (socketEscuchaKernel == ERROR) {                // CASO DE ERROR.
		log_error(log_memoria,
				" ¡¡¡ ERROR AL CREAR SOCKET. SE TERMINA EL PROCESO. !!! ");
		return;
	}
	log_info(log_memoria, "SOCKET CREADO.Valor: %d.", socketEscuchaKernel);

	// PUERTO
	log_info(log_memoria, " *** SE VA A ASOCIAR SOCKET CON PUERTO ... *** ");
	log_info(log_memoria, "PUERTO A USAR: %d.", arc_config->puerto);

	// ASOCIAR "SOCKET" CON "PUERTO".
	asociarSocket(socketEscuchaKernel     // SOCKET
			, arc_config->puerto      // PUERTO
			, log_memoria); // LOG
	log_info(log_memoria, " *** PUERTO ASOCIADO A SOCKET EXITOSAMENTE. *** ");

	// ESCUCHAR
	socketEscuchar(socketEscuchaKernel    // SOCKET
			, 10, log_memoria); // LOG
	while (1) {
		log_info(log_memoria, " +++ esperando conexiones... +++ ");
		conexionEntrante = aceptarConexionSocket(socketEscuchaKernel,
				log_memoria);
		if (conexionEntrante == ERROR) {
			log_error(log_memoria, "ERROR AL CONECTAR.");
			return;
		}
		/*buffer = malloc(sizeof(t_header));
		 recibiendoMensaje = socketRecibir(conexionEntrante, buffer,
		 sizeof(t_header), log_memoria);

		 printf("Recibimos por socket el comando: %d\n", buffer->comando);
		 log_info(log_memoria, "El mensaje que se recibio fue con el comando %d",
		 buffer->comando);

		 printf("Recibimos por socket el tamanio que vendra en el body: %d\n",
		 buffer->tamanio);
		 log_info(log_memoria,
		 "Recibimos un tamanio que vendra en el body de: %d",
		 buffer->tamanio);

		 printf(
		 "Recibimos por socket la cantidad de argumentos que vendran en el body: %d\n",
		 buffer->cantArgumentos);
		 log_info(log_memoria,
		 "Recibimos la cantidad de argumentos que vendran en el body de: %d",
		 buffer->cantArgumentos);

		 log_info(log_memoria,
		 "El valor de retorno de la funcion que recibio el mensaje fue: %d",
		 recibiendoMensaje);
		 log_info(log_memoria, "El tamanio de la estructura t_header es: %d",
		 sizeof(t_header));
		 if (recibiendoMensaje == sizeof(t_header)) {

		 log_info(log_memoria,
		 "Por enviar confirmacion a Kernel de que recibimos correctamente");

		 log_info(log_memoria,
		 "El tamanio de la confirmacion que enviamos es de: %d",
		 sizeof(recibiendoMensaje));
		 int resultadoEnvio = socketEnviar(conexionEntrante,
		 &recibiendoMensaje, sizeof(recibiendoMensaje), log_memoria);

		 log_info(log_memoria,
		 "Por hacer un malloc de: %d para guardar el body. ",
		 buffer->tamanio);
		 argumentosComando = malloc(buffer->tamanio);

		 memset(argumentosComando, '\0', buffer->tamanio);

		 recibiendoMensaje = socketRecibir(conexionEntrante,
		 argumentosComando, buffer->tamanio, log_memoria);

		 log_info(log_memoria, "Recibimos el/los argumentos: %s",
		 argumentosComando);
		 printf("Recibimos el/los argumentos: %s \n", argumentosComando);

		 log_info(log_memoria, "Por parsear los argumentos.");

		 argumentosParseados = string_split(argumentosComando, SEPARADOR);

		 for (int i = 0; argumentosParseados[i] != NULL; i++) {

		 log_info(log_memoria,
		 "Parseando queda en la posicion %i: el valor: %s", i,
		 argumentosParseados[i]);
		 printf("Parseando queda en la posicion %i: el valor: %s \n", i,
		 argumentosParseados[i]);

		 }*/

		/*
		log_info(log_memoria, "Por llamar a recibir mensaje");
		recibido = recibir_mensaje(conexionEntrante);

		printf("\n\n***Me llego un request***");
		log_info(log_memoria, "***Me llego un request***");

		string = procesar_request(recibido);
		borrar_mensaje(recibido);
		printf("\n\nRequest: %s", string.str);
		log_info(log_memoria, "Request: %s", string.str);
*/

		log_info(log_memoria, "Fin de parseo");
		printf("Fin de parseo. \n");

//	}
	}
}

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

void ejecutarHiloConsola() {
	log_info(log_memoria, "[HILO CONSOLA]Inicializando HILO CONSOLA");

	hiloCrear(&hiloConsolaMemoria, (void*)consola_prueba, NULL);
//	pthread_create(&hiloConsolaMemoria, NULL, consola_prueba(), NULL);
	log_info(log_memoria, "[HILO CONSOLA]Se crea HILO CONSOLA");
	//DUDAS RESPECTO A ESTE HILO, SI PONGO ESTO EMPIEZA A EJECUTAR Y NO PERMITIRA QUE OTROS ENTREN O QUE?
	pthread_join(hiloConsolaMemoria, NULL);
	log_info(log_memoria, "[HILO CONSOLA]HILO CONSOLA en ejecucion");
}

char* lectura_consola() {
char* linea = (char*) readline(">");
return linea;
}

void consola() {

log_info(log_memoria, "En el hilo de consola");

menu();

char bufferComando[MAXSIZE_COMANDO];
char **comandoSeparado;
char **comandoSeparado2;
char *separador2 = "\n";
char *separator = " ";
int comando;

while (1) {

	printf(">");

	fgets(bufferComando, MAXSIZE_COMANDO, stdin);

	add_history(linea);

	free(linea);

	//comandoSeparado = string_split(bufferComando, separador2);
	comandoSeparado = string_split(bufferComando, separator);

	//Tamanio del array

	for (int i = 0; comandoSeparado[i] != NULL; i++) {

		log_info(log_memoria, "En la posicion %d del array esta el valor %s", i,
				comandoSeparado[i]);

		tamanio = i + 1;
	}

	log_info(log_memoria, "El tamanio del vector de comandos es: %d", tamanio);

	switch (tamanio) {

	case 1: {
		comandoSeparado = string_split(bufferComando, separador2);
		log_info(log_memoria, "%s", comandoSeparado[0]);
		log_info(log_memoria, "%d", strcmp(comandoSeparado[0], "salir"));
		if (strcmp(comandoSeparado[0], "salir") == 0) {

			printf("Salir seleccionado\n");
			log_info(log_memoria, "Se selecciono Salir");

			return;
		} else {
			printf("Comando mal ingresado. \n");
			log_error(log_memoria,
					"Opcion mal ingresada por teclado en la consola");

			break;
		}
	}
	case 2:
		validarComando(comandoSeparado[0], log_memoria);
		break;
	case 3:
		validarComando(comandoSeparado[0], log_memoria);
		break;
	default: {
		printf("Comando mal ingresado. \n");
		log_error(log_memoria,
				"Opcion mal ingresada por teclado en la consola");
	}
		break;
	}
	/*
	 //comando = validacionComando(comandoSeparado[0],log_kernel);

	 //log_info(log_kernel, "El numero de comando ingresado es %d", comando);



	 //INTERPRETACION DE COMANDOS ANTERIORES
	 //}

	 /*switch(comandoIngresado){

	 case 1:
	 log_info(log_kernel, "select");
	 break;
	 case 2:
	 log_info(log_kernel, "insert");
	 break;
	 default:
	 printf("Comando mal ingresado. \n");
	 log_error(log_kernel,"Opcion mal ingresada por teclado en la consola");
	 break;
	 }

	 if(strcmp(comandoSeparado[0],"select") == 0){

	 printf("Se selecciono Select\n");

	 log_info(log_kernel,"Por llamar a enviarResultado");

	 int resultadoEnviarComando = enviarComando(comandoSeparado[0],log_kernel);
	 //break;
	 }
	 if(strcmp(comandoSeparado[0],"insert") == 0){

	 printf("Insert seleccionado\n");
	 break;
	 }

	 if(strcmp(comandoSeparado[0],"create") == 0){
	 printf("Create seleccionado\n");
	 break;
	 }

	 if(strcmp(comandoSeparado[0],"describe") == 0){
	 printf("Describe seleccionado\n");
	 break;
	 }
	 if(strcmp(comandoSeparado[0],"drop") == 0){
	 printf("Drop seleccionado\n");
	 break;
	 }
	 if(strcmp(comandoSeparado[0],"journal") == 0){
	 printf("Journal seleccionado\n");
	 break;
	 }
	 if(strcmp(comandoSeparado[0],"add") == 0){
	 printf("add seleccionado\n");
	 break;
	 }

	 if(strcmp(comandoSeparado[0],"run") == 0){
	 printf("Run seleccionado\n");
	 break;
	 }

	 if(strcmp(comandoSeparado[0],"salir") == 0){
	 break;
	 }
	 printf("Comando mal ingresado. \n");
	 log_error(log_kernel,"Opcion mal ingresada por teclado en la consola");
	 */

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
		"SALIR \n"
		"\n");

}

void validarComando(char* comando, t_log* logger) {

int resultadoComando = buscarComando(comando, logger);

switch (resultadoComando) {

case Select: {
	printf("Se selecciono Select\n");

	log_info(logger, "Por llamar a enviarResultado");

	int resultadoEnviarComando = enviarComando(comando, logger);

}
	break;

case insert: {
	printf("Insert seleccionado\n");
	log_info(logger, "Se selecciono insert");

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

}
	break;

case drop: {
	printf("Drop seleccionado\n");
	log_info(logger, "Se selecciono Drop");

}
	break;

case journal: {
	printf("Journal seleccionado\n");
	log_info(logger, "Se selecciono Journal");

}
	break;

default: {
	printf("Comando mal ingresado. \n");
	log_error(logger, "Opcion mal ingresada por teclado en la consola");
}
	break;

}
}

int enviarComando(char** comando, t_log* logger) {

log_info(logger, "En funcion enviarComando");

char* msj = malloc(7 * sizeof(char));

msj = comando;

log_info(logger, "El mensaje que vamos a enviar es: %s", msj);

//AQUI TENGO UNA DUDA, LA CONEXION:

//LA CONEXION SE INICIA EN
//conectarConServidorLisandraFileSystem
//POR LO TANTO YA TENGO EL INT DE ESO ASI QUE NO TENGO QUE VOLVER A
//INICIARLA
//	sockeConexionLF = conexionKernel();

log_info("Vamos a enviar a memoria por el socket %d", sockeConexionLF);

resultado_sendMsj = socketEnviar(sockeConexionLF, msj, strlen(msj),
		log_memoria);

if (resultado_sendMsj == ERROR) {

	log_error(log_memoria, "Error al enviar mensaje a memoria. Salimos");

	return ERROR;
}

log_info(log_memoria, "El mensaje se envio correctamente: %s", msj);

return 0;

}

int buscarComando(char* comando, t_log* logger) {

log_info(logger, "Recibimos el comando: %s", comando);

int i = 0;

//while (i < salir && strcmp(comandosPermitidos[i], comando)) {

//i++;

//}

for (i = 0; i <= salir && strcmp(comandosPermitidos[i], comando); i++) {

}

log_info(logger, "Se devuelve el valor %d", i);

return i;

}

/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CARGA DE LA CONFIGURACION Y EL LOG
 *-----------------------------------------------------------------------------*/
void cargarConfiguracion() {

log_info(log_memoria, "[CONFIGURANDO MODULO] RESERVAR MEMORIA.");
arc_config = malloc(sizeof(t_memoria_config));

log_info(log_memoria, "[CONFIGURANDO MODULO] BUSCANDO CONFIGURACION.");

configFile = config_create(PATH_MEMORIA_CONFIG);

if (configFile != NULL) {

	log_info(log_memoria, "[CONFIGURANDO MODULO] LEYENDO CONFIGURACION...");

	if (config_has_property(configFile, "PUERTO")) {

		arc_config->puerto = config_get_int_value(configFile, "PUERTO");
		log_info(log_memoria, "PUERTO PARA MODULO MEMORIA: %d",
				arc_config->puerto);

	} else {
		log_error(log_memoria, "[ERROR] NO HAY PUERTO CONFIGURADO");
	} // PUERTO

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
		log_info(log_memoria, "[CONFIGURANDO MODULO] IP DE SEEDS: %s",
				arc_config->ip_fs);

	} else {
		log_error(log_memoria, "[ERROR] NO HAY IPS PARA SEEDS");
	} // IP SEEDS

	if (config_has_property(configFile, "PUERTO_SEEDS")) {
		arc_config->puerto_seeds = config_get_array_value(configFile,
				"PUERTO_SEEDS");
		log_info(log_memoria, "[CONFIGURANDO MODULO] PUERTOS PARA SEEDS: %d",
				arc_config->puerto_seeds);

	} else {
		log_error(log_memoria,
				"[ERROR] NO SE ENCONTRARON LOS PUERTOS DE SEEDS");
	} // PUERTOS SEEDS

	if (config_has_property(configFile, "RETARDO_MEM")) {

		arc_config->retardo_mem = config_get_int_value(configFile,
				"RETARDO_MEM");
		log_info(log_memoria, "[CONFIGURANDO MODULO] RETARTDO MEMORIA: %d",
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
				"RETARDO_MEM");
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
				arc_config->retardo_mem);

	} else {
		log_error(log_memoria, "[ERROR] NO HAY NUMERO DE MEMORIA CONFIGURADO");
	} // MEMORY NUMBER

} else {

	log_error(log_memoria,
			"[WARNING] NO HAY ARCHIVO DE CONFIGURACION DE MODULO MEMORIA"); // ERROR: SIN ARCHIVO CONFIGURACION

}

}

/*-----------------------------------------------------
 * FUNCIONES DE JOURNAL
 *-----------------------------------------------------*/

void JOURNAL() {
	log_info(log_memoria, "[JOURNAL] EN JOURNAL");
	char* datosAPasar;
	log_info(log_memoria, "[JOURNAL] PROCEDO A ENVIAR LA INFORAMCION A LISANDRA");

	log_info(log_memoria, "[JOURNAL] ENVIO LA CANTIDAD EXACTA DE CARACTERES QUE LE VOY A ENVIAR");
	pasarValoresALisandra(datosAPasar);
	log_info(log_memoria, "[JOURNAL] TAMAÑO ENVIADO");

	log_info(log_memoria, "[JOURNAL] Lisandra responde que se puede enviar todo, procedo a hacerlo");

	log_info(log_memoria, "[JOURNAL] JOURNAL HECHO, LISANDRA LA HA RECIBIDO BIEN");
}

//PROTOTIPO
int pasarValoresALisandra(char* datos){
	retardo_fs(arc_config->retardo_fs);
}
/*-----------------------------------------------------
 * FUNCIONES PARA LA ADMINISTRACION DE MEMORIA
 *-----------------------------------------------------*/

