#define LFILESSYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../../Biblioteca/src/Biblioteca.c"

#define PATH_LFILESYSTEM_CONFIG "LFILESSYSTEM.txt"
#define LOG_PATH "logLFILESSYSTEM.txt"

t_log* log_lfilesystem;

typedef struct{

	int puerto_escucha;
	char* punto_montaje;
	int retardo;
	int tamaño_value;
	int tiempo_dump;


}t_lfilesystem_config;

t_lfilesystem_config* configFile;


void cargarConfiguracion();


#endif /* LFILESSYSTEM_H_ */
