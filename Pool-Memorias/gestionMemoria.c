/*
 * gestionMemoria.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#include "gestionMemoria.h"
//#include "../Biblioteca/src/Biblioteca.c"

void* accederYObtenerInfoDePaginaEnPosicion(int posicion, void* info){
	log_info(log_memoria, "[ACCEDIENDO A DATOS] Por acceder a la memoria a la posicion '%d'", posicion);
//	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	obtenerInfoDePagina(posicion, &info);
//	mutexDesbloquear(&mutex_memoria);
	log_info(log_memoria, "[ACCEDIENDO A DATOS] Datos obtenidos de la posicion '%d'", posicion);
//	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);

	return info;
}

pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos){
	unaPagina->nroPosicion= (short)nuevaPos;
	return unaPagina;
}

void asignarNuevaPaginaALaPosicion(
		int posLibre, pagina* pagina_nueva, char* valorAPoner,
		bool estadoAsignado, char* nombreTabla){
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Por guardar los datos de el valor y la nueva pagina en memoria");
//	char stringValor[max_valor_key];
//	strcpy(stringValor, valorAPoner);

	mutexBloquear(&mutex_memoria);
	mutexBloquear(&mutex_bitmap);

	//AQUI SE GUARDA LA PAGINA
	memcpy(bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key), pagina_nueva, sizeof(pagina));
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Pagina guardada");

	memcpy(bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1,valorAPoner, max_valor_key);
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] El valor de la pagina fue guardada, actualizo el BITMAP ocupando la posicion '%d'", posLibre);
//	free(stringValor);
	bitmapOcuparBit(bitmap, posLibre);

	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Se han guardado exitosamente los datos\nAhora mismo hay '%d' espacios libres", cantPaginasDisponibles);
	pagina* pagNew = malloc(sizeof(pagina));
	char valorString[max_valor_key];

	memcpy(pagNew, bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key), sizeof(pagina));
	memcpy(valorString, bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1, max_valor_key);
//	printf("\n\nNOMBRE QUE DEBO INGRESAR A BLOQUE LRU: %s\n\n\n", nombreTabla);
	modificar_bloque_LRU(nombreTabla, timestamp(), posLibre, estadoAsignado, true);

	printf("[asignarNuevaTablaAPosicionLibre]POSICION %d\nVALORES EN BLOQUE: KEY|VALUE|TIMESTAMP %d|%s|%f\nVALORES ORIGINALES: KEY|VALUE|TIMESTAMP %d|%s|%f\n",
				pagNew->nroPosicion, pagNew->key, valorString, pagNew->timestamp,
				pagina_nueva->key, valorAPoner,pagina_nueva->timestamp);

	log_info(log_memoria,"[asignarNuevaTablaAPosicionLibre]POSICION %d\nVALORES PUESTOS en bl: KEY|VALUE|TIMESTAMP %d|%s|%f\nVALORES PUESTOS: KEY|VALUE|TIMESTAMP %d|%s|%f\n",
			pagNew->nroPosicion, pagNew->key, valorString, pagNew->timestamp,
			pagina_nueva->key, valorAPoner,pagina_nueva->timestamp);
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
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Por buscar la KEY '%d'", keyBuscada);
	while(tablaasociadaaux!=NULL){
//		informacion = malloc(sizeof(pagina)+max_valor_key);
		pagina_devolver = selectPaginaPorPosicion(tablaasociadaaux->nropagina, false);

		if(pagina_devolver->key==keyBuscada){
			log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] SE ENCONTRO LA KEY EN LA POSICION PARA LA KEY|POSICION %d|%d",
				keyBuscada, tablaasociadaaux->nropagina);
		//	tablaasociadaaux->vecesAccedido +=1;
			i= tablaasociadaaux->nropagina;
			free(pagina_devolver->value);
			free(pagina_devolver);
//			free(informacion);
			mutexDesbloquear(&mutex_memoria);
			mutexDesbloquear(&mutex_pagina_referenciada_aux2);
			return i;
		}
//		free(pagina_devolver);
	//	free(informacion);
//		free(informacion);
		free(pagina_devolver->value);
		free(pagina_devolver);
		mutexDesbloquear(&mutex_memoria);
		tablaasociadaaux = tablaasociadaaux->sig;
	}
	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] NO SE ENCONTRO LA PAGINA PARA LA KEY EN LA POSICION PARA LA KEY%d",
			keyBuscada);


	return -1;
}

//Trata de buscar entre todos los segmentos la key buscada a partir de la key  y nombre tabla

//1* PARTE PARA BUSCAR UJNA KEY, usada principalmente para select
int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey
	(char* nombreTabla, u_int16_t keyBuscada,
		segmento** segmentoBuscado, int* nroDePagina)
{
	log_info(log_memoria,
			"[Buscar Key] Empiezo a buscar la key '%d' perteneciente a la tabla '%s'",
			keyBuscada, nombreTabla);
	segmento* seg_aux = buscarSegmentoPorNombreTabla(nombreTabla);
	*segmentoBuscado = seg_aux;
	if(seg_aux!=NULL){
		//EXISTE EL SEGMENTO
		log_info(log_memoria,
				"[Buscar Key] Se encontro el segmento con nombre '%s'",
				seg_aux->path_tabla);
		return buscarEntreTodasLasTablaPaginasLaKey(seg_aux->paginasAsocida, keyBuscada);
	}
	//NO EXISTE EL SEGMENTO
	log_info(log_memoria,
			"[Buscar Key] NO EXISTE NINGUN SEGMENTO ASOCIADA A '%s'; Devuelvo ERROR",
			nombreTabla);
	return -1;
}

int buscarEnMemoriaLaKey(u_int16_t keyBuscada){
	pagina* pag = malloc(sizeof(pagina));

	int pos;
	log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] ENTRO, Me pongo a buscar la key '%d'", keyBuscada);
	for(pos=0; pos<cantPaginasTotales; pos++){
		//ACCEDO A LA MEMORIA, HAGO RECORRIDO POR TODA ELLA BUSCANDO LA KEY Y DEVUELVO LA POSICION
		if(bitmapBitOcupado(bitmap, pos)){
			memcpy(pag, bloque_memoria+pos*(sizeof(pagina)+max_valor_key), sizeof(pagina));
			if(pag->key==keyBuscada){
				free(pag);
				log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] KEY ENCONTRADA '%d' en la posicion (%d)", keyBuscada, pos);
				return pos;
			}
		}
	}
	free(pag);
	log_info(log_memoria, "[BUSCAR KEY EN TODA MEMORIA] NO se encontro la key '%d', devuelvo -1 y SALIMOS", keyBuscada);
	//NO ENCONTRO NADA, DEVUELVO ERROR QUE ES -1
	return -1;
}

segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[BUSCANDO SEGMENTO X KEY] No se ha inicializado la tabla de segmentos, devuelvo NULL");
		return NULL;
	}
	segmento* todoSegmento = tablaSegmentos;
	char* nombreEnSegmento;
	log_info(log_memoria, "[BUSCANDO KEY] Buscando el segmento referido para la tabla '%s'", nombreTabla);
	while(todoSegmento!=NULL){
	//	nombreEnSegmento = obtenerNombreTablaDePath(todoSegmento->path_tabla);
		nombreEnSegmento = todoSegmento->path_tabla;
		log_info(log_memoria, "[BUSCANDO KEY] Nombre buscado | NOmbre obtenido '%s'|'%s'",
				nombreTabla, nombreEnSegmento);

		if(stringIguales(nombreEnSegmento, nombreTabla)){
			//SE ENCONTRO EL SEGMENTO BUSCADO
			log_info(log_memoria, "[BUSCANDO SEGMENTO X NOMBRETABLA] Se encontro el segmento buscado");
			return todoSegmento;
		}
		todoSegmento = todoSegmento->siguienteSegmento;
	}
	log_info(log_memoria, "[BUSCANDO SEGMENTO X KEY] NO se encontro el segmento buscado, devuelvo NULL");
	return NULL;
}

int buscarPaginaDisponible(u_int16_t key, bool existiaTabla,
		char* nombreTabla, segmento* segmetnoApuntado){
	int i;
	pagina_a_devolver* pag;
	pagina_referenciada* pagina_referenciada;

	mutexBloquear(&mutex_bitmap);
	log_info(log_memoria, "[BITMAP] Verifico si tiene espacio libre del total de paginas que son: '%d'",
				cantPaginasTotales);

	if(existiaTabla){
		log_info(log_memoria, "[BITMAP] Busco key [%d] en el segmento '%s'",
					key, segmetnoApuntado->path_tabla);
		pagina_referenciada = segmetnoApuntado->paginasAsocida;
	//	printf("\n\n[SEGMENTO]\nSEGMENTO NRO: %d\n\n\n", pagina_referenciada->nropagina);
		while(pagina_referenciada!=NULL){
			pag = selectObtenerDatos(pagina_referenciada->nropagina, false);
			log_info(log_memoria, "[BITMAP] Obtuve la pagina de la posicion '%d' y tiene key '%d' y busco la key '%d'",
					pagina_referenciada->nropagina, pag->key, key);
			if(pag->key == key){
				log_info(log_memoria, "\n[BITMAP] La posicion LIBRE: '%d'\n",
						pagina_referenciada->nropagina, key);
				free(pag);
				mutexDesbloquear(&mutex_bitmap);
				return pagina_referenciada->nropagina;
			}
			log_info(log_memoria, "[BITMAP] La posicion '%d' NO esta  libre,\n paso al siguiente BITMAP",
				pagina_referenciada->nropagina, key);

			free(pag);
			pagina_referenciada = pagina_referenciada->sig;
		}
		log_info(log_memoria, "[BITMAP]\n No se encontro en el segmento  '%s' la key '%d' buscad\nPaso a crearla!",
				segmetnoApuntado->path_tabla, key);
//		mutexDesbloquear(&mutex_bitmap);
	//	return -1;
	}

	for(i=0;i<cantPaginasTotales;i++){
		log_info(log_memoria, "[BITMAP] Busco la siguiente posicion libre para la key [%d]", i);
		if(!bitmapBitOcupado(bitmap, i)){
			//esta libre la posicion
			log_info(log_memoria, "[BITMAP] La posicion '%d' esta libre", i);
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
	double menortimestamp = timestamp();
	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	log_info(log_memoria, "[ALGORITMO LRU]\nENTRO A LRU, comienzo a buscar candidato");
	for(i=0; i<cantPaginasTotales;i++){
		memcpy(nodo, bloque_LRU+i*desplazamiento, sizeof(nodoLRU));
		if(nodo->estado == true){
			//SIGNIFICA QUE NO FUE MODIFICADO
			printf("TIMESTAMP ACTUAL = %f\nTIMESTAMP A COMPARAR: %f\n\n",
					menortimestamp, nodo->timestamp);
			if(menortimestamp > nodo->timestamp){
				printf("ENTRO AQUI\n\n");
				menortimestamp = nodo->timestamp;
				potencialCandidato = i;
			}
		}
	}
	//printf("[ALGORITMO LRU]\nLRU FINALIZADO\nCandidato a quitar elegido fue: <%d>",potencialCandidato);
	log_info(log_memoria, "[ALGORITMO LRU]\nLRU FINALIZADO\nCandidato a quitar elegido fue: <%d>",
			potencialCandidato);
	if(i<0){
		*nombreTablaCorrespondienteASacarTablaDePagina=NULL;
	} else {
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
	log_info(log_memoria, "[BORRANDO 1 SEGMENTO] Borrando el segmento <%s>", unSegmento->path_tabla);
	while(aux_segmentos != unSegmento){
		aux_aux_segmento = aux_segmentos;
		aux_segmentos = aux_segmentos->siguienteSegmento;
		if(aux_segmentos==NULL){
			log_info(log_memoria, "[BORRANDO 1 SEGMENTO]\n Segmento ya fue BORRADO por un DROP o JOURNAL");

			return;
		}
	}
	aux_aux_segmento->siguienteSegmento = aux_segmentos->siguienteSegmento;
	free(aux_segmentos->path_tabla);
	free(aux_segmentos);

	log_info(log_memoria, "[BORRANDO 1 SEGMENTO]\n Segmento <%s> BORRADO", unSegmento->path_tabla);
}



pagina* crear_pagina(int16_t key, char * valor, int posAsignada) {
	log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Por crear pagina y valor");
	pagina* pag = malloc(sizeof(pagina));
	pag->nroPosicion = posAsignada;
	pag->key = key;
//	aux_pagina->timestamp = timestamp();
	//SI TIENE TIMESTAMP EN 0 LE ASIGNAMOS EL ACTUAL
	double algo =timestamp();
	pag->timestamp = algo;

	log_info(log_memoria, "[CREANDO PAGINA Y VALOR]KEY|VALOR|TIMESTAMP: %d|%s|%f", key, valor, algo);
	log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Pagina creada y tambien su valor");
	return pag;
}

void incrementarAccesoDeKey(int pos){
	log_info(log_memoria, "[INCREMENTAR ACCESO EN 1] Entrando, modifico en tabla LRU el timestamp de '%d'"
			, pos);

	modificar_bloque_LRU("", timestamp(), pos, false, false);

	return;
}

//ESTE, CADA VEZ QUE SE LO INVOCA SE DEBE PONER UN SEMAFORO
void obtenerInfoDePagina(int i, void** informacion){
	log_info(log_memoria, "[OBTENIENDO DATOS] Empiezo a obtener datos de pagina '%d'", i);

	mutexBloquear(&mutex_memoria);
	memcpy(*informacion, bloque_memoria+i*(sizeof(pagina)+max_valor_key), sizeof(pagina)+max_valor_key);
	log_info(log_memoria, "[OBTENIENDO DATOS] Obtuve datos de pagina '%d'", i);
	return;
}

char* obtenerNombreTablaDePath(char* path){
	int i;
	int posUltimoBarra=0;
	log_info(log_memoria, "[Obteniendo nombre de la tabla] ENTRANDO");
	for(i=0; i < stringLongitud(path); i++){
		//ME PONGO A BUSCAR EL ULTIMO /
		if(strcmp(path[i], '/')){
			posUltimoBarra=i+1;
		}
	}
	//YA OBTUVE LA POSICION DE LA ULTIMA BARRA
	int longitudDeNombre = stringLongitud(path)+1 - posUltimoBarra;
	char nombre [longitudDeNombre];
	for(i=0; i < longitudDeNombre; i++) {
		nombre[i] = path[posUltimoBarra+i];
	}
	return nombre;
}

//SE CREA UN SEGMENTO NUEVO CON 1 PAGINA ASOCIADA Y EL NOMBRE DE LA TABLA TAMBIEN
segmento* segmento_crear(char* pathNombreTabla, pagina_referenciada* paginaRef) {
	log_info(log_memoria, "[CREANDO SEGMENTO] Creando segmento para la tabla '%s' asociando al nro de pagina '%d'",
			pathNombreTabla, paginaRef->nropagina);
	segmento* auxSeg = malloc(sizeof(segmento));
	auxSeg->path_tabla = (char*)malloc(strlen(pathNombreTabla)+1);
	auxSeg->paginasAsocida = paginaRef;
	log_info(log_memoria, "[CREANDO SEGMENTO] Estructura de la nueva tabla creada"),

//	memcpy(auxSeg->path_tabla, pathNombreTabla, strlen(pathNombreTabla));
	strcpy(auxSeg->path_tabla, pathNombreTabla);
	log_info(log_memoria, "[CREANDO SEGMENTO] PATH COPIADO '%s'", auxSeg->path_tabla);

	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[CREANDO SEGMENTO] ES EL PRIMER SEGMENTO EN SER CREADO '%s'", pathNombreTabla);
		//CREO EL PRIMER SEGMENTO DE LA TABLA

		auxSeg->siguienteSegmento=NULL;

	} else {
		log_info(log_memoria, "[CREANDO SEGMENTO] CREANDO OTRO SEGMENTO A LA TABLA SEGMENTOS '%s'", pathNombreTabla);
		mutexBloquear(&mutex_segmento_modificando);
		auxSeg->siguienteSegmento=tablaSegmentos;
		mutexDesbloquear(&mutex_segmento_modificando);
	}
	log_info(log_memoria, "[CREANDO SEGMENTO]SEGMENTO '%s' CREADO!!!", pathNombreTabla);
	return auxSeg;
}

void segmento_asociar_nueva_pagina(segmento* unSegmento, pagina_referenciada* ref){
	mutexBloquear(&mutex_segmento_modificando);
	unSegmento->cantPaginasAsociadas +=1;
	int cantidad = unSegmento->cantPaginasAsociadas;
	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Por asociar la pagina '%d' al segmento '%s'",
			ref->nropagina, unSegmento->path_tabla);

	mutexBloquear(&mutex_pagina_referenciada_aux);
	aux_tabla_paginas = unSegmento->paginasAsocida;
	while(aux_tabla_paginas->sig!=NULL){
		//ME PONGO A BUSCAR LA ULTIMA POSICION
		aux_tabla_paginas = aux_tabla_paginas->sig;
	}
	aux_tabla_paginas->sig = ref;
	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Segmento '%s' se le ha asignado con exito el nro de pagina %d",
				unSegmento->path_tabla, aux_tabla_paginas->nropagina);
	mutexDesbloquear(&mutex_pagina_referenciada_aux);
	mutexDesbloquear(&mutex_segmento_modificando);
}

void segmento_eliminar_nro_pagina(segmento* unSegmento, int nroAQuitar){

	log_info(log_memoria, "[ELIMINANDO PAGINA A SEGMENTO] Sacando nro de pagina '%d' del segmento '%s'",
			nroAQuitar, unSegmento->path_tabla);

	mutexBloquear(&mutex_segmento_aux);
	mutexBloquear(&mutex_segmento_modificando);
	//BUSCO NRO DE PAGINA->LA ELIMINO -> ACOMODO LA LISTA
	pagina_referenciada* tablaPaginaAux;
	pagina_referenciada* tablaPaginaBuscador;
	tablaPaginaBuscador = unSegmento->paginasAsocida;
	tablaPaginaAux = NULL;
//	printf("\nPASO A COMPARAR NUMEROS\n\n");
	while(tablaPaginaBuscador->nropagina != nroAQuitar){
	//	printf("\nNRO DEL SEGMENTO vs NROAQUITAR\n%d                ==      %d\n\n", tablaPaginaBuscador->nropagina, nroAQuitar);

		tablaPaginaAux = tablaPaginaBuscador;
		tablaPaginaBuscador = tablaPaginaBuscador->sig;
	}
//	printf("\nENCONTRADO\n%d == %d\n\n", tablaPaginaBuscador->nropagina, nroAQuitar);
	log_info(log_memoria, "\n[ELIMINANDO PAGIAN A SEGMENTO] Procedo a eliminar la tabla de pagina encontrada de: [%d]",
			tablaPaginaBuscador->nropagina);
	tablaPaginaAux = tablaPaginaBuscador->sig;
	mutexBloquear(&mutex_bitmap);
	liberarPosicionLRU(tablaPaginaBuscador->nropagina);
	mutexDesbloquear(&mutex_bitmap);
	free(tablaPaginaBuscador);

	if(tablaPaginaAux==NULL){
		log_info(log_memoria,
				"[ELIMINANDO PAGINA A SEGMETNO] \nElimiando Segmento <%s> porque se quedo sin tabla de pagina",
			unSegmento->path_tabla);
		borrarSegmentoPasadoPorParametro(unSegmento);

		} else {
			unSegmento->paginasAsocida = tablaPaginaAux;
			log_info(log_memoria,
			"[ELIMINANDO PAGINA A SEGMENTO] NRO de pagina '%d' eliminada del segmento '%s'",
					nroAQuitar, unSegmento->path_tabla);
		}

	mutexDesbloquear(&mutex_segmento_modificando);
	mutexDesbloquear(&mutex_segmento_aux);
}

pagina_a_devolver* segmentoBuscarInfoEnTablaDePagina(char* nombreTabla,
	u_int16_t key, bool comandoInsert){

	log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] Por buscar KEY '%d' y PATH '%s'",
			key, nombreTabla);
	segmento* aux = tablaSegmentos;
	while(aux!=NULL){
	//	if(strcmp(nombreTabla, obtenerNombreTablaDePath(aux->path_tabla))){
			if(strcmp(nombreTabla, aux->path_tabla)){
			int nroDePagina;
			log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', voy a buscar la key",
					nombreTabla);
			nroDePagina = buscarEntreTodasLasTablaPaginasLaKey(aux->paginasAsocida, key);
		//	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
			if(nroDePagina>-1){
				log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', KEY encontrada '%d'",
						nombreTabla, key);
				if(comandoInsert){
					//SE DEBE REALIZAR MODIFICACIONES
		//			funcionInsert(nombreTabla, 0, key, valorAPoner);
					return NULL;
				} else {
					//SOLO SE DEBE CONSULTAR LOS DATOS
					return selectObtenerDatos(nroDePagina, true);
				}
			}

		}
	}
	return NULL;

}

pagina_a_devolver* selectObtenerDatos(int nroDePaginaAIr, bool necesitoValue){
	log_info(log_memoria, "[Obtener Datos] Entrando a obtener datos");
	pagina_a_devolver* pag = malloc(sizeof(pagina_a_devolver));
	pagina* pag_con_datos = malloc(sizeof(pagina));

	void* informacion = malloc(sizeof(pagina)+max_valor_key);
	log_info(log_memoria, "[Obtener Datos] Me pongo a buscar la pagina requerida");
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
	log_info(log_memoria, "[Obtener Datos] KEY PAGINA OBTENIDA '%d'", pag->key);
//	log_info(log_memoria, "[Obtener Datos] VALUE OBTENIDO '%s'", pag->value);
//	mutexDesbloquear(&mutex_pagina_auxiliar);
	free(pag_con_datos);
	free(informacion);
	log_info(log_memoria, "[Obtener Datos] Saliendo de obtener datos");
	return pag;
}

pagina_a_devolver* selectPaginaPorPosicion(int posicion, bool deboDevolverEsteValor){
	log_info(log_memoria, "[SELECT] Por acceder a la memoria a la posicion '%d'", posicion);
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);

	void* info = malloc(sizeof(pagina)+max_valor_key);

	obtenerInfoDePagina(posicion, &info);

	pagina* pag = malloc(sizeof(pagina));


	memcpy(pag, info, sizeof(pagina));
	log_info(log_memoria, "[OBTENIENDO DATOS] Incremento el valor de Acceso para la pagina con key '%d'", pag->key);
	if(deboDevolverEsteValor){
		incrementarAccesoDeKey(pag->nroPosicion);
	}
	memcpy(bloque_memoria+posicion*(sizeof(pagina)+max_valor_key), pag, sizeof(pagina));
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Cantidad de accesos de la pagina NRO '%d' de la key '%d' ACTUALIZADA",
			posicion, pag->key);

//	memcpy(bloque_memoria+posicion*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1,valorAPoner, max_valor_key);
	pagina_a_devolver* devolver = malloc(sizeof(pagina_a_devolver));
	devolver->value=malloc(max_valor_key);
	devolver->key = pag->key;
	devolver->nroPosicion= pag->nroPosicion;
	devolver->timestamp=pag->timestamp;
	memcpy(devolver->value, info+sizeof(pagina)-1, max_valor_key);
	printf("DATO QUE OBTUVE DE INFO: <%s>\n", devolver->value);
	log_info(log_memoria, "[SELECT] Datos obtenidos de la posicion '%d'", posicion);
	free(pag);
	mutexDesbloquear(&mutex_memoria);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);

	return devolver;
}

//ESTO SOLO SIRVE CUANDO SE INSERTAN NUEVOS ELEMENTOS A MEMORIA
void tabla_pagina_crear(
		u_int16_t key, char* valor, bool flag_modificado,
		pagina_referenciada** devolver, char* nombreTabla,
		bool existeSegmento, segmento* segmetnoApuntado) {
	log_info(log_memoria, "[Crear Tabla y pagina] En crear Tabla de pagina y pagina nueva porque no estan con la key %d", key);

	pagina_referenciada* pag_ref = malloc(sizeof(pagina_referenciada));

	pag_ref->sig=NULL;
	int posicionAsignada = -1;
	//COMENTADO POR POSIBLE DEADLOCK
//	mutexBloquear(&mutex_pagina_auxiliar);
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);

	log_info(log_memoria, "[Crear Tabla y pagina] Tabla pagina creada");

		//MIENTRAS EST{E BLOQUEADO, NO SE PUEDE QUITAR O PONER NUEVAS PAGINAS
	log_info(log_memoria,
			"[Crear Tabla y pagina] Verifico si hay espacio libre en Memoria");
	posicionAsignada = buscarPaginaDisponible(key, existeSegmento, nombreTabla, segmetnoApuntado);
	if(posicionAsignada==-1){
		log_info(log_memoria,
				"[Crear Tabla y pagina] NO hay espacio libre por lo tanto activo el LRU");
		//SIGNIFICA QUE LLEGO AL TOPE DE PAGINAS EN MEMORIA
		//O SEA ESTAN TODAS OCUPADAS, APLICO LRU
		mutexBloquear(&LRUMutex);
		LRU(crear_pagina(key, valor, -1), &posicionAsignada, valor, flag_modificado, nombreTabla);

		pag_ref->nropagina=posicionAsignada;
	//	printf("\nNUMERO DE PAGINA ASIGNADA:\n<%d>\n", pag_ref->nropagina);
		mutexDesbloquear(&LRUMutex);
		log_info(log_memoria,
				"[Crear Tabla y pagina] LRU FINALIZADO, posicion de pagina NRO: %d",
				posicionAsignada);

	} else {

		log_info(log_memoria,
			"[Crear Tabla y pagina] La pagina '%d' esta LIBRES y la usare para guardar la pagina",
				posicionAsignada);
		//AUN HAY ESPACIO, GAURDO ESTA NUEVA PAGINA EN ALGUNA POSICION LIBRE
	//	pag_ref->nropagina=posicionAsignada;
		log_info(log_memoria,
				"[Crear Tabla y pagina] Asigno el espacio libre a la tabla pagina y a la pagina");
	//	aux_pag = actualizarPosicionAPagina(aux_crear_pagina, posicionAsignada);
		log_info(log_memoria,
				"[Crear Tabla y pagina] Procedo a asignar la tabla y la pagina a dicha posicion");

		asignarNuevaPaginaALaPosicion(posicionAsignada, crear_pagina(key, valor, posicionAsignada),
				valor, flag_modificado, nombreTabla);
		pag_ref->nropagina=posicionAsignada;

		log_info(log_memoria,
				"[Crear Tabla y pagina] ASIGNACION COMPLETADA");
	}
	log_info(log_memoria,
			"[Crear Tabla y pagina] Desactivo el mutex mutex_tabla_pagina_en_modificacion");
	memcpy(*devolver, pag_ref, sizeof(pagina_referenciada));
	imprimirAviso3(log_memoria, "\nNUMERO DE PAGINA PARA LA TABLA Y KEY ASIGNADA:\nTABLA [%s]\nKEY: [%d]\nPAGINA: [%d",
					nombreTabla, key, pag_ref->nropagina);

	free(pag_ref);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
//	mutexDesbloquear(&mutex_pagina_auxiliar);
	return;
}

void LRU(pagina* paginaCreada, int* nroAsignado, char* valor, bool flag_modificado,
		char* nombreTabla){
	mutexBloquear(&ACCIONLRU);
//	imprimirAviso(log_memoria, "[LRU] Comenzando el LRU, empiezo a buscar la pagina a reemplazar");
	log_info(log_memoria, "[LRU] Comenzando el LRU, empiezo a buscar la pagina a reemplazar");


	int candidatoAQuitar = -1;
	log_info(log_memoria, "[LRU] Busco la key entre los segmentos y tabla de paginas");
	char* nombreTablaQueDeboBuscar = malloc(tamanioPredefinidoParaNombreTabla);
	candidatoAQuitar = buscarEnBloqueLRUElProximoAQuitar(&nombreTablaQueDeboBuscar);
//	printf("\n\nLINEA 1925: NOMBRE QUE TENGO QUE BUSCAR: %s  -  %d\n", nombreTablaQueDeboBuscar, candidatoAQuitar);

	if(candidatoAQuitar<0){
				log_info(log_memoria, "[LRU sin candidato] NO hay nada que se puede quitar, por lo tanto se fuerza un JOURNAL");
				mutexBloquear(&JOURNALHecho);
				JOURNAL();
				log_info(log_memoria, "[LRU sin candidato] JOURNAL HECHO, lo asigno a la primera posicion");
				paginaCreada->nroPosicion=0;
				asignarNuevaPaginaALaPosicion(0, paginaCreada, valor, flag_modificado, nombreTabla);
				mutexDesbloquear(&JOURNALHecho);
			} else {
				log_info(log_memoria, "[LRU con candidato] Pondre la pagina en la posicion a reemplazar: %d", candidatoAQuitar);
				paginaCreada->nroPosicion=candidatoAQuitar;
	/*			printf("\n\n[QUE DEBO SACAR]\nSEGMENTO A QUITAR: %s\nPOSICION A SACAR: %d\n\n\n",
						nombreTablaQueDeboBuscar,
						candidatoAQuitar);
						*/
				segmento_eliminar_nro_pagina(
					buscarSegmentoPorNombreTabla(nombreTablaQueDeboBuscar),
						candidatoAQuitar);
		//		printf("\n\nPOR AQUI PUTO\n\n\n");
				asignarNuevaPaginaALaPosicion(candidatoAQuitar, paginaCreada,
						valor, flag_modificado, nombreTabla);

				*nroAsignado = candidatoAQuitar;

				log_info(log_memoria, "[LRU con candidato] LRU TERMINADO");

			}
	free(nombreTablaQueDeboBuscar);
	mutexDesbloquear(&ACCIONLRU);

}

void limpiar_todos_los_elementos_de_1_segmento(segmento* segmentoABorrar){
	//ESTO SE DEBE REVISAR, TIENE ERRORES
	log_info(log_memoria,
			"[LIBERAR SEGMENTO] Liberando todas las tablas y paginas del segmento y a si mismo '%s'",
			segmentoABorrar->path_tabla);
	liberarTodosLasTablasDePaginas(segmentoABorrar->paginasAsocida);
	free(segmentoABorrar);
	log_info(log_memoria, "[LIBERAR SEGMENTO] SEGMENTO LIBERADO");
}

void liberarTodosLasTablasDePaginas(pagina_referenciada* ref){
	pagina_referenciada* refaux;
	void* info = malloc(sizeof(pagina)+max_valor_key);
	mutexBloquear(&mutex_bitmap);
	while(ref!=NULL){
		refaux = ref->sig;
	//	retardo_memoria(arc_config->retardo_mem);
		memcpy(bloque_memoria+(ref->nropagina)*(sizeof(pagina)+max_valor_key), info, sizeof(pagina)+max_valor_key);
		liberarPosicionLRU(ref->nropagina);
		log_info(log_memoria, "[LIBERAR SEGMENTO] TABLA DE LA PAGINA NRO '%d' LIBERADA", ref->nropagina);
		free(ref);
		ref = refaux;
	}
	mutexDesbloquear(&mutex_bitmap);
	free(info);
	log_info(log_memoria, "[LIBERAR SEGMENTO] TABLA DE LA PAGINA LIBERADO POR COMPLETO");
}

void liberarPosicionLRU(int posicionAIr) {

	void* info = malloc(sizeof(nodoLRU));
	memcpy(bloque_LRU+posicionAIr*sizeof(nodoLRU), info, sizeof(nodoLRU));
	bitmapLiberarBit(bitmap, posicionAIr);
	log_info(log_memoria, "[LIBERAR SEGMENTO EN LRU]"
			"BLOQUE DE LA LRU EN LA POSICION NRO '%d' LIBERADA", posicionAIr);

}

void liberar_todo_segmento(){
	segmento* aux;
	log_info(log_memoria, "[LIBERAR SEGMENTO] BLOQUEO EL MUTEX mutex_segmento_en_modificacion PARA QUE NADIE PUEDA AGREGAR MIENTRAS");
	mutexBloquear(&mutex_segmento_en_modificacion);
	log_info(log_memoria, "[LIBERAR SEGMENTO] EMPIEZO A LIBERAR TODOS LOS SEGMENTOS");
	while(tablaSegmentos!=NULL){
		aux = tablaSegmentos->siguienteSegmento;
		log_info(log_memoria, "[LIBERAR SEGMENTO] LIBERANDO SEGMENTO DE TABLA '%s'", tablaSegmentos->path_tabla);
		liberarTodosLasTablasDePaginas(tablaSegmentos->paginasAsocida);
		free(tablaSegmentos->path_tabla);
		free(tablaSegmentos);
		tablaSegmentos = aux;
		log_info(log_memoria, "[LIBERAR SEGMENTO] SEGMENTO LIBERADO");
	}

	mutexDesbloquear(&mutex_segmento_en_modificacion);
	log_info(log_memoria, "[LIBERAR SEGMENTO] DESBLOQUEO EL MUTEX mutex_segmento_en_modificacion PARA MODIFICACIONES");
}

void liberar_toda_tabla_paginas(pagina_referenciada* pag){
	pagina_referenciada* aux;
	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
	while(pag!=NULL){
		aux = pag->sig;
		log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
		free(pag);
		pag = aux;
	}
	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] TERMINADO");
}

void vaciar_tabla_paginas_y_memoria(){
	pagina* pag;
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


}



void modificarValoresDeTablaYMemoriaAsociadasAKEY(int posAIr, char* valorNuevo, int nroPosicion) {
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	mutexBloquear(&mutex_memoria);
	pagina* aux = malloc(sizeof(pagina));
//	char valorString[max_valor_key];
	log_info(log_memoria, "[Modificar valores de pagina] Entrando");
	memcpy(aux, bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key), sizeof(pagina));
//	printf("1 hecho\n");

	aux->timestamp = timestamp();
	log_info(log_memoria, "[Modificar valor pagina] Incremento el acceso a la pagina '%d' de la key '%d'", posAIr, aux->key);
	incrementarAccesoDeKey(posAIr);

	//strcpy(valorString, valorNuevo);

	log_info(log_memoria,
"[Modificar valor pagina] Pagina modificada con key '%d' VALORES NUEVOS;  TIMESTAMP '%f'; VALOR '%s'",
											aux->key, aux->timestamp, valorNuevo);

	log_info(log_memoria,
			"[MOdificar valor pagina] Guardando los datos actualizados la pagina con key: %d",
			aux->key);


//	printf("\n\nEN MODIFICACION NUEVO TIMESTAMP: %d - %f\n\n", aux->key, aux->timestamp);


	memcpy(bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key), aux, sizeof(pagina));
	memcpy(bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1, valorNuevo, max_valor_key);
	log_info(log_memoria,
			"[MOdificar valor pagina] key: '%d', VALOR NUEVO: %s",
			aux->key, valorNuevo);

	modificar_bloque_LRU("xs", aux->timestamp, nroPosicion, true, false);

	log_info(log_memoria,
			"[MOdificar valor tabla pagina] Actualizar FLAG de tabla pagina asociada a la key: %d",
			aux->key);
//	actualizarFlagDeLaKey(aux->key);

	log_info(log_memoria,
"[MOdificar valor tabla pagina] FLAG ACTUALIZADO EN MODIFICADO PARA LA TABLA DE LA KEY|NRO DE PAGINA: %d|%d",
	aux->key, nroPosicion);

	log_info(log_memoria,
			"[Modificar valor pagina] Se ham modificado el FLAG de la tabla KEY|NRO DE PAGINA: %d|%d",
			aux->key,nroPosicion);
	free(aux);
	log_info(log_memoria,
			"[Modificar valor pagina] Desbloqueo el MUTEX mutex_tabla_pagina_en_modificacion");
	log_info(log_memoria,
			"[Modificar valores de pagina] Saliendo");
	mutexDesbloquear(&mutex_memoria);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
}


void modificar_bloque_LRU(char* nombreTabla, double timestamp, int nroPosicion, bool estado,
		bool vieneDeFuncionInsert)
{
//	mutexBloquear(&LRUMutex);
	log_info(log_memoria, "[MODIFICAR BLOQUE LRU]\nActualizar bloque LRU");
	nodoLRU* nuevoNodo = malloc(sizeof(nodoLRU));
	int desplazamiento = sizeof(nodoLRU)+tamanioPredefinidoParaNombreTabla;
	if(vieneDeFuncionInsert){
//		nuevoNodo->nombreTabla = malloc(strlen(nombreTabla));
//		memcpy(nuevoNodo->nombreTabla, nombreTabla, strlen(nombreTabla)+1);
	//	strcpy(nuevoNodo->nombreTabla, nombreTabla);
		nuevoNodo->estado=estado;
		nuevoNodo->nroPagina=nroPosicion;
		nuevoNodo->timestamp=timestamp;
		memcpy(bloque_LRU+nroPosicion*desplazamiento, nuevoNodo, sizeof(nodoLRU));
		memcpy(bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU), nombreTabla,
					strlen(nombreTabla)+1);
		/*
		printf("\n\n\nNOMBRE TABLA INGRESADA: <<<%s>>>", nombreTabla);
		char* auxnombre = malloc(tamanioPredefinidoParaNombreTabla);
		memcpy(auxnombre, bloque_LRU+nroPosicion*desplazamiento+sizeof(nodoLRU),
				tamanioPredefinidoParaNombreTabla);
		printf("\n\n\nNOMBRE TABLA OBTENIDA: <<<%s>>>", auxnombre);
		printf("[MODIFICAR BLOQUE LRU]\nDATOS INGRESADOS:\nNOMBRE TABLA <%s>\nNUMERO PAGINA: <%d>\nTIMESTAMP: <%f>\nESTADO PAGINA: <%d>",
				nombreTabla, nroPosicion, timestamp, estado);

		free(auxnombre);
		*/
		log_info(log_memoria, "[MODIFICAR BLOQUE LRU]\nDATOS INGRESADOS:\nNOMBRE TABLA <%s>\nNUMERO PAGINA: <%d>\nTIMESTAMP: <%f>\nESTADO PAGINA: <%d>",
				nombreTabla, nroPosicion, timestamp, estado);
	//	free(nuevoNodo->nombreTabla);
	} else {
		memcpy(nuevoNodo, bloque_LRU+nroPosicion*sizeof(nuevoNodo), sizeof(nodoLRU));
		nuevoNodo->timestamp=timestamp;
		if(estado==true){
			nuevoNodo->estado = estado;
		}

		memcpy(bloque_LRU+nroPosicion*desplazamiento, nuevoNodo, sizeof(nodoLRU));
		log_info(log_memoria, "[MODIFICAR BLOQUE LRU]\nDATOS ACTUALIZADOS DE LA POSICION: <%d>",
				nroPosicion);
	}
//	free(nuevoNodo->nombreTabla);
	free(nuevoNodo);
//	mutexDesbloquear(&LRUMutex);
}
