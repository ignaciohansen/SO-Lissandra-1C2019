/*
 * gestionMemoria.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#include "gestionMemoria.h"
#include "memoria.h"
#include <string.h>
#include "../Biblioteca/src/Biblioteca.h"
#include "estructuras.h"

/* VARIABLES GLOBALES */
/*
 * ¿Por qué acá y no en él .h?
 * Al ser variables globales de las que depende básicamente toda la memoria
 * me parece "peligroso" ponerlas accesibles desde cualquier lado. En momentos
 * de crisis podemos tocarlas sin pensar demasiado y romper todo.
 * */

int tablaPaginaArmada;
short memoriaArmada;
int tamanio, limpiandoMemoria;


/* FUNCIONES EXTERNAS */

void armarMemoriaPrincipal() {
	/* NO SÉ SI ESTO SE USA
	 * DE USARSE TIENE SENTIDO QUE ESTÉ EN ESTA FUNCIÓN,NO?
	 * */
	rwLockIniciar(&sem_insert_select);

	aux_crear_pagina = malloc(sizeof(pagina));
	aux_devolver_pagina = malloc(sizeof(pagina_a_devolver));
	aux_segmento = malloc(sizeof(segmento));
	aux_tabla_paginas = malloc(sizeof(pagina_referenciada));
	aux_tabla_paginas2 = malloc(sizeof(pagina_referenciada));

	log_info(log_memoria,"[ARMAR MEMORIA] Alocando el bloque de memoria de tamaño %d",arc_config->tam_mem);

//	printf("HACIENDO MEMORIA");

	bloque_memoria = malloc(arc_config->tam_mem);

	if (bloque_memoria == NULL) {
		log_info(log_memoria, "[ARMAR MEMORIA] NO se ha creado la memoria");
		liberar_todo_por_cierre_de_modulo();
		abortarProcesoPorUnErrorImportante(log_memoria,
				"[ARMAR MEMORIA] NO se ha guardado correctamente el tamaño a memoria");
	}
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria contigua");

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
//	cantPaginasDisponibles = 3;

//memoria->paginasTotales = cantPaginasDisponibles;
	cantPaginasTotales = cantPaginasDisponibles;
	//AQUI SE HAN INICIALIZADO LOS SEMAFOROS DE PAGINAS DISPONIBLES

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de pagina: %d",tamanioPagina);

	log_info(log_memoria,"[ARMAR MEMORIA] Cantidad maxima de paginas en memoria: %d",cantPaginasDisponibles);

	bitmap = bitmapCrear(cantPaginasTotales);

	log_info(log_memoria, "[ARMAR MEMORIA] BITMAP creado con tamaño de %d",	cantPaginasDisponibles);

	bloque_LRU = malloc(cantPaginasTotales*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla));
//	log_info(log_memoria, "[ARMAR MEMORIA] bloque_LRU para administrar el LRU CREADO");
	//log_info(log_memoria, "[ARMAR MEMORIA] Procedo a guardar los datos administrativos de memoria en el bloque de memoria");

	//memcpy(0, sizeof(memoria_principal), memoria);

	imprimirVerde(log_memoria,"[ARMAR MEMORIA] Memoria inicializada de forma correcta");

//	printf("MEMORIA TERMINADA");

	//PONGO ESTOS SEMAFOROS LISTOS PARA EMPEZAR A OPERAR
	memoriaArmada = 1;
	limpiandoMemoria = 0;
}

int loggearEstadoActual(FILE* fp)
{
	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[LOGGEANDO ESTADO ACTUAL MEMORIA] No se ha inicializado la tabla de segmentos");
		return -1;
	}
	segmento *aux_segmento = tablaSegmentos;
	pagina_referenciada *aux_pagina;
	int nro_pagina;
	pagina_a_devolver *algo;


	log_info(log_memoria, "[LOGGEANDO ESTADO ACTUAL MEMORIA]");
	fprintf(fp,"\n\n****************ESTADO DE TABLAS****************\n");
	while(aux_segmento != NULL){
		fprintf(fp,"\n\nTABLA %s",aux_segmento->path_tabla);
		aux_pagina = aux_segmento->paginasAsocida;
		fprintf(fp, "\n%10s;%30s;%20s", "key","value","timestamp");
		while(aux_pagina != NULL){
			nro_pagina = aux_pagina->nropagina;
			algo = selectPaginaPorPosicion(nro_pagina,false);
//			log_info(logger, "%d;%s;%lf", algo->key, algo->value, algo->timestamp);
			fprintf(fp, "\n%10d;%30s;%20llu", algo->key, algo->value, algo->timestamp);
			aux_pagina = aux_pagina->sig;
			free(algo->value);
			free(algo);
		}
		fprintf(fp,"\n\n");
		aux_segmento = aux_segmento->siguienteSegmento;
	}

	return 1;
}


int funcionInsert(char* nombreTabla, u_int16_t keyBuscada, char* valorAPoner, bool estadoAPoner, timestamp_mem_t timestamp_val){
//	log_info(log_memoria, "[INSERT] En funcion INSERT");
	if(strlen(valorAPoner)>=max_valor_key){
		log_error(log_memoria, "[INSERT] El valor '%s' excede el limite para los values: %d",arc_config->max_value_key);
		return -1;
	}


	//ESTO ES PARA BLOQUEAR CUALQUIER INSERT NUEVO CUANDO SE ESTA REALIZANDO LRU O JOURNAL
	//ADEMAS, SI YA SE ESTA REALIZANDO 1 INSERT ENTONCES

	//mutexBloquear(&mutex_bloque_LRU_modificando);
	mutexBloquear(&ACCIONLRU);
	mutexDesbloquear(&ACCIONLRU);

	rwLockEscribir(&sem_insert_select);

	segmento* segmentoBuscado = NULL;
	pagina_referenciada* ref = malloc(sizeof(pagina_referenciada));
//	log_info(log_memoria, "[INSERT] Me pongo a buscar el segmento y la tabla en base a '%s' y '%d'",nombreTabla, keyBuscada);
	mutexBloquear(&JOURNALHecho);
	mutexDesbloquear(&JOURNALHecho);

	int posicionAIr = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombreTabla, keyBuscada,&segmentoBuscado);

	if(posicionAIr==-1){
//		log_info(log_memoria, "[INSERT] Verifico si debo realizar LRU");
		mutexBloquear(&verificarSiBitmapLleno);
		if(bitmapLleno()){
//			log_info(log_memoria, "[INSERT] Debo realizar un LRU");
			LRU();
			posicionAIr = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombreTabla, keyBuscada,&segmentoBuscado);
		}
		mutexDesbloquear(&verificarSiBitmapLleno);
		log_info(log_memoria,"[INSERT] Hay marcos libres, no es necesario ejecutar LRU");
	}

	if(posicionAIr==-1){
		/*log_info(log_memoria, "[INSERT] Verifico si debo realizar LRU");
		mutexBloquear(&verificarSiBitmapLleno);
		if(bitmapLleno()){
			log_info(log_memoria, "[INSERT] Debo realizar un LRU");
			LRU();
		}
		mutexDesbloquear(&verificarSiBitmapLleno);*/
//		log_info(log_memoria, "[INSERT] NO se encontro la posicion a donde debo ir");
//		log_info(log_memoria, "[INSERT] Verifico 1* si esta FULL memoria");

		//CASO B, verifico si se encontro el segmento, caso contrario debo tambien crearlo
		pthread_mutex_lock(&mutex_bloquear_select_por_limpieza);
		if(segmentoBuscado==NULL){
			log_info(log_memoria, "[INSERT] No se encontro un segmento asociado a la tabla '%s'",nombreTabla);
			insertCrearPaginaConNuevoSegmento(nombreTabla, keyBuscada,
					ref, valorAPoner, estadoAPoner,
					segmentoBuscado,  timestamp_val);
		} else {
			log_info(log_memoria, "[INSERT] Se encontro un segmento asociado a la tabla '%s'",segmentoBuscado->path_tabla);
			//EXISTE EL SEGMENTO, SOLO CREO LA TABLA Y LA PAGINA Y SE LA ASIGNO
			//A LA COLA DE TABLA DE PAGINAS DE SEGMENTO

			tabla_pagina_crear(keyBuscada, valorAPoner, estadoAPoner,
					&ref, nombreTabla, true, segmentoBuscado, timestamp_val);

			if(ref->sig == NULL){
				log_info(log_memoria,"[DBG] OKAAAA");
			}else{
				log_info(log_memoria,"[DBG] mallllllllllllllll");
			}

			if(estadoAPoner) {
//				printf("[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",ref->nropagina, keyBuscada);
				log_info(log_memoria,"[INSERT] Pagina creada con NROPAGINA|KEY|FLAG: %d|%d|TRUE",ref->nropagina, keyBuscada);
						} else {
//				printf("[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|TRUE",ref->nropagina, keyBuscada);
				log_info(log_memoria,"[INSERT] Pagina creada con NROPAGINA|KEY|FLAG: %d|%d|FALSE",
					ref->nropagina, keyBuscada);
				}
		//	printf("\n\n\nPASAMOS INSERT\n\n");
//			log_info(log_memoria,"[DBG] 1");
			if(segmentoBuscado == NULL)
				log_info(log_memoria,"[DBG] Segmento buscado es null");
			pagina_referenciada* ref3 = segmentoBuscado->paginasAsocida;
			if(ref->sig == NULL){
				log_info(log_memoria,"[DBG] OKAAAA");
			}else{
				log_info(log_memoria,"[DBG] mallllllllllllllll");
			}
			if(ref3 == NULL)
				log_info(log_memoria,"[DBG] ref3 es null");
//			log_info(log_memoria,"[DBG] Antes del while");
	//		printf("\n\n\n%d\n\n", ref3->nropagina);
			while(ref3->sig!=NULL){
				ref3=ref3->sig;
		//		printf("\n\n\n%d\n\n", ref3->nropagina);
			}
//			log_info(log_memoria,"[DBG] Despues del while");
//			ref3->sig = ref;

		//	printf("\n\n\nPASAMOS INSERT\n\n");
		//	ref->sig = segmentoBuscado->paginasAsocida->sig;
		//	memcpy(segmentoBuscado->paginasAsocida->sig, ref->sig, sizeof(*pagina_referenciada));
		//	memcpy(segmentoBuscado->paginasAsocida, ref, sizeof(pagina_referenciada));
		//	segmentoBuscado->paginasAsocida = ref;
	//		nroTablaCreada = tabla_pagina_crear(keyBuscada, valorAPoner, true);
//			log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina. Nro de su posicion es aux y nueva '%d'/'%d'",
//					ref->nropagina, ref3->sig->nropagina);
	//		segmentoAgregarNroTabla(&segmentoBuscado, nroTablaCreada);
			log_info(log_memoria, "[INSERT] Pagina agregada al segmento '%s' en la posicion %d", nombreTabla,ref->nropagina);

//			log_info(log_memoria, "[INSERT HECHO] Compruebo si se añadio la nueva tabla de paginas de '%s'", nombreTabla);
//			pagina_referenciada* ref2 = segmentoBuscado->paginasAsocida;
//			int i = 0;
//			while(ref2!=NULL){
//				i++;
////				log_info(log_memoria, "\n[INSERT HECHO] Iteracion '%d'\nNro pagina: '%d'", i, ref2->nropagina);
//				ref2 = ref2->sig;
//			}
//	//		free(ref);
		}
		pthread_mutex_unlock(&mutex_bloquear_select_por_limpieza);
	} else {
	/*	printf("[INSERT A MODIFICAR] Existe el segmento '%s' y la pagina que referencia la key (%d) que es NRO '%d'. Procedo a poner el nuevo valor que es '%s'",
				segmentoBuscado->path_tabla, keyBuscada, posicionAIr, valorAPoner);*/
		/*
		 * SE CREO PERFECTAMENTE, CONOSCO EL SEGMENTO Y LA PAGINA A REFERENCIAR, PROCEDO A MODIFICAR Y ACCEDER
		 * A LA MEMORIA PARA LA MODIFICACION DE LOS CAMPOS
		 */
		log_info(log_memoria, "[INSERT] Ya hay una pagina para la key %d en el segmento %s",keyBuscada,segmentoBuscado->path_tabla);
		free(ref);
	//	free(segmentoBuscado);
		modificarValoresDeTablaYMemoriaAsociadasAKEY(posicionAIr, valorAPoner, timestamp_val);
	}
//	mutexDesbloquear(&mutex_bloque_LRU_modificando);
	rwLockDesbloquear(&sem_insert_select);
	return 1;
}

int funcionSelect(char* nombreTablaAIr, u_int16_t keyBuscada,
		          pagina_a_devolver** dato, char** valorADevolver)
{
	mutexBloquear(&mutex_bloquear_select_por_limpieza);
	mutexDesbloquear(&mutex_bloquear_select_por_limpieza);

	int direccionPagina;
	segmento *seg;

//	mutexBloquear(&mutex_bloque_LRU_modificando);
//	void* informacion = malloc(sizeof(pagina)+max_valor_key);
//	log_info(log_memoria,"[FUNCION SELECT] ENTRANDO POR NUEVA PETICION. Valor de key de los datos solicitados: SEGMENTO: % s KEY: %d",nombreTablaAIr,keyBuscada);
	direccionPagina = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey
			(nombreTablaAIr, keyBuscada, &seg);

	if(direccionPagina==-1){
		imprimirMensaje2(log_memoria, "[SELECT] No se encontro la pagina para la key %d de la tabla %s en memoria",keyBuscada,	nombreTablaAIr);
//			free(informacion);
			return 0;
	}
//	log_info(log_memoria, "[FUNCION SELECT] Numero de pagina a donde debo ir: %d.Me pongo a buscar los datos", direccionPagina);
	free((*dato)->value);
	free(*dato);
	*dato = selectPaginaPorPosicion(direccionPagina, true);
//	modificar_bloque_LRU(NULL, timestamp(), direccionPagina, true, false);

//	mutexDesbloquear(&mutex_bloque_LRU_modificando);

//	log_info(log_memoria, "[FUNCION SELECT] Se encontro el dato");

	return 1;
}

int funcionDrop(char* nombre) {
	mutexBloquear(&ACCIONLRU);
	segmento* segmentoBuscado;
	segmento* segmentoAnterior = tablaSegmentos;
//	log_info(log_memoria, "[FUNCION DROP] EN FUNCION DROP");

	mutexBloquear(&mutex_segmento_en_modificacion);

//	printf("[FUNCION DROP] aqui\n\n");
	segmentoBuscado = buscarSegmentoPorNombreTabla(nombre);
	if(segmentoBuscado==NULL){
		log_info(log_memoria, "[DROP] En memoria no hay un segmento para la tabla '%s'", nombre);
		//NO HAY NADA ASI QUE DEVUELVO ERROR
		mutexDesbloquear(&ACCIONLRU);
		mutexDesbloquear(&mutex_segmento_en_modificacion);
		return -1;
	}
	if(strcmp(nombre, tablaSegmentos->path_tabla)==0){
//		printf("[FUNCION DROP] Se encontro <%s> y es el primero de todos\n",tablaSegmentos->path_tabla);
		log_info(log_memoria, "[DROP] Se encontro el segmento para la tabla '%s' y sera borrado de memoria",tablaSegmentos->path_tabla);
		tablaSegmentos = tablaSegmentos->siguienteSegmento;
		limpiar_todos_los_elementos_de_1_segmento(segmentoAnterior);
		mutexDesbloquear(&ACCIONLRU);
		mutexDesbloquear(&mutex_segmento_en_modificacion);
		return 1;
	}
//	printf("[FUNCION DROP] Empiezo a buscar el anterior de ese segmento");
//	log_info(log_memoria, "[FUNCION DROP] Empiezo a buscar el anterior de ese segmento");
	while(segmentoAnterior->siguienteSegmento != segmentoBuscado) {
		segmentoAnterior = segmentoAnterior->siguienteSegmento;
	}

	segmentoAnterior->siguienteSegmento=segmentoBuscado->siguienteSegmento;
	log_info(log_memoria, "[DROP] Se encontro el segmento para la tabla '%s' y sera borrado de memoria",segmentoBuscado->path_tabla);
//	log_info(log_memoria, "[FUNCION DROP] Libero todo el segmento: <%s>", segmentoBuscado->path_tabla);
	limpiar_todos_los_elementos_de_1_segmento(segmentoBuscado);
	mutexDesbloquear(&ACCIONLRU);
	mutexDesbloquear(&mutex_segmento_en_modificacion);

	return 1;
}

void liberar_todo_por_cierre_de_modulo() {


	// ESTE TIENE 1 ERROR el de abajo comentado
	//	cerrarTodosLosHilosPendientes();
	rwLockEscribir(&sem_insert_select);
	log_info(log_memoria,"[LIBERAR] Por liberar Segmentos y sus tablas de paginas");
	liberar_todo_segmento();

	//LIBERA LA RAM.
//	log_info(log_memoria,"[LIBERAR] Empiezo a liberar todos los elementos que se han inicializado");
//	if(aux_crear_pagina!=NULL){
	//log_info(log_memoria, "[LIBERAR] Por liberar aux_crear_pagina");
	free(aux_crear_pagina);
	//log_info(log_memoria, "[LIBERAR] aux_crear_pagina liberado");
//	}

//	if(aux_devolver_pagina!=NULL){
	//log_info(log_memoria, "[LIBERAR] Por liberar aux_devolver_pagina");
	free(aux_devolver_pagina);
	//log_info(log_memoria, "[LIBERAR] aux_devolver_pagina liberado");
//	}

//	if(aux_segmento!=NULL){
	//log_info(log_memoria, "[LIBERAR] Por liberar aux_segmento");
	free(aux_segmento);
	//log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
//	}
//	if(aux_tabla_paginas!=NULL){
	//log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas");
	free(aux_tabla_paginas);
	//log_info(log_memoria, "[LIBERAR] aux_tabla_paginas");
//		}
//	if(aux_tabla_paginas2!=NULL){
	//log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas2");
	free(aux_tabla_paginas2);
	//log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
//	}

//	if(memoriaArmada==1){
//	log_info(log_memoria, "[LIBERAR] Por liberar memoria");
	free(bloque_memoria);
	log_info(log_memoria, "[LIBERAR] memoria Liberada");
//	}
//	log_info(log_memoria, "[LIBERAR] Por liberar BItmap");
	bitmapDestruir(bitmap);
	log_info(log_memoria, "[LIBERAR] BITMAP destruido");
//	log_info(log_memoria, "[LIBERAR] Por liberar Struct configuracion");

	liberar_config();

	log_info(log_memoria, "[LIBERAR] Struct configuracion Liberada");
	if (log_memoria != NULL) {
		log_info(log_memoria, "[LIBERAR] Liberando log_memoria");
		log_info(log_memoria,
				">>>>>>>>>>>>>>>FIN DE PROCESO MEMORIA<<<<<<<<<<<<<<<");
		log_destroy(log_memoria);
		log_memoria = NULL;
	}


	if (tablas_fp != NULL){
		fprintf(tablas_fp,"\n\n\n********FINALIZANDO********");
		fprintf(tablas_fp,"\n\n\n");
		fclose(tablas_fp);
	}


}

void liberar_config(void)
{
	log_info(log_memoria, "[LIBERANDO CONFIG]");
	if(arc_config->ip != NULL)
		free(arc_config->ip);
	if(arc_config->ip_fs != NULL)
		free(arc_config->ip_fs);


	if(arc_config->ip_seeds != NULL){
		int i = 0;
		char * aux = arc_config->ip_seeds[0];
		while(aux!=NULL){
			free(aux);
			i++;
			aux = arc_config->ip_seeds[i];
		}
		free(arc_config->ip_seeds);
	}

	if(arc_config->puerto_seeds != NULL){
		int i = 0;
		char * aux = arc_config->puerto_seeds[0];
		while(aux!=NULL){
			free(aux);
			i++;
			aux = arc_config->puerto_seeds[i];
		}
		free(arc_config->puerto_seeds);
	}
	free(arc_config);

	config_destroy(configFile);

}

void* accederYObtenerInfoDePaginaEnPosicion(int posicion, void* info){
//	log_info(log_memoria, "[ACCEDIENDO A DATOS] Por acceder a la memoria a la posicion '%d'", posicion);
//	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	obtenerInfoDePagina(posicion, &info);
//	mutexDesbloquear(&mutex_memoria);
//	log_info(log_memoria, "[ACCEDIENDO A DATOS] Datos obtenidos de la posicion '%d'", posicion);
//	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);



	return info;
}
//
//pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos){
//	unaPagina->nroPosicion= (short)nuevaPos;
//	return unaPagina;
//}

void asignarNuevaPaginaALaPosicion(
		int posLibre, pagina* pagina_nueva, char* valorAPoner,
		bool estadoAsignado, char* nombreTabla){
//	char stringValor[max_valor_key];
//	strcpy(stringValor, valorAPoner);

	mutexBloquear(&mutex_memoria);
	mutexBloquear(&mutex_bitmap);

	int desplazamieto = sizeof(pagina)+max_valor_key;
	//AQUI SE GUARDA   LA PAGINA
	memcpy(bloque_memoria+posLibre*desplazamieto, pagina_nueva, sizeof(pagina));
//	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Pagina guardada");

	//memcpy(bloque_memoria+posLibre*desplazamieto+sizeof(pagina)-1,valorAPoner, max_valor_key);
	memcpy(bloque_memoria+posLibre*desplazamieto+sizeof(pagina)-1,valorAPoner, strlen(valorAPoner)+1);//@martin revisar
//	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] El valor de la pagina fue guardada, actualizo el BITMAP ocupando la posicion '%d'", posLibre);
//	free(stringValor);
	bitmapOcuparBit(bitmap, posLibre);

	log_info(log_memoria, "[ASIGNANDO PAGINA] Se ha asignado exitosamente el marco. Quedan '%d' marcos libres", cantPaginasDisponibles);
	pagina* pagNew = malloc(sizeof(pagina));
	char valorString[max_valor_key];

	memcpy(pagNew, bloque_memoria+posLibre*desplazamieto, sizeof(pagina));
	memcpy(valorString, bloque_memoria+posLibre*desplazamieto+sizeof(pagina)-1, max_valor_key);
//	printf("\n\nNOMBRE QUE DEBO INGRESAR A BLOQUE LRU: %s\n\n\n", nombreTabla);
	timestamp_mem_t a = timestamp();
/*	printf("[TIMESTAMP NUEVO]\nDATOS INGRESADOS:\nTIMESTAMP: <%f>\n\n",
					timestamp);*/
	modificar_bloque_LRU(nombreTabla, a, posLibre, estadoAsignado, true);
/*
	printf("[asignarNuevaTablaAPosicionLibre]POSICION %d\nVALORES EN BLOQUE: KEY|VALUE|TIMESTAMP %d|%s|%f\nVALORES ORIGINALES: KEY|VALUE|TIMESTAMP %d|%s|%f\n",
				pagNew->nroPosicion, pagNew->key, valorString, pagNew->timestamp,
				pagina_nueva->key, valorAPoner,pagina_nueva->timestamp);
*/
	/*
	log_info(log_memoria,"[asignarNuevaTablaAPosicionLibre]POSICION %d\n"
			"VALORES PUESTOS en bl: KEY|VALUE|TIMESTAMP %d|%s|%llu\n"
			"VALORES PUESTOS: KEY|VALUE|TIMESTAMP %d|%s|%llu\n",
			pagNew->nroPosicion, pagNew->key, valorString, pagNew->timestamp,
			pagina_nueva->key, valorAPoner,pagina_nueva->timestamp);
			*/
	log_info(log_memoria,"[ASIGNANDO PAGINA] Marco: %d. Valores en marco: KEY|VALUE|TIMESTAMP %d|%s|%llu",
				pagNew->nroPosicion, pagNew->key, valorString, pagNew->timestamp);
	free(pagNew);
	free(pagina_nueva);
	mutexDesbloquear(&mutex_bitmap);
	mutexDesbloquear(&mutex_memoria);
}

//INTENTA BUSCAR EN QUE POSICION DE LA MEMORIA CONTIGUA SE ENCUNETRA LA PAGINA CON ESA KEY

int buscarEntreTodasLasTablaPaginasLaKey(pagina_referenciada* tablasAsociadasASegmento,
		u_int16_t keyBuscada){

		int i =-1;
	mutexBloquear(&mutex_pagina_referenciada_aux2);
	pagina_referenciada* tablaasociadaaux = tablasAsociadasASegmento;
	pagina_a_devolver* pagina_devolver;
	//	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Por buscar la KEY '%d' en la pagina '%d'",
	//			posicionABuscar, keyBuscada);
//	int posicionDeLaKey = buscarEnMemoriaLaKey(keyBuscada);
//	void* informacion;
	log_info(log_memoria, "[BUSCANDO PAGINA] Se busca key '%d'", keyBuscada);
	int cont=0;
	while(tablaasociadaaux!=NULL){
//		informacion = malloc(sizeof(pagina)+max_valor_key);
		pagina_devolver = selectPaginaPorPosicion(tablaasociadaaux->nropagina, false);

		if(pagina_devolver->key==keyBuscada){
			log_info(log_memoria, "[BUSCANDO PAGINA] Se encontro la key %d en la pagina %d. Marco %d",keyBuscada,cont,tablaasociadaaux->nropagina);
		//	tablaasociadaaux->vecesAccedido +=1;
			i= tablaasociadaaux->nropagina;
			free(pagina_devolver->value);
			free(pagina_devolver);
//			free(informacion);
			mutexDesbloquear(&mutex_memoria);
			mutexDesbloquear(&mutex_pagina_referenciada_aux2);
			return i;
		}
		cont++;
//		free(pagina_devolver);
	//	free(informacion);
//		free(informacion);
		free(pagina_devolver->value);
		free(pagina_devolver);
		mutexDesbloquear(&mutex_memoria);
		tablaasociadaaux = tablaasociadaaux->sig;
	}
	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
	log_info(log_memoria, "[BUSCANDO PAGINA] La key %d no tiene una pagina asignada",keyBuscada);


	return -1;
}

//Trata de buscar entre todos los segmentos la key buscada a partir de la key  y nombre tabla

//1* PARTE PARA BUSCAR UJNA KEY, usada principalmente para select
int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey
	(char* nombreTabla, u_int16_t keyBuscada,
		segmento** segmentoBuscado)
{
	log_info(log_memoria,"[BUSCANDO SEGMENTO] Buscando segmento para la tabla %s",nombreTabla);
	segmento* seg_aux = buscarSegmentoPorNombreTabla(nombreTabla);
	*segmentoBuscado = seg_aux;
	if(seg_aux!=NULL){
		//EXISTE EL SEGMENTO
		log_info(log_memoria,"[BUSCANDO SEGMENTO] Se encontro el segmento para la tabla %s",seg_aux->path_tabla);
		return buscarEntreTodasLasTablaPaginasLaKey(seg_aux->paginasAsocida, keyBuscada);
	}
	//NO EXISTE EL SEGMENTO
	log_info(log_memoria,"[BUSCANDO SEGMENTO] No hay un segmento para la tabla %s",	nombreTabla);
	return -1;
}

//int buscarEnMemoriaLaKey(u_int16_t keyBuscada){
//	pagina* pag = malloc(sizeof(pagina));
//
//	int pos;
//	log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] ENTRO, Me pongo a buscar la key '%d'", keyBuscada);
//	for(pos=0; pos<cantPaginasTotales; pos++){
//		//ACCEDO A LA MEMORIA, HAGO RECORRIDO POR TODA ELLA BUSCANDO LA KEY Y DEVUELVO LA POSICION
//		if(bitmapBitOcupado(bitmap, pos)){
//			memcpy(pag, bloque_memoria+pos*(sizeof(pagina)+max_valor_key), sizeof(pagina));
//			if(pag->key==keyBuscada){
//				free(pag);
//				log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] KEY ENCONTRADA '%d' en la posicion (%d)", keyBuscada, pos);
//				return pos;
//			}
//		}
//	}
//	free(pag);
//	log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] NO se encontro la key '%d', devuelvo -1 y SALIMOS", keyBuscada);
//	//NO ENCONTRO NADA, DEVUELVO ERROR QUE ES -1
//	return -1;
//}

segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[BUSCANDO SEGMENTO] No se ha inicializado la tabla de segmentos, devuelvo NULL");
		return NULL;
	}
	segmento* todoSegmento = tablaSegmentos;
	char* nombreEnSegmento;
//	log_info(log_memoria, "[BUSCANDO SEGMENTO] Buscando el segmento para la tabla '%s'", nombreTabla);
	while(todoSegmento!=NULL){
	//	nombreEnSegmento = obtenerNombreTablaDePath(todoSegmento->path_tabla);
		nombreEnSegmento = todoSegmento->path_tabla;
//		log_info(log_memoria, "[BUSCANDO SEGMENTO] Nombre buscado | NOmbre obtenido '%s'|'%s'",
//				nombreTabla, nombreEnSegmento);

		if(strcmp(nombreTabla, nombreEnSegmento) == 0){
			//SE ENCONTRO EL SEGMENTO BUSCADO
//			log_info(log_memoria, "[BUSCANDO SEGMENTO] Se encontro el segmento para la tabla %s",nombreTabla);
			return todoSegmento;
		}
		todoSegmento = todoSegmento->siguienteSegmento;
	}
//	log_info(log_memoria, "[BUSCANDO SEGMENTO] No se encontro el segmento para la tabla %s",nombreTabla);
	return NULL;
}

int buscarPaginaDisponible(u_int16_t key, bool existiaTabla,
		char* nombreTabla, segmento* segmetnoApuntado){
	int i;
	pagina_a_devolver* pag;
	pagina_referenciada* pagina_referenciada;

	mutexBloquear(&mutex_bitmap);
//	log_info(log_memoria, "[BITMAP] Verifico si tiene espacio libre del total de paginas que son: '%d'",	cantPaginasTotales);
	if(existiaTabla){
//		log_info(log_memoria, "[BITMAP] Busco key [%d] en el segmento '%s'",	key, segmetnoApuntado->path_tabla);
		pagina_referenciada = segmetnoApuntado->paginasAsocida;
		while(pagina_referenciada!=NULL){
			pag = selectObtenerDatos(pagina_referenciada->nropagina, false);
//			log_info(log_memoria, "[BITMAP] Obtuve la pagina de la posicion '%d' y tiene key '%d' y busco la key '%d'",	pagina_referenciada->nropagina, pag->key, key);
			if(pag->key == key){
//				log_info(log_memoria, "[BITMAP] La posicion LIBRE: '%d'",pagina_referenciada->nropagina, key);
				free(pag);
				mutexDesbloquear(&mutex_bitmap);
				return pagina_referenciada->nropagina;
			}
//			log_info(log_memoria, "[BITMAP] La posicion '%d' NO esta  libre, paso al siguiente BITMAP",pagina_referenciada->nropagina, key);
			free(pag);
			pagina_referenciada = pagina_referenciada->sig;
		}

//		log_info(log_memoria, "[BITMAP] No se encontro en el segmento  '%s' la key '%d' buscad. Paso a crearla!",
//				segmetnoApuntado->path_tabla, key);
//		mutexDesbloquear(&mutex_bitmap);
	//	return -1;
	}
	for(i=0;i<cantPaginasTotales;i++){
//		log_info(log_memoria, "[BITMAP] Busco la siguiente posicion libre para la key [%d]", i);
		if(!bitmapBitOcupado(bitmap, i)){
			//esta libre la posicion
//			log_info(log_memoria, "[BITMAP] La posicion '%d' esta libre", i);
			mutexDesbloquear(&mutex_bitmap);
			return i;
		}
	}
	log_info(log_memoria, "[BITMAP] YA NO EXISTEN POSICIONES LIBRE EN EL BITMAP");
	mutexDesbloquear(&mutex_bitmap);
	return -1;
}

int buscarEnBloqueLRUElProximoAQuitar(char** nombreTablaCorrespondienteASacarTablaDePagina) {
	nodoLRU* nodo = malloc(sizeof(nodoLRU));
	int i, potencialCandidato = -1;
	timestamp_t menortimestamp = timestamp();
	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	log_info(log_memoria, "[LRU] ENTRO A LRU, comienzo a buscar candidato");
	for(i=0; i<cantPaginasTotales;i++){
		memcpy(nodo, bloque_LRU+i*desplazamiento, sizeof(nodoLRU));
		if(nodo->estado == false){
			//SIGNIFICA QUE NO FUE MODIFICADO
	//		printf("TIMESTAMP ACTUAL = %f\nTIMESTAMP A COMPARAR: %f\n",
	//				menortimestamp, nodo->timestamp);
			if(menortimestamp > nodo->timestamp){

				menortimestamp = nodo->timestamp;
				potencialCandidato = i;
	//			printf("NUEVO TIMESTAMP MINIMO: %f\n\n", menortimestamp);
			}
		}
	}
	//printf("[ALGORITMO LRU]\nLRU FINALIZADO\nCandidato a quitar elegido fue: <%d>",potencialCandidato);
	if(potencialCandidato<0){
		log_info(log_memoria, "[LRU] No hay marcos que puedan ser reemplazados");
//		*nombreTablaCorrespondienteASacarTablaDePagina=NULL;
	} else {
		log_info(log_memoria, "[LRU] Marco a reemplazar: %d",potencialCandidato);
		memcpy(*nombreTablaCorrespondienteASacarTablaDePagina,
				bloque_LRU+potencialCandidato*desplazamiento+sizeof(nodoLRU),
				tamanioPredefinidoParaNombreTabla-1);
	}
	free(nodo);
	//ESTO SE DA SI NO SE ENCUENTRA LO QUE SE QUIERE BUSCAR
	return potencialCandidato;
}

void borrarSegmentoPasadoPorParametro(segmento* unSegmento){
	segmento* aux_segmentos = tablaSegmentos;
	segmento* aux_aux_segmento = tablaSegmentos;
	log_info(log_memoria, "[BORRANDO SEGMENTO] Borrando el segmento <%s>", unSegmento->path_tabla);
	while(aux_segmentos != unSegmento){
		aux_aux_segmento = aux_segmentos;
		aux_segmentos = aux_segmentos->siguienteSegmento;
		if(aux_segmentos==NULL){
			log_info(log_memoria, "[BORRANDO SEGMENTO] El segmento ya fue BORRADO anteriormente");
			return;
		}
	}
	aux_aux_segmento->siguienteSegmento = aux_segmentos->siguienteSegmento;
//	free(aux_segmentos->path_tabla);
//	free(aux_segmentos);

//	log_info(log_memoria,"[DEBUGGGGG_AUX_AUX] D1: %p, D2: %p, D3: %p",unSegmento,aux_segmentos,aux_aux_segmento);

	log_info(log_memoria, "[BORRANDO SEGMENTO] Segmento <%s> BORRADO", unSegmento->path_tabla);
}



pagina* crear_pagina(int16_t key, char * valor, int posAsignada, timestamp_mem_t timestamp_val) {
	//log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Por crear pagina y valor");
	pagina* pag = malloc(sizeof(pagina));
	pag->nroPosicion = posAsignada;
	pag->key = key;
//	aux_pagina->timestamp = timestamp();
	//SI TIENE TIMESTAMP EN 0 LE ASIGNAMOS EL ACTUAL
	if(timestamp_val == 0)
		pag->timestamp = timestamp();
	else
		pag->timestamp = timestamp_val;

	log_info(log_memoria, "[CREANDO PAGINA] Pagina creada: KEY|VALOR|TIMESTAMP: %d|%s|%llu", key, valor, pag->timestamp);
	//log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Pagina creada y tambien su valor");
	return pag;
}

void actualizarTiempoUltimoAcceso(int pos, bool estadoAPoner, bool vieneDeInsert){
//	log_info(log_memoria, "[LRU] Actualizando el times Entrando, modifico en tabla LRU el timestamp de '%d'", pos);

	modificar_bloque_LRU(NULL, timestamp(), pos, estadoAPoner, vieneDeInsert);

	return;
}

//ESTE, CADA VEZ QUE SE LO INVOCA SE DEBE PONER UN SEMAFORO
void obtenerInfoDePagina(int i, void** informacion){
//	log_info(log_memoria, "[OBTENIENDO DATOS] Empiezo a obtener datos de pagina '%d'", i);

	mutexBloquear(&mutex_memoria);
	memcpy(*informacion, bloque_memoria+i*(sizeof(pagina)+max_valor_key), sizeof(pagina)+max_valor_key);
//	log_info(log_memoria, "[OBTENIENDO DATOS] Obtuve datos de pagina '%d'", i);
	return;
}

char* obtenerNombreTablaDePath(char* path){
	int i;
	int posUltimoBarra=0;
//	log_info(log_memoria, "[Obteniendo nombre de la tabla] ENTRANDO");
	for(i=0; i < stringLongitud(path); i++){
		//ME PONGO A BUSCAR EL ULTIMO /
		if( path[i] == '/'){
			posUltimoBarra=i+1;
		}
	}
	//YA OBTUVE LA POSICION DE LA ULTIMA BARRA
	int longitudDeNombre = stringLongitud(path)+1 - posUltimoBarra;

	//SI DECLARAS LA VARIABLE ESTATICAMENTE SE ELIMINA AL RETORNAR DE LA FUNCION!
	//char nombre [longitudDeNombre];
	char *nombre = malloc(longitudDeNombre+1);
	for(i=0; i < longitudDeNombre; i++) {
		nombre[i] = path[posUltimoBarra+i];
	}
	return nombre;
}

//SE CREA UN SEGMENTO NUEVO CON 1 PAGINA ASOCIADA Y EL NOMBRE DE LA TABLA TAMBIEN
segmento* segmento_crear(char* pathNombreTabla, pagina_referenciada* paginaRef) {
	log_info(log_memoria, "[CREANDO SEGMENTO] Creando segmento para la tabla '%s' y asociando marco '%d'",
			pathNombreTabla, paginaRef->nropagina);
	segmento* auxSeg = malloc(sizeof(segmento));
	auxSeg->path_tabla = (char*)malloc(strlen(pathNombreTabla)+1);
	auxSeg->paginasAsocida = paginaRef;
//	log_info(log_memoria, "[CREANDO SEGMENTO] Estructura de la nueva tabla creada"),

//	memcpy(auxSeg->path_tabla, pathNombreTabla, strlen(pathNombreTabla));
	strcpy(auxSeg->path_tabla, pathNombreTabla);
//	log_info(log_memoria, "[CREANDO SEGMENTO] PATH COPIADO '%s'", auxSeg->path_tabla);

	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[CREANDO SEGMENTO] Es el primer segmento en ser creado");
		//CREO EL PRIMER SEGMENTO DE LA TABLA

		auxSeg->siguienteSegmento=NULL;

	} else {
//		log_info(log_memoria, "[CREANDO SEGMENTO] CREANDO OTRO SEGMENTO A LA TABLA SEGMENTOS '%s'", pathNombreTabla);
		mutexBloquear(&mutex_segmento_modificando);
		auxSeg->siguienteSegmento=tablaSegmentos;
		mutexDesbloquear(&mutex_segmento_modificando);
	}
	log_info(log_memoria, "[CREANDO SEGMENTO] Segmento para la tabla '%s' creado correctamente!!!", pathNombreTabla);
	return auxSeg;
}

//void segmento_asociar_nueva_pagina(segmento* unSegmento, pagina_referenciada* ref){
//	mutexBloquear(&mutex_segmento_modificando);
//	unSegmento->cantPaginasAsociadas +=1;
//	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Por asociar la pagina '%d' al segmento '%s'",
//			ref->nropagina, unSegmento->path_tabla);
//
//	mutexBloquear(&mutex_pagina_referenciada_aux);
//	aux_tabla_paginas = unSegmento->paginasAsocida;
//	while(aux_tabla_paginas->sig!=NULL){
//		//ME PONGO A BUSCAR LA ULTIMA POSICION
//		aux_tabla_paginas = aux_tabla_paginas->sig;
//	}
//	aux_tabla_paginas->sig = ref;
//	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Segmento '%s' se le ha asignado con exito el nro de pagina %d",
//				unSegmento->path_tabla, aux_tabla_paginas->nropagina);
//	mutexDesbloquear(&mutex_pagina_referenciada_aux);
//	mutexDesbloquear(&mutex_segmento_modificando);
//}

void segmento_eliminar_nro_pagina(segmento* unSegmento, int nroAQuitar){

//	log_info(log_memoria, "[ELIMINANDO PAGINA] Sacando marco '%d' del segmento '%s'",nroAQuitar, unSegmento->path_tabla);

	mutexBloquear(&mutex_segmento_aux);
	mutexBloquear(&mutex_segmento_modificando);
	//BUSCO NRO DE PAGINA->LA ELIMINO -> ACOMODO LA LISTA
	pagina_referenciada* tablaPaginaBuscador;
	pagina_referenciada* tablaPaginaAnterior;
	tablaPaginaBuscador = unSegmento->paginasAsocida;
//	tablaPaginaAux = NULL;
	tablaPaginaAnterior = NULL;
//	printf("\nPASO A COMPARAR NUMEROS\n\n");
	int cont=0;
	while(tablaPaginaBuscador->nropagina != nroAQuitar){
		cont++;
	//	printf("\nNRO DEL SEGMENTO vs NROAQUITAR\n%d                ==      %d\n\n", tablaPaginaBuscador->nropagina, nroAQuitar);
//		log_info(log_memoria, "nro pag %d",tablaPaginaBuscador->nropagina);
//		tablaPaginaAux = tablaPaginaBuscador;
		tablaPaginaAnterior = tablaPaginaBuscador;
		tablaPaginaBuscador = tablaPaginaBuscador->sig;
	}
//	printf("\nENCONTRADO\n%d == %d\n\n", tablaPaginaBuscador->nropagina, nroAQuitar);
	log_info(log_memoria, "[ELIMINANDO PAGINA] Se elimina la pagina %d del segmento %s. Marco %d",cont,unSegmento->path_tabla,tablaPaginaBuscador->nropagina);
	//tablaPaginaAux = tablaPaginaBuscador->sig;
	if(tablaPaginaAnterior == NULL){
		unSegmento->paginasAsocida = tablaPaginaBuscador->sig;
	}else{
		tablaPaginaAnterior->sig = tablaPaginaBuscador->sig;
	}

	mutexBloquear(&mutex_bitmap);
	liberarPosicionLRU(tablaPaginaBuscador->nropagina);
	mutexDesbloquear(&mutex_bitmap);
//	free(tablaPaginaBuscador);

	//if(tablaPaginaAux==NULL){
	if(unSegmento->paginasAsocida == NULL){
		log_info(log_memoria,	"[ELIMINANDO PAGINA] Eliminando Segmento <%s> porque se quedo sin paginas",unSegmento->path_tabla);
		borrarSegmentoPasadoPorParametro(unSegmento);
		// REVISAR POR AQUI @NACHO @MARTIN

		}// else {
//			//unSegmento->paginasAsocida = tablaPaginaAux;
//			//log_info(log_memoria,"[ELIMINANDO PAGINA A SEGMENTO] NRO de pagina '%d' eliminada del segmento '%s'",nroAQuitar, unSegmento->path_tabla);
//		}

	mutexDesbloquear(&mutex_segmento_modificando);
	mutexDesbloquear(&mutex_segmento_aux);
}
//
//pagina_a_devolver* segmentoBuscarInfoEnTablaDePagina(char* nombreTabla,
//	u_int16_t key, bool comandoInsert){
//
//	log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] Por buscar KEY '%d' y PATH '%s'",
//			key, nombreTabla);
//	segmento* aux = tablaSegmentos;
//	while(aux!=NULL){
//	//	if(strcmp(nombreTabla, obtenerNombreTablaDePath(aux->path_tabla))){
//			if(strcmp(nombreTabla, aux->path_tabla)){
//			int nroDePagina;
////			log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', voy a buscar la key",	nombreTabla);
//			nroDePagina = buscarEntreTodasLasTablaPaginasLaKey(aux->paginasAsocida, key);
//		//	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
//			if(nroDePagina>-1){
////				log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', KEY encontrada '%d'",	nombreTabla, key);
//				if(comandoInsert){
//					//SE DEBE REALIZAR MODIFICACIONES
//		//			funcionInsert(nombreTabla, 0, key, valorAPoner);
//					return NULL;
//				} else {
//					//SOLO SE DEBE CONSULTAR LOS DATOS
//					return selectObtenerDatos(nroDePagina, true);
//				}
//			}
//
//		}
//	}
//	return NULL;
//
//}

pagina_a_devolver* selectObtenerDatos(int nroDePaginaAIr, bool necesitoValue){
//	log_info(log_memoria, "[Obtener Datos] Entrando a obtener datos");
	pagina_a_devolver* pag = malloc(sizeof(pagina_a_devolver));
	pagina* pag_con_datos = malloc(sizeof(pagina));

	void* informacion = malloc(sizeof(pagina)+max_valor_key);
//	log_info(log_memoria, "[Obtener Datos] Me pongo a buscar la pagina requerida");
	informacion = accederYObtenerInfoDePaginaEnPosicion(nroDePaginaAIr, informacion);
	mutexDesbloquear(&mutex_memoria);
//	mutexBloquear(&mutex_pagina_auxiliar);
	memcpy(pag_con_datos, informacion, sizeof(pagina));


	if(necesitoValue){
		pag->value = malloc(max_valor_key+1);
		memcpy(&pag->value, informacion+sizeof(pagina)-1, max_valor_key);
	}


	pag->key=pag_con_datos->key;
	pag->timestamp=pag_con_datos->timestamp;
//	log_info(log_memoria, "[Obtener Datos] KEY PAGINA OBTENIDA '%d'", pag->key);
//	log_info(log_memoria, "[Obtener Datos] VALUE OBTENIDO '%s'", pag->value);
//	mutexDesbloquear(&mutex_pagina_auxiliar);
	free(pag_con_datos);
	free(informacion);
//	log_info(log_memoria, "[Obtener Datos] Saliendo de obtener datos");
	return pag;
}

pagina_a_devolver* selectPaginaPorPosicion(int posicion, bool deboDevolverEsteValor){
//	log_info(log_memoria, "[SELECT] Por acceder a la memoria a la posicion '%d'", posicion);
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);

	void* info = malloc(sizeof(pagina)+max_valor_key);

	obtenerInfoDePagina(posicion, &info);

	pagina* pag = malloc(sizeof(pagina));


	memcpy(pag, info, sizeof(pagina));
//	log_info(log_memoria, "[OBTENIENDO DATOS] Actualizando el tiempo del último acceso para la pagina con key '%d'", pag->key);
	if(deboDevolverEsteValor){
		actualizarTiempoUltimoAcceso(pag->nroPosicion, false, false);
		log_info(log_memoria, "[SELECT] ultimo acceso de la pagina de la key %d actualizado", pag->key);
	}
	memcpy(bloque_memoria+posicion*(sizeof(pagina)+max_valor_key), pag, sizeof(pagina));
//	memcpy(bloque_memoria+posicion*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1,valorAPoner, max_valor_key);
	pagina_a_devolver* devolver = malloc(sizeof(pagina_a_devolver));
	devolver->value=malloc(max_valor_key);
	devolver->key = pag->key;
	devolver->nroPosicion= pag->nroPosicion;
	devolver->timestamp=pag->timestamp;
	memcpy(devolver->value, info+sizeof(pagina)-1, max_valor_key);
//	printf("DATO QUE OBTUVE DE INFO: <%s>\n", devolver->value);
//	log_info(log_memoria, "[SELECT] Datos obtenidos de la posicion '%d'", posicion);
	free(info);
	free(pag);
	mutexDesbloquear(&mutex_memoria);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);

	return devolver;
}

//ESTO SOLO SIRVE CUANDO SE INSERTAN NUEVOS ELEMENTOS A MEMORIA
void tabla_pagina_crear(
		u_int16_t key, char* valor, bool flag_modificado,
		pagina_referenciada** devolver, char* nombreTabla,
		bool existeSegmento, segmento* segmetnoApuntado,
		timestamp_mem_t timestamp_val) {
//	log_info(log_memoria, "[CREANDO TABLA DE PAGINAS] En crear Tabla de pagina y pagina nueva porque no estan con la key %d", key);

	pagina_referenciada* pag_ref = malloc(sizeof(pagina_referenciada));
	pag_ref->sig=NULL;
	pag_ref->nropagina= -1;
	int posicionAsignada = -1;
	//COMENTADO POR POSIBLE DEADLOCK
//	mutexBloquear(&mutex_pagina_auxiliar);
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
//	log_info(log_memoria, "[CREANDO PAGINA] Tabla de paginas para la tabla %s creada",nombreTabla);
		//MIENTRAS EST{E BLOQUEADO, NO SE PUEDE QUITAR O PONER NUEVAS PAGINAS
//	log_info(log_memoria,"[Crear Tabla y pagina] Verifico si hay espacio libre en Memoria");
	posicionAsignada = buscarPaginaDisponible(key, existeSegmento, nombreTabla, segmetnoApuntado);

/*	if(posicionAsignada==-1){
		mutexBloquear(&LRUMutex);
		log_info(log_memoria,
				"[Crear Tabla y pagina] NO hay espacio libre por lo tanto activo el LRU");
		//SIGNIFICA QUE LLEGO AL TOPE DE PAGINAS EN MEMORIA
		//O SEA ESTAN TODAS OCUPADAS, APLICO LRU
		LRU(crear_pagina(key, valor, -1,timestamp_val), &posicionAsignada, valor, flag_modificado, nombreTabla);
		pag_ref->nropagina=posicionAsignada;
	//	printf("\nNUMERO DE PAGINA ASIGNADA:\n<%d>\n", pag_ref->nropagina);
		mutexDesbloquear(&LRUMutex);
		log_info(log_memoria,
				"[Crear Tabla y pagina] LRU FINALIZADO, posicion de pagina NRO: %d",
				posicionAsignada);
	} else {
*/
		log_info(log_memoria,"[CREANDO PAGINA] Se asigna el marco %d para la pagina",posicionAsignada);
		//AUN HAY ESPACIO, GAURDO ESTA NUEVA PAGINA EN ALGUNA POSICION LIBRE
	//	pag_ref->nropagina=posicionAsignada;
//		log_info(log_memoria,"[Crear Tabla y pagina] Asigno el espacio libre a la tabla pagina y a la pagina");
	//	aux_pag = actualizarPosicionAPagina(aux_crear_pagina, posicionAsignada);
//		log_info(log_memoria,"[Crear Tabla y pagina] Procedo a asignar la tabla y la pagina a dicha posicion");

		asignarNuevaPaginaALaPosicion(posicionAsignada, crear_pagina(key, valor, posicionAsignada,timestamp_val),
				valor, flag_modificado, nombreTabla);
		pag_ref->nropagina=posicionAsignada;

//		log_info(log_memoria,"[Crear Tabla y pagina] ASIGNACION COMPLETADA");
//	}
//	log_info(log_memoria,
//			"[Crear Tabla y pagina] Desactivo el mutex mutex_tabla_pagina_en_modificacion");
	memcpy(*devolver, pag_ref, sizeof(pagina_referenciada));
//	log_info(log_memoria, "[CREANDO TABLA DE PAGINAS] Se guardo la pagina en el marco %d", pag_ref->nropagina);

	if(segmetnoApuntado != NULL){
		pagina_referenciada *aux = segmetnoApuntado->paginasAsocida;
		pagina_referenciada *anterior = NULL;
	//	if(posicionAsignada==-1){
		log_info(log_memoria, "[DBG] Entro al if");
		while(aux != NULL){
			log_info(log_memoria, "[DBG] Recorriendo paginas del segmento");
			anterior = aux;
			aux = aux->sig;
		}
		if(anterior == NULL){
			segmetnoApuntado->paginasAsocida = *devolver;
		}
		else{
			anterior->sig = *devolver;
		}
	}

	free(pag_ref);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
//	mutexDesbloquear(&mutex_pagina_auxiliar);
	return;
}

void LRU(
		/*
	 	pagina* paginaCreada, int* nroAsignado, char* valor, bool flag_modificado,
		char* nombreTabla
		*/
		){
	mutexBloquear(&ACCIONLRU);
//	imprimirAviso(log_memoria, "[LRU] Comenzando el LRU, empiezo a buscar la pagina a reemplazar");
	log_info(log_memoria, "[LRU] Comenzando el LRU, busco la pagina a reemplazar");


	int candidatoAQuitar = -1;
//	log_info(log_memoria, "[LRU] Busco la key entre los segmentos y tabla de paginas");
	char* nombreTablaQueDeboBuscar = malloc(tamanioPredefinidoParaNombreTabla);
//	printf("\nLINEA 1074 LRU: Se activo el LRU\n");
	candidatoAQuitar = buscarEnBloqueLRUElProximoAQuitar(&nombreTablaQueDeboBuscar);
//	printf("\n\nLINEA 1925: NOMBRE QUE TENGO QUE BUSCAR: %s  -  %d\n", nombreTablaQueDeboBuscar, candidatoAQuitar);

	if(candidatoAQuitar<0){
				free(nombreTablaQueDeboBuscar);
//				imprimirAviso(log_memoria, "JOURNAL FORZOSO ACTIVADO");
				log_info(log_memoria, "[LRU] No hay nada que se pueda reemplazar, se fuerza un JOURNAL");


				procesoJournal(-1);
				fprintf(tablas_fp,"\nEjecutado JOURNAL POR MEMORIA FULL");
				loggearEstadoActual(tablas_fp);
			//	log_info(log_memoria, "[LRU sin candidato] JOURNAL HECHO, lo asigno a la primera posicion");
			//	paginaCreada->nroPosicion=0;
			//	asignarNuevaPaginaALaPosicion(0, paginaCreada, valor, flag_modificado, nombreTabla);
			} else {
				log_info(log_memoria, "[LRU] Se asigno el marco %d para la pagina", candidatoAQuitar);
	//			paginaCreada->nroPosicion=candidatoAQuitar;
	/*			printf("\n\n[QUE DEBO SACAR]\nSEGMENTO A QUITAR: %s\nPOSICION A SACAR: %d\n\n\n",
						nombreTablaQueDeboBuscar,
						candidatoAQuitar);
						*/
				//@revisar @aqui @sincro @aqui
				segmento_eliminar_nro_pagina(
					buscarSegmentoPorNombreTabla(nombreTablaQueDeboBuscar),
						candidatoAQuitar);
				free(nombreTablaQueDeboBuscar);
		//		printf("\n\nPOR AQUI PUTO\n\n\n");
		/*		asignarNuevaPaginaALaPosicion(candidatoAQuitar, paginaCreada,
						valor, flag_modificado, nombreTabla);

				*nroAsignado = candidatoAQuitar;
*/
//				log_info(log_memoria, "[LRU con candidato] LRU TERMINADO");
			}
	mutexDesbloquear(&ACCIONLRU);
	return candidatoAQuitar;
}

void limpiar_todos_los_elementos_de_1_segmento(segmento* segmentoABorrar){
	//ESTO SE DEBE REVISAR, TIENE ERRORES @revisar @aqui
	log_info(log_memoria,"[BORRANDO SEGMENTO] Borrando segmento %s y liberando todas las paginas asociadas",segmentoABorrar->path_tabla);
	liberarTodosLasTablasDePaginas(segmentoABorrar->paginasAsocida);
	log_info(log_memoria,"1");
	free(segmentoABorrar->path_tabla);
	log_info(log_memoria,"2");
	free(segmentoABorrar);
	log_info(log_memoria,"3");
//	log_info(log_memoria, "[LIBERAR SEGMENTO] SEGMENTO LIBERADO");
}

void liberarTodosLasTablasDePaginas(pagina_referenciada* ref){
	pagina_referenciada* refaux;
	void* info = malloc(sizeof(pagina)+max_valor_key);
	mutexBloquear(&mutex_bitmap);
	int i = 0;
	while(ref!=NULL){
		refaux = ref->sig;
	//	retardo_memoria(arc_config->retardo_mem);
		log_info(log_memoria, "[DBG] Borrando");
		memcpy(bloque_memoria+(ref->nropagina)*(sizeof(pagina)+max_valor_key), info, sizeof(pagina)+max_valor_key);
		liberarPosicionLRU(ref->nropagina);
		//log_info(log_memoria, "[LIBERAR SEGMENTO] TABLA DE LA PAGINA NRO '%d' LIBERADA", ref->nropagina);
		free(ref);
		ref = refaux;
		i++;
	}
	log_info(log_memoria, "[DBG] SALGO DEL WHILE");
	mutexDesbloquear(&mutex_bitmap);
	free(info);
	log_info(log_memoria, "[BORRANDO SEGMENTO] Se borraron todas las paginas del segmento");
}

void liberarPosicionLRU(int posicionAIr) {

	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	void* info = malloc(sizeof(nodoLRU));
	memcpy(bloque_LRU+posicionAIr*desplazamiento, info, sizeof(nodoLRU));
	free(info);
	bitmapLiberarBit(bitmap, posicionAIr);
	log_info(log_memoria, "[DBG] BLOQUE DE LA LRU EN LA POSICION NRO '%d' LIBERADA", posicionAIr);

}

void liberar_todo_segmento(){
	segmento* aux;
	mutexBloquear(&mutex_segmento_en_modificacion);
	log_info(log_memoria, "[BORRANDO SEGMENTO] Se borraran todos los segmentos");
	while(tablaSegmentos!=NULL){
		aux = tablaSegmentos->siguienteSegmento;
		log_info(log_memoria, "[BORRANDO SEGMENTO] LIBERANDO SEGMENTO DE TABLA '%s'", tablaSegmentos->path_tabla);
		liberarTodosLasTablasDePaginas(tablaSegmentos->paginasAsocida);
		free(tablaSegmentos->path_tabla);
		free(tablaSegmentos);
		tablaSegmentos = aux;
	}

	mutexDesbloquear(&mutex_segmento_en_modificacion);
}

//void liberar_toda_tabla_paginas(pagina_referenciada* pag){
//	pagina_referenciada* aux;
//	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
//	while(pag!=NULL){
//		aux = pag->sig;
//		log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
//		free(pag);
//		pag = aux;
//	}
//	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] TERMINADO");
//}

/*void vaciar_tabla_paginas_y_memoria(){
	pagina* pag;
	printf("\nFALTA UN MALLOC!!!!!!!!!!!!!!!!!");
	printf("\nFunción vaciar_tabla_paginas_y_memoria\n");
	getchar();
	// SI ESTO SE USA ACÁ FALTA UN MALLOC
	pag->key=-1;
	pag->nroPosicion=-1;
	pag->timestamp=-1;
	char valor_a_nulo [max_valor_key];
	log_info(log_memoria, "[LIBERAR PAGINAS] BLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	int i;


	for(i=0; i<cantPaginasTotales;i++){

			memcpy(bloque_memoria+i*(sizeof(pagina)+max_valor_key), pag, sizeof(pagina)+max_valor_key);
		}
		log_info(log_memoria, "[LIBERAR PAGINAS] PAGINAS VACIADA");

	//EMPIEZA A BORRAR DATOS, QUIZAS DEBAMOS PONER 1 SEMAFORO MUTEX PARA INDICAR QUE YA SE BORRARON LAS PAGINAS

	log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Por liberar la tabla de paginas de todo segmento");

	segmento* segaux = tablaSegmentos;
	while(segaux!=NULL){
		pagina_referenciada* ref = segaux->paginasAsocida;
		pagina_referenciada* otro;
		log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Liberando tablas de paginas del segmento '%s'", segaux->path_tabla);
		while(ref!=NULL){
			otro = ref->sig;
			log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Pagina nro '%d'LIBERADO", ref->nropagina);
			free(ref);
			ref = otro;
		}
	}

	log_info(log_memoria, "[LIBERAR PAGINAS] TABLA DE PAGINAS VACIADA");

	log_info(log_memoria, "[LIBERAR PAGINAS] PONGO EL SEMAFORO cantPaginasDisponibles = cantPaginasTotales");
	cantPaginasDisponibles=cantPaginasTotales;
	log_info(log_memoria, "[LIBERAR PAGINAS] DESBLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);


}*/



void modificarValoresDeTablaYMemoriaAsociadasAKEY(int posAIr, char* valorNuevo, timestamp_mem_t timestamp_val) {
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	mutexBloquear(&mutex_memoria);
	pagina* aux = malloc(sizeof(pagina));
//	char valorString[max_valor_key];

	//log_info(log_memoria, "[Modificar valores de pagina] Entrando");
	memcpy(aux, bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key), sizeof(pagina));
//	printf("1 hecho\n");

	if(timestamp_val == 0)
		timestamp_val = timestamp();
	if(timestamp_val <= aux->timestamp){
		log_info(log_memoria, "[INSERT] El timestamp es anterior al almacenado. Ignorando cambios");
		free(aux);
		//log_info(log_memoria,"[Modificar valores de pagina] Saliendo");
		mutexDesbloquear(&mutex_memoria);
		mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
		return;
	}
	aux->timestamp = timestamp_val;
	actualizarTiempoUltimoAcceso(posAIr, true, true);

	//strcpy(valorString, valorNuevo);
/*
	log_info(log_memoria,
"[Modificar valor pagina] Pagina modificada con key '%d' VALORES NUEVOS;  TIMESTAMP '%llu'; VALOR '%s'",
											aux->key, aux->timestamp, valorNuevo);
*/
//	log_info(log_memoria,"[MOdificar valor pagina] Guardando los datos actualizados la pagina con key: %d",	aux->key);


//	printf("\n\nEN MODIFICACION NUEVO TIMESTAMP: %d - %llu\n\n", aux->key, aux->timestamp);

	memcpy(bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key), aux, sizeof(pagina));

	memcpy(bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1, valorNuevo,strlen(valorNuevo)+1);
	log_info(log_memoria,"[INSERT] Se guardo el valor %s en la pagina %d",valorNuevo, posAIr);

//	modificar_bloque_LRU("%", aux->timestamp, posAIr, true, false);

//	log_info(log_memoria,	"[MOdificar valor tabla pagina] Actualizar FLAG de tabla pagina asociada a la key: %d",
//			aux->key);
////	actualizarFlagDeLaKey(aux->key);

//	log_info(log_memoria,	"[MOdificar valor tabla pagina] FLAG ACTUALIZADO EN MODIFICADO PARA LA TABLA "
//			"DE LA KEY|NRO DE PAGINA: %d|%d",
//			aux->key, posAIr);

//	log_info(log_memoria,
//			"[Modificar valor pagina] Se ham modificado el FLAG de la tabla KEY|NRO DE PAGINA: %d|%d",
//			aux->key,posAIr);
	free(aux);
//	log_info(log_memoria,
//			"[Modificar valor pagina] Desbloqueo el MUTEX mutex_tabla_pagina_en_modificacion");
//	log_info(log_memoria,
//			"[Modificar valores de pagina] Saliendo");
	mutexDesbloquear(&mutex_memoria);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
}


void modificar_bloque_LRU(char* nombreTabla, timestamp_mem_t timestamp, int nroPosicion, bool estado,
		bool vieneDeFuncionInsert)
{
//	mutexBloquear(&LRUMutex);
//	log_info(log_memoria, "[MODIFICAR BLOQUE LRU]\nActualizar bloque LRU");
	nodoLRU* nuevoNodo = malloc(sizeof(nodoLRU));
	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	char* nombreDeTabla = malloc(tamanioPredefinidoParaNombreTabla);
	if(vieneDeFuncionInsert){
//		nuevoNodo->nombreTabla = malloc(strlen(nombreTabla));
//		memcpy(nuevoNodo->nombreTabla, nombreTabla, strlen(nombreTabla)+1);
	//	strcpy(nuevoNodo->nombreTabla, nombreTabla);
		nuevoNodo->estado=estado;
		nuevoNodo->nroPagina=nroPosicion;
		nuevoNodo->timestamp=timestamp;

		nodoLRU *anterior = malloc(sizeof(nodoLRU));
		memcpy(anterior, bloque_LRU+nroPosicion*desplazamiento, sizeof(nodoLRU));
		if(anterior->estado != nuevoNodo->estado){
			log_info(log_memoria,"[LRU] Poniendo flag de modificado de marco %d en TRUE",nroPosicion);
		}
		free(anterior);
		memcpy(bloque_LRU+nroPosicion*desplazamiento, nuevoNodo, sizeof(nodoLRU));
		if(nombreTabla != NULL){
			memcpy(bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU), nombreTabla,strlen(nombreTabla)+1);
		}
		memcpy(nombreDeTabla, bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU),tamanioPredefinidoParaNombreTabla);
		if(verificarSiEstaFUll()){
			imprimirAviso(log_memoria,"******MEMORIA FULL, SE DEBE REALIZAR 1 JOURNAL SI SE INGRESA ALGO NUEVO******");
		}
		/*
		printf("\n\n\nNOMBRE TABLA INGRESADA: <<<%s>>>", nombreTabla);
		char* auxnombre = malloc(tamanioPredefinidoParaNombreTabla);
		memcpy(auxnombre, bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU),
				tamanioPredefinidoParaNombreTabla);
		printf("\n\n\nNOMBRE TABLA OBTENIDA: <<<%s>>>", auxnombre);
		printf("[MODIFICAR BLOQUE LRU]\nDATOS INGRESADOS:\nNOMBRE TABLA <%s>\nNUMERO PAGINA: <%d>\nTIMESTAMP: <%llu>\nESTADO PAGINA: <%d>",
				nombreTabla, nroPosicion, timestamp, estado);

		free(auxnombre);
		*/
		/*printf("[MODIFICAR BLOQUE LRU] DATOS INGRESADOS: NOMBRE TABLA <%s>. NUMERO PAGINA: <%d>. TIMESTAMP: <%llu>. ESTADO PAGINA: <%d>.",
				nombreDeTabla, nroPosicion, timestamp, estado);*/
		log_info(log_memoria, "[MODIFICANDO DATOS LRU] TABLA <%s>. MARCO: <%d>. TIMESTAMP: <%llu> FLAG: <%d>",
				nombreDeTabla, nroPosicion, timestamp, estado);
	//	free(nuevoNodo->nombreTabla);
	} else {
		memcpy(nuevoNodo, bloque_LRU+nroPosicion*desplazamiento, sizeof(nodoLRU));
		memcpy(nombreDeTabla, bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU),
				tamanioPredefinidoParaNombreTabla);
		nuevoNodo->timestamp=timestamp;
		/*
  	  	if(estado==false){
			nuevoNodo->estado = estado;
		}
		*/

		memcpy(bloque_LRU+nroPosicion*desplazamiento, nuevoNodo, sizeof(nodoLRU)-1);
		log_info(log_memoria, "[MODIFICANDO DATOS LRU] TABLA <%s>. MARCO: <%d>. TIMESTAMP: <%llu> FLAG: <%d>",
						nombreDeTabla, nroPosicion, timestamp, estado);
//		log_info(log_memoria, "[MODIFICAR BLOQUE LRU] DATOS ACTUALIZADOS DE LA POSICION: <%d>",
//				nroPosicion);

	}
	free(nombreDeTabla);
//	free(nuevoNodo->nombreTabla);
	free(nuevoNodo);
//	mutexDesbloquear(&LRUMutex);
}

bool verificarSiEstaFUll(){
	int i = 0;
	nodoLRU* nuevoNodo = malloc(sizeof(nodoLRU));
	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	//log_info(log_memoria, "[VERIFICANDO SI ESTA FULL] ENTRANDO");
	for(i=0;i<cantPaginasTotales;i++){
		memcpy(nuevoNodo, bloque_LRU+i*desplazamiento, sizeof(nodoLRU));
		if(nuevoNodo->estado == 0){
//			log_info(log_memoria, "[VERIFICANDO SI ESTA FULL] 1 Nodo no fue modificado por lo tanto no esta FULL");
			free(nuevoNodo);
			return false;
		}
	}
	free(nuevoNodo);
//	log_info(log_memoria, "[VERIFICANDO SI ESTA FULL] Esta FULL");
	return true;
}

/*-----------------------------------------------------
 * FUNCIONES DE JOURNAL
 *-----------------------------------------------------*/
//
//bool bloque_LRU_en_posicion_i_tiene_flag_activado(int posicion){
//	nodoLRU* nodoSolicitado = malloc(sizeof(nodoLRU));
//	memcpy(nodoSolicitado, bloque_LRU+posicion*sizeof(nodoLRU), sizeof(nodoLRU));
//	if(nodoSolicitado->estado){
//		free(nodoSolicitado);
//		return true;
//	}
//	free(nodoSolicitado);
//	return false;
//}

void liberarDatosJournal(datosJournal* datos){
	datosJournal* aux;
//	log_info(log_memoria, "[JOURNAL] LIBERANDO DATOSJOURNAL");

	while(datos!=NULL){
		free(datos->value);
		free(datos->nombreTabla);
		aux = datos->sig;
		free(datos);
		datos = aux;
	}
//	log_info(log_memoria, "[JOURNAL] DATOSJOURNAL LIBERADO");

}

void limpiezaGlobalDeMemoriaYSegmentos(){
	log_info(log_memoria, "[LIBERANDO MEMORIA] BORRO SEGMENTOS");
	limpiar_y_destruir_todo_segmento();
//	log_info(log_memoria, "[limpiezaGlobalDeMemoriaYSegmentos] COMIENZO A LIBERAR MEMORIAS");
	void* liberarMemoriaPrincipal = malloc(sizeof(arc_config->tam_mem));
//	void* liberarLRU = malloc(cantPaginasTotales*sizeof(nodoLRU));
//	void* liberarLRU = malloc(cantPaginasTotales*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla));
	/*nodoLRU *auxLRU = malloc(sizeof(nodoLRU));
	auxLRU->estado = false;
	auxLRU->nroPagina = -1;
	auxLRU->timestamp = 0;
	for(int i = 0; i<cantPaginasTotales; i++){
		memcpy(bloque_LRU + sizeof(nodoLRU)*i, auxLRU, sizeof(nodoLRU));
	}*/
//	memcpy(bloque_LRU, liberarLRU, sizeof(cantPaginasTotales*sizeof(nodoLRU)));
//	memcpy(bloque_LRU, liberarLRU, cantPaginasTotales*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla));
//	memcpy(bloque_memoria, liberarMemoriaPrincipal, sizeof(arc_config->tam_mem));
	free(liberarMemoriaPrincipal);
//	free(liberarLRU);


	/*libero bloque LRU*/
	free(bloque_LRU);
	bloque_LRU = malloc(cantPaginasTotales*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla));
	//free(auxLRU);
	cantPaginasDisponibles = cantPaginasTotales;
	bitmapDestruir(bitmap);
	bitmap = bitmapCrear(cantPaginasTotales);
	log_info(log_memoria, "[LIBERANDO MEMORIA] MEMORIA LIBERADA");
}

void limpiar_y_destruir_todo_segmento(){
	log_info(log_memoria, "[LIBERANDO SEGMENTOS] Entrando, comienza la limpieza total");
	segmento* siguienteSegmento;
	while(tablaSegmentos!=NULL){
		siguienteSegmento = tablaSegmentos->siguienteSegmento;
		log_info(log_memoria, "[LIBERANDO SEGMENTOS] Limpiando segmento: %s", tablaSegmentos->path_tabla);
		limpiar_todos_los_elementos_de_1_segmento(tablaSegmentos);
		tablaSegmentos = siguienteSegmento;
	}
	log_info(log_memoria, "[LIBERANDO SEGMENTOS] TODOS LOS SEGMENTOS FUERON LIBERADOS");
}

datosJournal* obtener_todos_journal(){
	datosJournal* datosDevolver = NULL;
	int posicion;
	char* nombreTabla;
	char* valor = malloc(max_valor_key);
	pagina* pag = malloc(sizeof(pagina));
	//log_info(log_memoria, "[OBTENER TODO JOURNAL] ENTRANDO");
	for(posicion=0;posicion<cantPaginasTotales;posicion++){

		nombreTabla =malloc(tamanioPredefinidoParaNombreTabla);
		if(bloque_LRU_en_posicion_fue_modificado(posicion, &nombreTabla)){
			//log_info(log_memoria, "[OBTENER TODO JOURNAL] Obtengo datos de la posicion %d",posicion);
			datosJournal* datos = malloc(sizeof(datosJournal));

			memcpy(pag,	bloque_memoria+posicion*(sizeof(pagina)+max_valor_key),	sizeof(pagina));
			datos->nombreTabla = malloc(strlen(nombreTabla)+1);
			memcpy(datos->nombreTabla, nombreTabla, strlen(nombreTabla)+1);
			datos->key = pag->key;
			datos->timestamp = pag->timestamp;
			datos->value = malloc(max_valor_key);
			memcpy(datos->value,
					bloque_memoria+posicion*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1,
					max_valor_key);
			datos->sig= datosDevolver;
			datosDevolver = datos;
			log_info(log_memoria, "[JOURNAL] Segmento: %s. Key: %d. Valor: %s. Timestamp: %llu",
					datos->nombreTabla, datos->key, datos->value, datos->timestamp);
		}
		free(nombreTabla);
	}
//	datosJournal* extra = datosDevolver;
/*
	while(extra!=NULL){
		printf("\nOBTENGO DATOS:\nNombre: [%s]\nKey: [%d]\nTimestamp: [%f]\nVALUE: [%s]\n\n",
				extra->nombreTabla, extra->key, extra->timestamp, extra->value);
		extra = extra->sig;
	}
*/
//	datosAPasar = datos;
//	sleep(5);
	free(valor);
	free(pag);
	return datosDevolver;
}


bool bloque_LRU_en_posicion_fue_modificado(int pos, char** nombreADevolver){
	//REVISO PRIMERO BITMAP
//	log_info(log_memoria, "[bloque_LRU_en_posicion_fue_modificado]\nENTRANDO");
	if(!bitmapBitOcupado(bitmap, pos)){
//		log_info(log_memoria, "\n[bloque_LRU_en_posicion_fue_modificado]\n La posicion %d no fue ocupada, devuelvo FALSE", pos);
		return false;
	}
	nodoLRU* nodoSolicitado = malloc(sizeof(nodoLRU));
	//log_info(log_memoria, "[bloque_LRU_en_posicion_fue_modificado] Obteniendo datos de la posicion %d", pos);
	memcpy(nodoSolicitado, bloque_LRU+pos*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla), sizeof(nodoLRU));
	memcpy(*nombreADevolver, bloque_LRU+pos*(sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla)+
			sizeof(nodoLRU), tamanioPredefinidoParaNombreTabla);
	if(nodoSolicitado->estado){
		//log_info(log_memoria, "[Bloque_LRU_en_posicion_fue_modificado] BLOQUE %d modificado, devuelvo TRUE", pos);
		free(nodoSolicitado);
		return true;
	}
//	log_info(log_memoria, "[bloque_LRU_en_posicion_fue_modificado]La posicion %d no fue modifcada, devuelvo FALSE", pos);
	free(nodoSolicitado);
	return false;
}

//PROTOTIPO


bool bitmapLleno(){
	int posicion;
	for(posicion=0;posicion<cantPaginasTotales;posicion++){
		if(!bitmapBitOcupado(bitmap, posicion)){
			return false;
		}
	}
	return true;
}

void insertCrearPaginaConNuevoSegmento(char* nombreTabla, u_int16_t keyBuscada,
		pagina_referenciada* ref, char* valorAPoner, bool estadoAPoner,
		segmento* segmentoBuscado, timestamp_mem_t timestamp_val){
//	log_info(log_memoria, "[INSERT] Tampoco se encontro que existe un segmento asociado a la tabla '%s'. Procedo a crear el segmento, tabla de pagina y alojar la pagina en memoria",
//							nombreTabla);
		//NO SE ENCONTRO NINGUN SEGMENTO CON EL NOMBRE DE LA TABLA BUSCADA POR LO TANTO DEBO CREARLA

		//		log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina, el nro de su posicion es '%d'", nroTablaCreada);
				//tabla_pagina_crear(keyBuscada, valorAPoner, estadoAPoner,
				//		&ref, nombreTabla, estadoAPoner, NULL);
	tabla_pagina_crear(keyBuscada, valorAPoner, estadoAPoner,
								&ref, nombreTabla, false, segmentoBuscado, timestamp_val);
//	if(estadoAPoner) {
//		log_info(log_memoria,"[INSERT] Pagina creada con NROPAGINA|KEY|FLAG: %d|%d|TRUE",ref->nropagina, keyBuscada);
//	} else {
//		log_info(log_memoria,"[INSERT] Tabla de pagina referenciada creada con info NROPAGINA|KEY|FLAG: %d|%d|FALSE",ref->nropagina, keyBuscada);
//	}
	segmentoBuscado = segmento_crear(nombreTabla, ref);
	segmentoBuscado->paginasAsocida = ref;
//	log_info(log_memoria, "[INSERT] SEGMENTO Creado para la tabla '%s' . Como es el primer segmento TABLA SEGMENTOS apuntara este elemento", segmentoBuscado->path_tabla);
//	if(tablaSegmentos == NULL)
//		log_debug(log_memoria,"ES NULL WACHOOOOOOOOOOOOO");
	segmentoBuscado->siguienteSegmento = tablaSegmentos;
	tablaSegmentos = segmentoBuscado;
//	log_info(log_memoria, "[INSERT] Se ha creado un segmento para la tabla '%s'", nombreTabla);

}
