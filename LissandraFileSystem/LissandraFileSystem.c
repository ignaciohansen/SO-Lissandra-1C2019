/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"

int main() {

	LissandraFSInicioLogYConfig();

	return 0;
}

void LissandraFSInicioLogYConfig() {
	pantallaLimpiar();
	imprimirMensajeProceso("Iniciando el modulo LISSANDRA FILE SYSTEM\n");
	log_lfilesystem = archivoLogCrear(LOG_PATH, "Proceso Lissandra File System");

	log_info(log_lfilesystem,
				"Ya creado el Log, coninuamos cargando la estructura de configuracion.");

	cargarConfiguracion();
}

void cargarConfiguracion() {

	log_info(log_lfilesystem,
			"Por reservar memoria para variable de configuracion.");

	t_lfilesystem_config* arc_config = malloc(sizeof(t_lfilesystem_config));

	t_config* configFile;

	log_info(log_lfilesystem,
			"Por crear el archivo de config para levantar archivo con datos.");


	configFile = config_create("../LISANDRAFS.txt");

	if(configFile == NULL)
	{
		imprimirMensajeProceso("NO se ha encontrado el archivo de configuracion\n");
	}

	if (configFile != NULL) {
		int ok = 1;
		imprimirMensajeProceso("Se ha encontrado el archivo de configuracion\n");

		log_info(log_lfilesystem, "LissandraFS: Leyendo Archivo de Configuracion...");

		if(config_has_property(configFile,"PUERTO_ESCUCHA")){

			log_info(log_lfilesystem, "Almacenando el puerto");

			//Por lo que dice el texto
			arc_config->puerto_escucha = config_get_int_value(configFile,
				"PUERTO_ESCUCHA");

			log_info(log_lfilesystem, "El puerto de escucha es: %d", arc_config->puerto_escucha);

		}else {
			log_error(log_lfilesystem,
					"El archivo de configuracion no contiene el PUERTO_ESCUCHA");
			ok--;

		}

		if(config_has_property(configFile,"PUNTO_MONTAJE")){

			log_info(log_lfilesystem, "Almacenando el PUNTO DE MONTAJE");

			//Por lo que dice el texto
			arc_config->punto_montaje = config_get_int_value(configFile,
				"PUNTO_MONTAJE");

			log_info(log_lfilesystem, "El puerto de montaje es: %d", arc_config->punto_montaje);

		}else {
			log_error(log_lfilesystem,
					"El archivo de configuracion no contiene el PUNTO_MONTAJE");
			ok--;

		}

		if(config_has_property(configFile,"RETARDO")){

			log_info(log_lfilesystem, "Almacenando el retardo");

			//Por lo que dice el texto
			arc_config->retardo = config_get_int_value(configFile,
				"RETARDO");

			log_info(log_lfilesystem, "El retardo de respuesta es: %d", arc_config->retardo);

		}else {
			log_error(log_lfilesystem,
					"El archivo de configuracion no contiene RETARDO");
			ok--;

		}

		if(config_has_property(configFile,"TAMANIO_VALUE")){

			log_info(log_lfilesystem, "Almacenando el tamaño del valor de una key");

			//Por lo que dice el texto
			arc_config->tamanio_value = config_get_int_value(configFile,
				"TAMANIO_VALUE");

			log_info(log_lfilesystem, "El tamaño del valor es: %d", arc_config->tamanio_value);

		}else {
			imprimirAviso(log_lfilesystem, "El archivo de configuracion no contiene el TAMANIO_VALUE");
			ok--;

		}

		if(config_has_property(configFile,"TIEMPO_DUMP")){

			log_info(log_lfilesystem, "Almacenando el puerto");

			//Por lo que dice el texto
			arc_config->tiempo_dump = config_get_int_value(configFile,
				"TIEMPO_DUMP");

			log_info(log_lfilesystem, "El tiempo de dumpeo es: %d", arc_config->tiempo_dump);

		}else {
			imprimirAviso(log_lfilesystem, "El archivo de configuracion no contiene el TIEMPO_DUMP");
			ok--;

		}


		if(ok>0) {
			imprimirVerde(log_lfilesystem,"Se ha cargado todos los datos del archivo de configuracion");
		//	log_info(log_lfilesystem, "Se ha cargado todos los datos del archivo de configuracion");

		} else {
			imprimirError(log_lfilesystem, "ERROR: No Se han cargado todos o algunos los datos del archivo de configuracion\n");
	//		imprimirMensajeProceso("ERROR: No Se han cargado todos los datos del archivo de configuracion\n");
		}

	}


}

