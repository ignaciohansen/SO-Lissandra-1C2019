#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.c"
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



#define PATH_BIN ".bin"
#define PATH_BLOQUES "/Bloques/"
#define PATH_LFILESYSTEM_CONFIG "../Config/LFS_CONFIG.txt"
#define PATH_LFILESYSTEM_METADATA "/Metadata/Metadata"
#define PATH_LFILESYSTEM_BITMAP "/Metadata/Bitmap.bin"
#define LOG_PATH "../Log/LOG_LFS.txt"
#define TABLE_PATH "../Tables/"

#define atoa(x) #x

t_log* logger;

typedef struct{	
	int puerto_escucha;
	char* punto_montaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;
}t_lfilesystem_config;


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

char *separador2 = "\n";
char *separator = " ";

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

t_dictionary* memtable;
t_list * listaTablasRepetidas;
t_list * listaTablasInsertadas;

/*--------------------------------------------------------------------------------------------
 * 									Elementos de comandos
 *--------------------------------------------------------------------------------------------
 */

char* tablaAverificar; // directorio de la tabla
char* path_tabla_metadata;
char* archivoParticion;
char* registroPorAgregar;

int comandoSelect(char* tabla, char* key);
void comandoInsertSinTimestamp(char* tabla,char* key,char* value);
void comandoInsert(char* tabla,char* key,char* value,char* timestamp);
void comandoDrop(char* tabla);
void comandoCreate(char* tabla,char* consistencia,char* particiones,char* tiempoCompactacion);
void comandoDescribeEspecifico(char* tabla);
void comandoDescribe();


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


int timestamp_inicio;
int cantidad_de_dumps = 0;
int dumps_a_dividir =1;

void esperarTiempoDump();
char* armarPathTablaParaDump(char* tabla);
void crearArchivoTemporal(char* path,char* tabla);
void realizarDump();

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

void retornarValores(char* tabla);

void retornarValoresDirectorio();

void escanearParticion(int particion);

char* buscarBloque(char* key);

void eliminarTablaCompleta(char* tabla);

bool validarKey(char* key);

bool validarValue(char* value);

char* desenmascararValue(char* value);

void cerrarTodo();





#endif /* LFILESSYSTEM_H_ */
