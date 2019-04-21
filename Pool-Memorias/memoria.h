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
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../Biblioteca/src/Biblioteca.c"

#define PATH_MEMORIA_CONFIG "../MEMORIA.txt"
#define LOG_PATH "logMEMORIA.txt"
#define MAXSIZE_COMANDO 200
#define ERROR -1

t_log* log_memoria;
int socketEscuchaKernel,conexionEntrante,recibiendoMensaje;
int sockeConexionLF, resultado_sendMsj;
int tamanio;
char* linea;
char* argumentosComando;
char** argumentosParseados;
t_header* buffer;


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

typedef struct {
	int tamanioMemoria;
	struct nodoSegmento* segmentoMemoria;
} memoria_principal;

typedef struct pagina { //unidad de memoria
	long     timestamp;
	int      key;
	char[15] value;
} pagina_memoria;

typedef struct pagina_t {
		int   nro_pagina;
		char* nombreTabla;
		char* path_tabla;
		bool  modif;
		//DEBE SER LISTA DE PAGINAS
		struct pagina_memoria* puntero_pagina;
		// struct nodoSegmento* siguienteSegmento;
}reg_tabla_pagina;

typedef struct nodoSegmento{
		int   nro_segmento;
		char* nombreTabla;
		char* path_tabla;
		//DEBE SER LISTA DE PAGINAS
		struct reg_tabla_pagina* reg_pagina;
		struct nodoSegmento* siguienteSegmento;
}segmento;

	typedef struct nodo_valor{
		long timestamp;
		int16_t key;
		char* value;
	}valor_pagina;

	typedef struct paginas{
			int numero;
			struct nodo_valor* valor_pagina;
			struct paginas* siguientePagina;
			bool flag;
		}pagina;


memoria_principal* memoria;

t_memoria_config* arc_config;

/*---------------------------------------------------
 * FUNCIONES PARA MEMORIA PRINCIPAL
 ---------------------------------------------------*/
void armarMemoriaPrincipal();

/*---------------------------------------------------
 * FUNCIONES PARA LA CONFIGURACION Y FINALIZACION
 ---------------------------------------------------*/

void cargarConfiguracion();
char* lectura_consola();
void terminar_memoria(t_log* g_log);
void inicioLogYConfig();

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
 * MODIFICAR TIEMPO DE RETARDO DE CONFIGURACION
 *---------------------------------------------------*/


void modificarTiempoRetardoMemoria(int nuevoCampo);

void modificarTiempoRetardoFileSystem(int nuevoCampo);

void modificarTiempoRetardoGossiping(int nuevoCampo);

void modificarTiempoRetardoJournal(int nuevoCampo);

void modificarTIempoRetardo(int nuevoCampo, char* campoAModificar);

/*
 * FUNCIONES PARA ADMINISTRAR LA MEMORIA
 */
/**
	* @NAME: list_create
	* @DESC: Crea una lista
	*/
	segmento * segmento_crear(char* path,  char* nombreTabla,pagina* pag);
	pagina * pagina_crear(valor_pagina* pag, int numero);
	valor_pagina * valor_pagina_crear(int time, int16_t key, char * valor);

	/**
	* @NAME: list_destroy
	* @DESC: Destruye una lista sin liberar
	* los elementos contenidos en los nodos
	*/

	void segmento_destruir(segmento*);
	void pagina_destruir(pagina*);
	void valor_destruir(valor_pagina*);


	bool chequear_si_memoria_tiene_espacio(int espacioAOcupar);

	/**
	* @NAME: list_destroy_and_destroy_elements
	* @DESC: Destruye una lista y sus elementos
	*/

	void segmento_destruir_y_vaciar_elementos(segmento *);
	void pagina_destruir_y_vaciar_elementos(pagina *);

	/**
	* @NAME: list_add
	* @DESC: Agrega un elemento al final de la lista
	*/

	void segmento_agregar_pagina(segmento*, pagina*);
	void pagina_agregar_valores(pagina*, valor_pagina*);
	void pagina_poner_estado_modificado(pagina* pag);

	/*
	 * AGREGAR NUEVO ELEMENTO AL PRINCIPIO DE TODO
	 */
	int segmento_agregar_inicio_pagina(segmento*, pagina*);
	int pagina_agregar_inicio_valores(pagina*, valor_pagina*);


	void valores_reemplazar_items(valor_pagina* valor_pag, int timestamp, char* valor);
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
	void limpiar_memoria(memoria_principal* mem);
	int limpiar_segmento(segmento * seg);
	int limpiar_valores_pagina(valor_pagina* val);
	int limpiar_paginas(pagina* pag);
	int limpiar_valores_pagina(valor_pagina* valores);

	/**
	* @NAME: list_clean
	* @DESC: Quita y destruye todos los elementos de la lista
	*/
	void limpiar_y_destruir_todo_lo_de_segmento(segmento *);


	bool chequear_si_memoria_tiene_espacio(int tamanio);
	/**
	* @NAME: list_iterate
	* @DESC: Itera la lista llamando al closure por cada elemento
	*/
	void list_iterate(t_list *, void(*closure)(void*));

	/**
	* @NAME: list_find
	* @DESC: Retorna el primer valor encontrado, el cual haga que condition devuelva != 0
	*/
	void *list_find(t_list *, bool(*closure)(void*));

	//Busca una tabla entre los segmentos de memoria
	//Si no lo encuentra devuelve un ERROR sino solo 1
	int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado);


	int obtener_valores(int16_t key, segmento* segmentoHost, valor_pagina* valorADevolver);
	int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado);

	/**
	* @NAME: list_is_empty
	* @DESC: Verifica si la lista esta vacia
	*/

	int segmento_esta_vacio(segmento *);


#endif /* MEMORIA_H_ */


