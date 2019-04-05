/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"

int main() {

	log_lfilesystem = archivoLogCrear(LOG_PATH, "Proceso Lissandra File System");

	log_info(log_memoria,
			"Ya creado el Log, coninuamos cargando la estructura de configuracion.");

	cargarConfiguracion();

	return 0;

}

void cargarConfiguracion() {

	log_info(log_lfilesystem,
			"Por reservar memoria para variable de configuracion.");

	t_lfilesystem_config* arc_config = malloc(sizeof(t_lfilesystem_config));

	t_config* configFile;

	log_info(log_lfilesystem,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_LFILESYSTEM_CONFIG);

	if (configFile != NULL) {
		/*
		 * ESTO NO ESTA PROBADO, PUEDEN MODIFICARLO
		 *
		log_info(log_lfilesystem, "LissandraFS: Leyendo Archivo de Configuracion...");

		log_info(log_lfilesystem, "Almacenando el puerto");

		arc_config->puerto_escucha = config_get_int_value(configFile,
				"puerto_escucha");

		log_info(log_kernel, "El puerto es: %d", arc_config->puerto_escucha);
		*/
	}

}

