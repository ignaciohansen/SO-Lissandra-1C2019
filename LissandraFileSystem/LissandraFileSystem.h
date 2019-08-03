#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include "InotifyLFS.h"
#include "lfsComunicacion.h"

//Agregadas para directorio
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define PRUEBAS
#ifdef PRUEBAS
	#define LOG_PATH "../Log/LOG_LFS.txt"
	#define PATH_LFILESYSTEM_CONFIG "../Config/LFS_CONFIG.txt"
#else
	#define LOG_PATH "/home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Log/LOG_LFS.txt"
	#define PATH_LFILESYSTEM_CONFIG "/home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Config/LFS_CONFIG.txt"
#endif

//ESTOS CAMBIOS LOS HICE YA QUE TENGO LOS ARCHIVOS EN OTRA RUTA

#define PATH_BIN ".bin"
#define PATH_TMP ".tmp"
#define PATH_BLOQUES "/Bloques/"

#define PATH_LFILESYSTEM_METADATA "/Metadata/Metadata"
#define PATH_LFILESYSTEM_BITMAP "/Metadata/Bitmap.bin"

#define TABLE_PATH "/Tables/"


#define atoa(x) #x

t_log* logger;

typedef struct{	
	char* ip;
	int puerto_escucha;
	char* punto_montaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;
}t_lfilesystem_config;

typedef enum{
	EST_LEER,
	EST_TIMESTAMP,
	EST_KEY,
	EST_VALUE,
	EST_SEP,
	EST_FIN
}estadoLecturaBloque_t;

t_lfilesystem_config* configFile;
t_list* list_queries;


char** buffer;

int socketEscuchaMemoria, conexionEntrante, recibiendoMensaje;

/*--------------------------------------------------------------------------------------------
 * 									SET UP LISANDRA FILE SYSTEM
 *--------------------------------------------------------------------------------------------
 */
void LisandraSetUP(bool logEnConsola);
bool cargarConfiguracion();
void iniciaabrirServidorLissandra();
void atenderRequest(char* linea);

/*--------------------------------------------------------------------------------------------
 * 									Elementos de consola
 *--------------------------------------------------------------------------------------------
 */

#define MAXSIZE_COMANDO 200
//enum {Select, insert, create, describe, drop, salir};
char* tabla_Path;
void consola();
void menu();



/*--------------------------------------------------------------------------------------------
 * 									Elementos de escucha
 *--------------------------------------------------------------------------------------------
 */
void listenSomeLQL();

/*--------------------------------------------------------------------------------------------
 * 									Estructura metadatas
 *--------------------------------------------------------------------------------------------
 */

typedef struct{
int tam_registro;
char* value;
u_int64_t timestamp;
u_int16_t key;

}t_registroMemtable;

typedef struct{
	char* consistency;
	int particiones;
	int compaction_time;

}t_metadata_tabla;

//t_metadata_tabla* metadata; //La hago variable local, sino va a romper al tener procesos concurrentes

typedef struct{
	int size;
	char** bloques;

}t_particion;

typedef struct{
	int size;
	t_list *bloques;
}t_datos_particion;

typedef struct{
	int size;
	t_list* bloques;

}t_bloquesUsados;

typedef struct{
	int size;
	t_list* bloques;

}t_regArchivoSelect;

t_particion* particionTabla;

typedef struct{
	int blocks;
	int block_size;
	char* magic_number;

}t_metadata_LFS;

t_metadata_LFS* metadataLFS;

sem_t semaforoQueries;
Mutex memtable_mx;
Mutex listaTablasInsertadas_mx;

pthread_mutex_t semaforo;

t_dictionary* memtable;
t_list * listaTablasInsertadas;
t_list* listaRegistrosMemtable;
t_list* listaRegistrosTabla;


/*--------------------------------------------------------------------------------------------
 * 									Elementos de comandos
 *--------------------------------------------------------------------------------------------
 */

typedef struct{
	RWlock rwLockTabla; //*?
	Mutex drop_mx;
}t_sems_tabla;

t_dictionary* dicSemTablas;

RWlock sem_rw_memtable;

t_registroMemtable* comandoSelect(char* tabla, char* key);
int comandoInsertSinTimestamp(char* tabla,char* key,char* value);
int comandoInsert(char* tabla,char* key,char* value,char* timestamp);
int comandoDrop(char* tabla);
int comandoCreate(char* tabla,char* consistencia,char* particiones,char* tiempoCompactacion);
char* comandoDescribeEspecifico(char* tabla);
char* comandoDescribe();


/*--------------------------------------------------------------------------------------------
 * 									Elementos de bitmap
 *--------------------------------------------------------------------------------------------
 */

char* bitmapPath;
t_bitarray* bitarray;
//int bytesAEscribir;
FILE* archivoBitmap;

int existeArchivo(char* path);
void cargarBitmap();
int abrirBitmap();
void crearBitarray();
void persistirCambioBitmap();
int cantBloquesLibresBitmap();
int estadoBloqueBitmap(int bloque);
int ocuparBloqueLibreBitmap(int bloque);
int liberarBloqueBitmap(int bloque);
int obtenerPrimerBloqueLibreBitmap();
int obtenerPrimerBloqueOcupadoBitmap();
int cantidadBloquesOcupadosBitmap();


/*--------------------------------------------------------------------------------------------
 * 									Elementos de dump
 *--------------------------------------------------------------------------------------------
 */


int timestamp_inicio;
//int cantidad_de_dumps = 0;
//int dumps_a_dividir =1;
int indiceTablaParaTamanio;
//int tamanioRegistros[];

void esperarTiempoDump();
char* armarPathTablaParaDump(char* tabla,int dumps);
int cantidadDumpsTabla(char* pathTabla);
int crearArchivoTemporal(char* path,char* tabla);
void realizarDump();

/*--------------------------------------------------------------------------------------------
 * 									Operaciones para bloques
 *--------------------------------------------------------------------------------------------
 */

int tamTotalListaRegistros(t_list* listaRegistros);
int cuantosBloquesNecesito(int tam_total);
void* armarBufferConRegistros(t_list* listaRegistros, int tam_total_registros);
int escribirVariosBloques(t_list* bloques, int tam_total_registros, void* buffer);
int escribirBloque(int bloque, int size, int offset, void* buffer);
t_list* leerBloque(char* path);
t_list* leerBloquesConsecutivos(t_list *nroBloques, int tam_total);
t_registroMemtable *leerBloquesConsecutivosUnaKey(t_list *nroBloques, int tam_total, uint16_t key_buscada, bool es_unica);
void crearBloques();
char* crearPathBloque(int bloque);
int abrirArchivoBloque(FILE **fp, int nroBloque, char *modo);

/*--------------------------------------------------------------------------------------------
 * 									Elementos de archivos temporales
 *--------------------------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------------------------
 * 									Elementos de Compactacion
 *--------------------------------------------------------------------------------------------
 */


typedef struct{
	uint64_t retardo;
	char *tabla;
}args_compactacion_t;

t_dictionary* dicHilosCompactacion;

t_list *obtenerTablas(void);
void iniciarSemaforosCompactacion(void);
void *correrCompactacion(args_compactacion_t *args);
t_datos_particion *obtenerDatosParticion(char *path_particion);
void matarYBorrarHilos(pthread_t *thread);
int compactarTabla(char *tabla);
int guardarRegistrosParticion(char *path_tabla, int particion, t_list *registros_list);
void liberarBloquesDeArchivo(char *path_tabla, char *extension);
void incorporarRegistro(t_list *registros, t_registroMemtable *nuevo);
void actualizarListaRegistros(t_list *listas_registros, t_list *nuevos);


/*--------------------------------------------------------------------------------------------
 * 									Otros
 *--------------------------------------------------------------------------------------------
 */

void INThandler(int sig);

/*void validarComando(char** comando, int tamanio, t_log* logger);

int buscarComando(char* comando, t_log* logger);

void validarLinea(char** lineaIngresada, t_log* logger);*/

t_metadata_tabla* obtenerMetadataTabla(char* tabla);

int obtenerMetadata();

int verificarTabla(char* tabla);

char* retornarValores(char* tabla, t_metadata_tabla* metadata);

char* retornarValoresDirectorio();

void escanearParticion(int particion);

char* buscarBloque(char* key);

void eliminarTablaCompleta(char* tabla);

bool validarKey(char* key);

bool validarValue(char* value);

void validarLinea(char** lineaIngresada);

void validarComando(char** comando, int tamanio);

int buscarComando(char* comando);

char* desenmascararValue(char* value);

void cerrarTodo();

void liberarTodosLosRecursosGlobalesQueNoSeCerraron();

void *imprimirRegistro(t_registroMemtable *reg);

t_registroMemtable* tomarMayorRegistro(t_registroMemtable* reg1,t_registroMemtable* reg2, t_registroMemtable* reg3,t_registroMemtable* reg4);

t_registroMemtable* pruebaRegMayorTime();

t_registroMemtable* armarRegistroNulo();

int pruebaLecturaBloquesConsecutivos(void);

t_list *obtenerArchivosDirectorio(char *path, char *terminacion);

char* rutaParticion(char* tabla, int particion);

t_registroMemtable *crearCopiaRegistro(t_registroMemtable *origen);

void borrarRegistro(t_registroMemtable *reg);

void vaciarMemtable(void);

void borrarListaMemtable(t_list *lista);

bool existeDirectorio(char *path);

void borrar_array_config(char **array);

#endif /* LFILESSYSTEM_H_ */
