#ifndef LFILESSYSTEM_H_
#define LFILESSYSTEM_H_

#include "../Biblioteca/src/Biblioteca.c"

#define PATH_LFILESYSTEM_CONFIG "../LFS_CONFIG.txt"
#define LOG_PATH "LOG_LFS.txt"

t_log* logger;

typedef struct{
	char* ip;
	int puerto_escucha;
	char* punto_montaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;


}t_lfilesystem_config;

t_lfilesystem_config* configFile;
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
void consola();
void menu();

/*--------------------------------------------------------------------------------------------
 * 									Elementos de escucha
 *--------------------------------------------------------------------------------------------
 */
void listenSomeLQL();

#endif /* LFILESSYSTEM_H_ */
