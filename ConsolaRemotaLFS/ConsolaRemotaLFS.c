/*
 * ConsolaRemotaLFS.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "ConsolaRemotaLFS.h"

int main() {

	log_cremota = archivoLogCrear(LOG_PATH, "Consola remota para LFS.");
	cargarConfiguracion();
	menu();
	consola();

}

void cargarConfiguracion() {

	crem_config = malloc(sizeof(t_cremota_config));

	t_config* configFile;

	configFile = config_create(PATH_CREMOTA_CONFIG);

	if (configFile != NULL) {

		log_info(log_cremota, "Consola Remota: Leyendo Archivo de Configuracion...");

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

	log_info(log_cremota,"Configuración cargada. Se libera variable de configuración temporal.");

	free(configFile);

}

int conexionConsolaRemota() { // función que retorna el nro del socket listo para escuchar.

	socketCRemota = nuevoSocket(log_cremota);

	if (socketCRemota == ERROR) {
		log_error(log_cremota,"[ERROR] Hubo un problema al querer crear el socket . Salimos del Proceso");
		return ERROR;
	}

	log_info(log_cremota, "PRUEBA: %d ", crem_config->puerto_servidor);

	socketAsociadoCRemota = conectarSocket( socketCRemota
			                        , crem_config->ip_servidor
									, crem_config->puerto_servidor
									, log_cremota);

	if (socketAsociadoCRemota == ERROR) {
		log_error(log_cremota, "[ERROR] Hubo un problema al intentar conectar la consola remot. Salimos del proceso.");
		return -1;
	} else {

		log_info(log_cremota, "[OK] Consola remota conectada. Puerto: %d.", socketAsociadoCRemota);
		return socketCRemota;
	}
} // int conexionConsolaRemota()

void menu() {

	printf("Los comandos que se pueden ingresar son: \n"
			"COMANDOS \n\n"
			"Insert \t"
			"Select \t"
			"Create \t"
			"Describe \t"
			"Drop \t\n"
			"Journal\t"
			"add \t"
			"run \t"
			"metrics \n\n"
			"SALIR \t"
			"\n");
}

void consola() {

	char bufferComando[MAXSIZE_COMANDO];
	char **comandoSeparado;

	while (1) {

		linea = readline(">");

		if(linea){
			add_history(linea);
		}

		if(!strncmp(linea,"exit",4)){
			free(linea);
			break;
		}

		strtok(linea, "\n");

		// (+) Realización del envio
		conexionConsolaRemota();

		sendedMsgStatus = socketEnviar( socketCRemota
				                      , linea
									  , string_length(linea) + 1
				                      , log_cremota);

		if (sendedMsgStatus == ERROR) {
			log_error(log_cremota, "Error al enviar mensaje a memoria. Salimos");
		} else {
			log_info(log_cremota, "El mensaje se envio correctamente");
		}
		// (-) Realización del envio

		free(linea);

	} // while(1)

} // void consola()



