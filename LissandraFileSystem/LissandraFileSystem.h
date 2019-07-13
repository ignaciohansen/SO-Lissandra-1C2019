#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>

//Agregadas para directorio
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MARTIN
#ifdef MARTIN
	#define LOG_PATH "../Log/LOG_LFS.txt"
	#define PATH_LFILESYSTEM_CONFIG "../Config/LFS_CONFIG.txt"
#else
	#define LOG_PATH "/home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Log/LOG_LFS.txt"
	#define PATH_LFILESYSTEM_CONFIG "/home/utnso/tp-2019-1c-mi_ultimo_segundo_tp/LissandraFileSystem/Config/LFS_CONFIG.txt"
#endif

#define PATH_BIN ".bin"
#define PATH_TMP ".tmp"
#define PATH_BLOQUES "/Bloques/"

#define PATH_LFILESYSTEM_METADATA "/Metadata/Metadata"
#define PATH_LFILESYSTEM_BITMAP "/Metadata/Bitmap.bin"

#define TABLE_PATH "/Tables/"


#define atoa(x) #x

t_log* logger;

typedef struct{	
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

int socketEscuchaMemoria, conexionEntrante, recibiendoMensaje,tamanio;

/*--------------------------------------------------------------------------------------------
 * 									SET UP LISANDRA FILE SYSTEM
 *--------------------------------------------------------------------------------------------
 */
void LisandraSetUP();
bool cargarConfiguracion();
void iniciaabrirServidorLissandra();


/*--------------------------------------------------------------------------------------------
 * 									Elementos de consola
 *--------------------------------------------------------------------------------------------
 */

#define MAXSIZE_COMANDO 200
//enum {Select, insert, create, describe, drop, salir};
char* linea;
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
	char* consistency;
	int particiones;
	int compaction_time;

}t_metadata_tabla;

t_metadata_tabla* metadata;

typedef struct{
	int size;
	char** bloques;

}t_particion;

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



int comandoSelect(char* tabla, char* key);
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
int bytesAEscribir;
FILE* archivoBitmap;

int existeArchivo(char* path);
void cargarBitmap();
int abrirBitmap();
t_bitarray* crearBitarray();
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

typedef struct{
int tam_registro;
char* value;
double timestamp;
u_int16_t key;

}t_registroMemtable;


int timestamp_inicio;
//int cantidad_de_dumps = 0;
//int dumps_a_dividir =1;
int indiceTablaParaTamanio;
//int tamanioRegistros[];

void esperarTiempoDump();
char* armarPathTablaParaDump(char* tabla,int dumps);
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
void crearBloques();
char* crearPathBloque(int bloque);
int abrirArchivoBloque(FILE **fp, int nroBloque, char *modo);

/*--------------------------------------------------------------------------------------------
 * 									Elementos de archivos temporales
 *--------------------------------------------------------------------------------------------
 */




/*--------------------------------------------------------------------------------------------
 * 									Otros
 *--------------------------------------------------------------------------------------------
 */

int obtenerMetadataTabla(char* tabla);

int obtenerMetadata();

int verificarTabla(char* tabla);

char* retornarValores(char* tabla);

char* retornarValoresDirectorio();

void escanearParticion(int particion);

char* buscarBloque(char* key);

void eliminarTablaCompleta(char* tabla);

bool validarKey(char* key);

bool validarValue(char* value);

char* desenmascararValue(char* value);

void cerrarTodo();

void *imprimirRegistro(t_registroMemtable *reg);

int pruebaLecturaBloquesConsecutivos(void);


#endif /* LFILESSYSTEM_H_ */
