#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.c"

#define PATH_LFILESYSTEM_CONFIG "../Config/LFS_CONFIG.txt"
#define LOG_PATH "../Log/LOG_LFS.txt"

t_log* logger;

typedef struct{	
	int puerto_escucha;
	char* punto_montaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;
}t_lfilesystem_config;

typedef struct{
	char* consistency;
	int particiones;
	int compaction_time;

}t_metadata_tabla;



t_lfilesystem_config* configFile;

t_metadata_tabla* metadata;


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
//Comandos
int comandoSelect(char* tabla, int key);

void obtenerMetadata();

int verificarTabla(char* tabla);

/*--------------------------------------------------------------------------------------------
 * 									Elementos de escucha
 *--------------------------------------------------------------------------------------------
 */
void listenSomeLQL();

#endif /* LFILESSYSTEM_H_ */
