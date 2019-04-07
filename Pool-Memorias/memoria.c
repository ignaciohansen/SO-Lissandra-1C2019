/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "memoria.h"

void terminar_memoria(t_log* g_log);

int main() {
    /*
     * 1) Conectar a LFS, hacer handshake: obtener "tamaño máximo de value" p. admin de paginas
     * 2) Iniciar la memoria principal
     * 3) Ejecutar Gossiping
     */

	// LOGGING
	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
	log_info(log_memoria, " \n ========== Iniciación de Pool de Memoria ========== \n \n ");

	// CONFIG
	cargarConfiguracion();

	// Lectura desde consola de Query-LQL de "Pool de Memorias"
	char* comando = lectura_consola();
	log_info(log_memoria,"Se lee de consola la línea: "); log_info(log_memoria,comando);

	comando = stringRemoverVaciosIzquierda(comando); // depurado
	switch (true){
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
	}
	/*
	if (stringContiene(comando,"SELECT")) {
		log_info(log_memoria,"Se encontró comando SELECT");
	} else {
		log_info(log_memoria,"No se encontró ningún comando");
	}
	*/

	//terminar_memoria(log_memoria);
	log_destroy(log_memoria);
	free(log_memoria);
	log_memoria = NULL;

	return 0;

}

char* lectura_consola() {
	char* linea = (char*)readline(">");
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

void terminar_memoria(t_log* g_log) {
	log_destroy(g_log);
	free(g_log);
	g_log = NULL;
}

}

