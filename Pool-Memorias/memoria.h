/*
 * kernel.h
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../Biblioteca/src/Biblioteca.c"

#define PATH_MEMORIA_CONFIG "../Config/MEMORIA.txt"
#define LOG_PATH "../Log/logMEMORIA.txt"
#define MAXSIZE_COMANDO 200
#define ERROR -1

t_log* log_memoria;
int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
int sockeConexionLF, resultado_sendMsj;
int tablaPaginaArmada;
short memoriaArmada;
int tamanio, limpiandoMemoria;
char* linea;
char* argumentosComando;
char** argumentosParseados;
t_header* buffer;

/*---------------------------------------------------------------------------------
 * 								MUTEX Y SEMAFOROS
 ----------------------------------------------------------------------------------*/

Mutex memoria_mutex_paginas_disponibles;
Mutex mutex_segmento_en_modificacion;
Mutex mutex_tabla_pagina_en_modificacion;
Mutex mutex_segmento_modificando;
Mutex mutex_limpiando_memoria;
Mutex mutex_pagina_auxiliar;
Mutex mutex_pagina_referenciada_aux;
Mutex mutex_pagina_referenciada_aux2;
Mutex mutex_segmento_aux;
Mutex LRUMutex;
Semaforo paginasSinUsar; //TIENE CAPACIDAD HASTA PARA cantPaginasTotales
int cantPaginasDisponibles, cantPaginasTotales;


void* bloque_memoria;

int max_valor_key;

typedef struct{
    int puerto;
    char* ip_fs;
    int puerto_fs;
/*
    char* ip_seeds[10];    // esto debe ser flexible
    int puerto_seeds[10];  // esto debe ser flexible
*/
    char* ip_seeds;
    //SACO EL [10] A PUERTO_SEEDS
    char** puerto_seeds;
    int retardo_mem;
    int retardo_fs;
    int tam_mem;
    int retardo_journal;
    int retardo_gossiping;
    int memory_number;

    //NUEVO CAMPO AÑADIDO, MAXIMO VALOR EN BYTE DE LA KEY
    //ESTA SOLO SE OBTIENE A PARTIR DE CONECTAR CON FS
    int max_value_key;
}t_memoria_config;

int a;

typedef struct pagina{
	short nroPosicion;
	long timestamp;
	u_int16_t key;
}pagina;

typedef struct pagina_a_devolver{
	short nroPosicion;
	long timestamp;
	u_int16_t key;
	char* value;
}pagina_a_devolver;

typedef struct pagina_referenciada{
//	int numero;
//	struct pagina* valor_pagina;
//	struct tabla_paginas* siguientePagina;
	int vecesAccedido;
//	u_int16_t key;
	pagina_a_devolver* pag;
	int nropagina; //RELACIONAR UNA PAGINA EN MEMORIA
	u_int16_t key;
	bool flag;
	struct pagina_referenciada* sig;
}pagina_referenciada;

typedef struct nodoSegmento{
	int cantPaginasAsociadas;
	char* path_tabla;
	pagina_referenciada* paginasAsocida;
	struct nodoSegmento* siguienteSegmento;
}segmento;

//memoria_principal* memoria;
segmento* tablaSegmentos;
t_memoria_config* arc_config;

//ESTOS 2 SOLO SE USARAN PARA CARGAR DATOS, NADA MAS
pagina_referenciada* aux_tabla_paginas;
pagina_referenciada* aux_tabla_paginas2;
pagina* aux_pagina;
segmento* aux_segmento;

int main();


/*---------------------------------------------------
 * FUNCIONES PARA MEMORIA PRINCIPAL
 ---------------------------------------------------*/
void armarMemoriaPrincipal();
void* obtenerInfoDePagina(int i, void* informacion);

/*---------------------------------------------------
 * FUNCIONES PARA LA CONFIGURACION Y FINALIZACION
 ---------------------------------------------------*/

void cargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);
void inicioLogYConfig();
void liberar_todo_por_cierre_de_modulo();

/*---------------------------------------------------
 * FUNCIONES PARA LA CONSOLA
 ---------------------------------------------------*/

void ejecutarHiloConsola();
void consola();
void menu();
void validarComando(char*,t_log*);
int buscarComando(char*,t_log*);
int enviarComando(char**, t_log*);

/*---------------------------------------------------
 * FUNCIONES PARA LAS CONEXIONES
 *---------------------------------------------------*/

void crearConexionesConOtrosProcesos();
void levantarServidor();
void conectarConServidorLisandraFileSystem();
void crearHIloEscucharKernel();
void escucharConexionKernel();
void crearHIloEscuchaLFS();


/*---------------------------------------------------
 * PROCESO JOURNAL
 *---------------------------------------------------*/

void JOURNAL(pagina* paginaAPasar, char* path_tabla);

//void pasar_valores_modificados_a_Lisandra(segmento* elSegmento, unidad_memoria* unidad_de_memoria);

/*---------------------------------------------------
 * MODIFICAR TIEMPO DE RETARDO DE CONFIGURACION
 *---------------------------------------------------*/


void modificarTiempoRetardoMemoria(int nuevoCampo);

void modificarTiempoRetardoFileSystem(int nuevoCampo);

void modificarTiempoRetardoGossiping(int nuevoCampo);

void modificarTiempoRetardoJournal(int nuevoCampo);

void modificarTIempoRetardo(int nuevoCampo, char* campoAModificar);

/*---------------------------------------------------
 * FUNCIONES OBTENER VALORES MEDIANTE UNA KEY
 *---------------------------------------------------*/

//int obtener_valores(char* nombreTabla, int16_t key, unidad_memoria* unidadExtra);

/*---------------------------------------------------
 * FUNCIONES PARA ADMINISTRAR LA MEMORIA
 *---------------------------------------------------*/

	pagina_a_devolver* selectObtenerDatos(int nroDePaginaAIr);
	int buscarEntreTodasLasTablaPaginasLaKey(pagina_referenciada* tablasAsociadasASegmento,	int16_t keyBuscada);

	segmento* buscarSegmentoPorNumero(int numeroABuscar);
	segmento* buscarSegmentoPorNombreTabla(char* nombreTabla);
	pagina_referenciada* buscarPaginaPorNumero(int numeroABuscar, segmento* seg);

//	int limpiarUnidad(unidad_memoria* unidad_de_memoria);
	/* @NAME: list_create
	* @DESC: Crea una lista
	*/

	int tabla_pagina_crear(int16_t key, char* valor, bool flag_modificado);
//	pagina * pagina_crear(long timestamp, int16_t key, char * valor);
	pagina* pagina_crear(long timestampNuevo, int16_t key, char * valor, char* nombreTabla);
	//EL DE ABAJO CREA LA UNIDAD, EL DE ARRIBA DESPUES MANDA A BUSCAR EL SEGMENTO
	pagina* crear_pagina(int16_t key, char * valor);


	/*
	 * SEGMENTO
	 */
	void segmento_crear(char* pathNombreTabla, pagina_referenciada* paginaRef);
	void eliminar_nro_pagina_de_segmento(segmento* unSegmento, int nroAQuitar);
	void segmento_destruir(segmento*);
	void segmento_destruir_y_vaciar_elementos(segmento *);
	int limpiar_segmento(segmento * seg);
	int segmento_esta_vacio(segmento *);
	void aniadirNuevaPaginaASegmento(pagina* nuevaPag, char* nombreTabla);
	int buscarKeyPorTablaPagina(pagina_referenciada* tabla_pagina_auxx, int16_t keyBuscada);
	void asociar_nueva_pagina_a_segmento(segmento* unSegmento, int posicion);


	//ACCESO A MEMORIA
	void* obtenerInfoDePagina(int i, void* informacion);
	void* accederYObtenerInfoDePaginaEnPosicion(int posicion, void* info);


	/**
	* @NAME: list_destroy
	* @DESC: Destruye una lista sin liberar
	* los elementos contenidos en los nodos
	*/


	void tabla_pagina_destruir(pagina_referenciada*);
	void pagina_destruir(pagina*);


	bool chequear_si_memoria_tiene_espacio(int espacioAOcupar);

	/**
	* @NAME: list_destroy_and_destroy_elements
	* @DESC: Destruye una lista y sus elementos
	*/


	void pagina_destruir_y_vaciar_elementos(pagina *);

	/**
	* @NAME: list_add
	* @DESC: Agrega un elemento al final de la lista
	*/

	void segmento_agregar_pagina(segmento*, pagina_referenciada*);
	void pagina_agregar_valores(pagina_referenciada*, pagina*);
	void pagina_poner_estado_modificado(pagina_referenciada* pag);

	/*
	 * AGREGAR NUEVO ELEMENTO AL PRINCIPIO DE TODO
	 */
	int segmento_agregar_inicio_pagina(segmento*, pagina_referenciada*);
	int pagina_agregar_inicio_valores(pagina_referenciada*, pagina*);


	void valores_reemplazar_items(pagina* valor_pag, int timestamp, char* valor);
	/**
	* @NAME: list_remove
	* @DESC: Remueve un elemento de la lista de
	* una determinada posicion y lo retorna.
	*/
	void * segmento_sacar(segmento *);
	void * tabla_sacar(segmento *);

	/**
	* @NAME: list_clean
	* @DESC: Quita todos los elementos de la lista.
	* HECHO
	*/

	//DEVUELVEN EL VALOR DEL TAMAÑO DE MEMORIA
	//QUE FUE LIBERADO
//	void limpiar_memoria(memoria_principal* mem);

	int limpiar_valores_pagina(pagina* val);
	int limpiar_paginas(pagina_referenciada* pag);
	int limpiar_valores_pagina(pagina* valores);

	/**
	* @NAME: list_clean
	* @DESC: Quita y destruye todos los elementos de la lista
	*/
	void limpiar_y_destruir_todo_lo_de_segmento(segmento *);


	bool chequear_si_memoria_tiene_espacio(int tamanio);

	//Busca una tabla entre los segmentos de memoria
	//Si no lo encuentra devuelve un ERROR sino solo 1
	int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado);


//	int obtener_valores(int16_t key, segmento* segmentoHost, valor_pagina* valorADevolver);
	int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado);

/*
 * NUEVAS FUNCIONES
 */
	void funcionInsert(char* nombreTabla, int16_t keyBuscada, char* valorAPoner);

	void LRU(pagina* paginaCreada);

	pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos);

	String obtenerNombreTablaDePath(String path);

	void pasarValoresALisandra(char* datos);
#endif /* MEMORIA_H_ */


