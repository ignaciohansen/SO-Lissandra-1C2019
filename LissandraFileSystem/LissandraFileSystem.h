#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.c"
#include <commons/collections/list.h>

#define PATH_BIN ".bin"
#define PATH_LFILESYSTEM_CONFIG "../Config/LFS_CONFIG.txt"
#define LOG_PATH "../Log/LOG_LFS.txt"
#include <stdio.h>

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

#endif /* LFILESSYSTEM_H_ */

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
	int blocks;
	int block_size;
	char* magic_number;

}t_metadata_LFS;

t_metadata_LFS* metadataLFS;

/*--------------------------------------------------------------------------------------------
 * 									Elementos de comandos
 *--------------------------------------------------------------------------------------------
 */

char* tablaAverificar;

int comandoSelect(char* tabla, char* key);

void obtenerMetadata();

int verificarTabla(char* tabla);

void escanearParticion(int particion);
