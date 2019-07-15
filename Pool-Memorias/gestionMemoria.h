/*
 * gestionMemoria.h
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#ifndef GESTION_MEMORIA_INC
#define GESTION_MEMORIA_INC

//#include "../Biblioteca/src/Biblioteca.c"
#include "estructuras.h"
//#include "memoria.h"

/* FUNCIONES EXTERNAS */

void armarMemoriaPrincipal(void);
int loggearEstadoActual(FILE *fp);
int funcionInsert(char* nombreTabla, u_int16_t keyBuscada, char* valorAPoner, bool estadoAPoner, timestamp_mem_t timestamp_val);
int funcionSelect(char* nombreTablaAIr, u_int16_t keyBuscada,pagina_a_devolver** dato, char** valorADevolver);
int funcionDrop(char* nombre);
int funcionDescribe(char* nombreTablaAIr);
void liberar_todo_por_cierre_de_modulo(void);
void liberar_config(void);

void insertCrearPaginaConNuevoSegmento(char* nombreTabla, u_int16_t keyBuscada,
		pagina_referenciada* ref, char* valorAPoner, bool estadoAPoner,
		segmento* segmentoBuscado, timestamp_mem_t timestamp_val);

/* FUNCIONES INTERNAS A LA BIBLIOTECA */

void modificarValoresDeTablaYMemoriaAsociadasAKEY(int posAIr, char* valorNuevo, timestamp_mem_t timestamp_val);
//timestamp_mem_t timestamp(void);
void liberar_todo_segmento(void);

//ESTE DE AQUI ABAJO DEVUELVE LA POSICION EN DONDE SE ENCUENTA LA KEY BUSCADA
//si no lo encunetra devuelve ERROR O -1
int buscarEnMemoriaLaKey(u_int16_t keyBuscada);
pagina_a_devolver* selectPaginaPorPosicion(int posicion, bool deboDevolverEsteValor);

//int obtener_valores(char* nombreTabla, int16_t key, unidad_memoria* unidadExtra);



/*---------------------------------------------------
 * FUNCIONES PARA ADMINISTRAR LA MEMORIA
 *---------------------------------------------------*/
	void actualizarTiempoUltimoAcceso(int pos, bool estadoAPoner, bool vieneDeInsert);
	void agregarNuevaPaginaALaPosicion(pagina_a_devolver* pagina, int posicion);


	pagina_a_devolver* selectObtenerDatos(int nroDePaginaAIr, bool necesitoValue);
	int buscarEntreTodasLasTablaPaginasLaKey(pagina_referenciada* tablasAsociadasASegmento,	u_int16_t keyBuscada);
	int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey
		(char* nombreTabla, u_int16_t keyBuscada,
			segmento** segmentoBuscado);

	segmento* buscarSegmentoPorNumero(int numeroABuscar);
	segmento* buscarSegmentoPorNombreTabla(char* nombreTabla);
	pagina_referenciada* buscarPaginaPorNumero(int numeroABuscar, segmento* seg);

//	int limpiarUnidad(unidad_memoria* unidad_de_memoria);
	/* @NAME: list_create
	* @DESC: Crea una lista
	*/

	void tabla_pagina_crear(
			u_int16_t key, char* valor, bool flag_modificado,
			pagina_referenciada** devolver, char* nombreTabla,
			bool existeSegmento, segmento* segmetnoApuntado, timestamp_mem_t timestamp_val);
//	pagina * pagina_crear(long timestamp, int16_t key, char * valor);
	pagina* pagina_crear(long timestampNuevo, u_int16_t key, char * valor, char* nombreTabla);
	//EL DE ABAJO CREA LA UNIDAD, EL DE ARRIBA DESPUES MANDA A BUSCAR EL SEGMENTO
	pagina* crear_pagina(int16_t key, char * valor, int posicionAsignada, timestamp_mem_t timestamp_val);


	/*
	 * SEGMENTO
	 */
	segmento* segmento_crear(char* pathNombreTabla, pagina_referenciada* paginaRef);
	void eliminar_nro_pagina_de_segmento(segmento* unSegmento, int nroAQuitar);
	void segmento_destruir(segmento*);
	void segmento_destruir_y_vaciar_elementos(segmento *);
	int limpiar_segmento(segmento * seg);
	int segmento_esta_vacio(segmento *);
	void aniadirNuevaPaginaASegmento(pagina* nuevaPag, char* nombreTabla);
	int buscarKeyPorTablaPagina(pagina_referenciada* tabla_pagina_auxx, u_int16_t keyBuscada);
	void asociar_nueva_pagina_a_segmento(segmento* unSegmento, int posicion);


	//ACCESO A MEMORIA
	void obtenerInfoDePagina(int i, void** informacion);
	void* accederYObtenerInfoDePaginaEnPosicion(int posicion, void* info);

	void tabla_pagina_destruir(pagina_referenciada*);
	void pagina_destruir(pagina*);


	bool chequear_si_memoria_tiene_espacio(int espacioAOcupar);

	void pagina_destruir_y_vaciar_elementos(pagina *);

	void segmento_agregar_pagina(segmento*, pagina_referenciada*);
	void pagina_agregar_valores(pagina_referenciada*, pagina*);
	void pagina_poner_estado_modificado(pagina_referenciada* pag);

	/*
	 * AGREGAR NUEVO ELEMENTO AL PRINCIPIO DE TODO
	 */
	int segmento_agregar_inicio_pagina(segmento*, pagina_referenciada*);
	int pagina_agregar_inicio_valores(pagina_referenciada*, pagina*);




	int limpiar_valores_pagina(pagina* val);
	int limpiar_paginas(pagina_referenciada* pag);
	int limpiar_valores_pagina(pagina* valores);

	void limpiar_y_destruir_todo_segmento(void);
	void limpiar_todos_los_elementos_de_1_segmento(segmento* segmentoABorrar);
	void liberarTodosLasTablasDePaginas(pagina_referenciada* ref);
	void liberarPosicionLRU(int posicionAIr);

	bool chequear_si_memoria_tiene_espacio(int tamanio);

//	int obtener_valores(int16_t key, segmento* segmentoHost, valor_pagina* valorADevolver);
	int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado);

/*
 * NUEVAS FUNCIONES
 */


	void LRU(
	/*		pagina* paginaCreada, int* nroAsignado,
			char* value, bool flag_modificado,	char* nombreTabla
			*/
			);

	pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos);

	char* obtenerNombreTablaDePath(char* path);

	int pasarValoresALisandra(datosJournal* datos, int socket_lfs);

	int buscarPaginaDisponible(u_int16_t key, bool existiaTabla, char* nombreTabla, segmento* segmetnoApuntado);

	void modificar_bloque_LRU(char* nombreTabla, timestamp_mem_t timestamp, int nroPosicion, bool estado,
			bool vieneDeInsert);

	int buscarEnBloqueLRUElProximoAQuitar(char** nombreTablaADevolver);

	void borrarSegmentoPasadoPorParametro(segmento* unSegmento);

	void selectHardcodeado(char* nombreTablaAIr, u_int16_t keyBuscada, void* dato);


void insertHardcodeado(int cant, int inicio, void* info, char* valorNuevo, char* nombreTabla);

/*
 *							JOURNAL
 */

bool verificarSiEstaFUll(void);
int JOURNAL(int socket_lfs);
int procesoJournal(int socket_lfs);
datosJournal* obtener_todos_journal(void);
bool bloque_LRU_en_posicion_fue_modificado(int pos, char** nombreADevolver);
void liberarDatosJournal(datosJournal* datos);
void limpiezaGlobalDeMemoriaYSegmentos(void);
bool bitmapLleno(void);

#endif
