/*
 * estructuras.h
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */


#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "commons/bitarray.h"
#include <pthread.h>
#include <stdint.h>
#include "../Biblioteca/src/Biblioteca.h"

//#include "parser.h"
//#include "retardos.h"
//#include "memoria.h"

pthread_t journalHilo;

typedef struct{
	int socket;
	pthread_t *hilo;
}cliente_t;

t_log* log_memoria;
FILE* tablas_fp;
t_bitarray* bitmap;

RWlock sem_insert_select;

pthread_mutex_t mutex_retardos_memoria;

pthread_mutex_t memoria_mutex_paginas_disponibles;
pthread_mutex_t mutex_segmento_en_modificacion;
pthread_mutex_t mutex_tabla_pagina_en_modificacion;

pthread_mutex_t mutex_segmento_modificando;
pthread_mutex_t mutex_limpiando_memoria;
pthread_mutex_t mutex_pagina_auxiliar;

pthread_mutex_t mutex_pagina_referenciada_aux;
pthread_mutex_t mutex_pagina_referenciada_aux2;
pthread_mutex_t mutex_segmento_aux;

pthread_mutex_t mutex_crear_pagina_nueva;
pthread_mutex_t LRUMutex, ACCIONLRU;
pthread_mutex_t JOURNALHecho;

pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_bloque_LRU_modificando;

pthread_mutex_t mutex_borrar_datos_de_1_segmento;
pthread_mutex_t mutex_drop;

pthread_mutex_t mutex_info_request;

pthread_mutex_t mutex_bloquear_select_por_limpieza;

pthread_mutex_t verificarSiBitmapLleno;

//RETARDO JOURNAL
bool activo_retardo_journal;


//Semaforo paginasSinUsar; //TIENE CAPACIDAD HASTA PARA cantPaginasTotales
int cantPaginasDisponibles, cantPaginasTotales;
int tamanioPredefinidoParaNombreTabla;


typedef uint64_t timestamp_mem_t;

/*
 * ESTRUCTURAS
 */
typedef struct{
    int puerto;
    char* ip_fs;
    int puerto_fs;
    char* ip;
    char** ip_seeds;
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
	timestamp_mem_t timestamp;
	u_int16_t key;
}pagina;

typedef struct pagina_a_devolver{
	short nroPosicion;
	timestamp_mem_t timestamp;
	u_int16_t key;
	char* value;
}pagina_a_devolver;

typedef struct pagina_referenciada{
//	int numero;
//	struct pagina* valor_pagina;
//	struct tabla_paginas* siguientePagina;
//	int vecesAccedido;
//	u_int16_t key;
//	pagina_a_devolver* pag;
	int nropagina; //RELACIONAR UNA PAGINA EN MEMORIA
//	u_int16_t key;
	struct pagina_referenciada* sig;
}pagina_referenciada;

typedef struct nodoSegmento{
	int cantPaginasAsociadas;
	char* path_tabla;
	pagina_referenciada* paginasAsocida;
	struct nodoSegmento* siguienteSegmento;
}segmento;

typedef struct nodoLRU {
//	char* nombreTabla;
	int nroPagina;
	timestamp_mem_t timestamp;
	bool estado;
}nodoLRU;

typedef struct datosRequest{
	int tamanioReq1;
	char* req1;
	int tamanioReq2;
	char* req2;
	int tamanioReq3;
	char* req3;
}datosRequest;

typedef struct datosJournal{
	char* nombreTabla;
	char* value;
	timestamp_mem_t timestamp;
	u_int16_t key;
	struct datosJournal* sig;
} datosJournal;

//memoria_principal* memoria;
segmento* tablaSegmentos;
t_memoria_config* arc_config;

//ESTOS 2 SOLO SE USARAN PARA CARGAR DATOS, NADA MAS
pagina_referenciada* aux_tabla_paginas;
pagina_referenciada* aux_tabla_paginas2;
pagina* aux_crear_pagina;
pagina_a_devolver* aux_devolver_pagina;
segmento* aux_segmento;

void* bloque_memoria;
void* bloque_LRU;

int max_valor_key;




#endif /* ESTRUCTURAS_H_ */
