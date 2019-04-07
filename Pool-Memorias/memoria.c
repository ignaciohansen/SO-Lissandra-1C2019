/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "memoria.h"

int main() {

	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");

	log_info(log_memoria,
			"Ya creado el Log, coninuamos cargando la estructura de configuracion.");

	cargarConfiguracion();

	// procesamiento LQL del Pool de Memorias
	char* comando;
	comando = lectura_consola();

	return 0;

}

char* lectura_consola() {
	char* linea;
	linea = readline(">");
	return linea;
}

void cargarConfiguracion() {

	log_info(log_memoria,
			"Por reservar memoria para variable de configuracion.");

	t_memoria_config* arc_config = malloc(sizeof(t_memoria_config));

	t_config* configFile;

	log_info(log_memoria,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_MEMORIA_CONFIG);

	if (configFile != NULL) {
		/*
		 * Esto no esta probado, pueden modificarlo
		 *
		log_info(log_memoria, "Memoria: Leyendo Archivo de Configuracion...");

		log_info(log_memoria, "Almacenando el puerto");


		arc_config->puerto = config_get_int_value(configFile,
				"puerto_escucha");

		log_info(log_kernel, "El puerto es: %d", arc_config->puerto_escucha);
		*/
	}

}

