/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"

int main() {

	log_kernel = archivoLogCrear(LOG_PATH, "Proceso Kernel");

	log_info(log_kernel,
			"Ya creado el Log, coninuamos cargando la estructura de configuracion.");

	cargarConfiguracion();

	return 0;

}

void cargarConfiguracion() {

	log_info(log_kernel,
			"Por reservar memoria para variable de configuracion.");

	t_kernel_config* arc_config = malloc(sizeof(t_kernel_config));

	t_config* configFile;

	log_info(log_kernel,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_KERNEL_CONFIG);

	if (configFile != NULL) {

		log_info(log_kernel, "Kernel: Leyendo Archivo de Configuracion...");

		log_info(log_kernel, "Almacenando el puerto");

		arc_config->puerto_escucha = config_get_int_value(configFile,
				"puerto_escucha");

		log_info(log_kernel, "El puerto es: %d", arc_config->puerto_escucha);
	}

}

