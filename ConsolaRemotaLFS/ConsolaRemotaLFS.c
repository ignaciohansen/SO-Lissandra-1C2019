/*
 * ConsolaRemotaLFS.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "ConsolaRemotaLFS.h"

int main() {

	cargarConfiguracion();

}

void cargarConfiguracion() {

	log_info(log_cremota,
			"Por reservar memoria para variable de configuracion.");

	crem_config = malloc(sizeof(t_cremota_config));

	t_config* configFile;

	log_info(log_cremota,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_CREMOTA_CONFIG);

	if (configFile != NULL) {

		log_info(log_cremota, "Kernel: Leyendo Archivo de Configuracion...");

		if (config_has_property(configFile, "PUERTO_SERVIDOR")) {

			log_info(log_cremota, "[ENCONTRADO] Puerto del servidor.");

			crem_config->puerto_servidor = config_get_int_value(configFile,"PUERTO_SERVIDOR");

			log_info(log_cremota, "[PUERTO] El puerto del servidor es: %d",crem_config->puerto_servidor);

		} else {
			log_error(log_cremota,
					"El archivo de configuracion no contiene el PUERTO de la Memoria");

		}

		if (config_has_property(configFile, "IP_SERVIDOR")) {

			log_info(log_cremota, "[ENCONTRADO] IP del servidor.");

			crem_config->ip_servidor = config_get_string_value(configFile, "IP_SERVIDOR");

			log_info(log_cremota, "[IP] La Ip del servidor es: %s",    crem_config->ip_servidor);

		} else {

			log_error(log_cremota,
					"El archivo de configuracion no contiene la IP de la Memoria");

		}

	} else {

		log_error(log_cremota, "[ERROR] No se encontro el archivo de configuracion para configurar la consola remota.");

	} // if (configFile != NULL)

	log_info(log_cremota,"Cargamos todo lo que se encontro en el archivo de configuracion. Liberamos la variable config que fue utlizada para navegar el archivo de configuracion");

	free(configFile);

}


