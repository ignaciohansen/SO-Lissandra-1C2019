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
			"Ya creado el Log, continuamos cargando la estructura de configuracion, llamando a la funcion.");

	cargarConfiguracion();

	log_info(log_kernel,
				"Devuelta en Main, Funcion cargarConfiguracion() finalizo.");

	log_info(log_kernel,
				"Antes de llamar a socketCrear.");
	
	socket_CMemoria = nuevoSocket(log_kernel);

	if(socket_CMemoria == ERROR){

		log_error(log_kernel,"Hubo un problema al querer crear el socket desde Kernel. Salimos del Proceso");

		return 0;
	}

	log_info(log_kernel,
				"El Socket creado es: %d .",socket_CMemoria);

	log_info(log_kernel,
				"por llamar a la funcion connectarSocket() para conectarnos con Memoria");

	log_info(log_kernel,"PRUEBA: %d ",arc_config->puerto_memoria);
	resultado_Conectar = conectarSocket(socket_CMemoria, arc_config->ip_memoria, arc_config->puerto_memoria,log_kernel);

	if(resultado_Conectar == ERROR){

		log_error(log_kernel,"Hubo un problema al querer Conectarnos con Memoria. Salimos del proceso");

		return 0;
	}else{

	log_info(log_kernel,
				"Nos conectamos con exito, el resultado fue %d",resultado_Conectar);

	char* msj = malloc(10*sizeof(char));
	msj = "PruebaK\n";
	
	resultado_sendMsj = socketEnviar(socket_CMemoria,msj,strlen(msj),log_kernel);

	if(resultado_sendMsj == ERROR){

		log_error(log_kernel,"Error al enviar mensaje a memoria. Salimos");
		return -1;
	}

	log_info(log_kernel,"El mensaje se envio correctamente");
	}

	log_info(log_kernel,"Salimoooos");

	return 0;

}

void cargarConfiguracion() {
	
	log_info(log_kernel,
			"Por reservar memoria para variable de configuracion.");

	arc_config = malloc(sizeof(t_kernel_config));

	t_config* configFile;

	log_info(log_kernel,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_KERNEL_CONFIG);

	if (configFile != NULL) {

		log_info(log_kernel, "Kernel: Leyendo Archivo de Configuracion...");

		if(config_has_property(configFile,"PUERTO_MEMORIA")){
			
			log_info(log_kernel, "Almacenando el puerto");

			//Por lo que dice el texto
			arc_config->puerto_memoria = config_get_int_value(configFile,
				"PUERTO_MEMORIA");

			log_info(log_kernel, "El puerto de la memoria es: %d", arc_config->puerto_memoria);

		}else {
			log_error(log_kernel,
					"El archivo de configuracion no contiene el PUERTO de la Memoria");

		}

		if(config_has_property(configFile,"IP_MEMORIA")){

			log_info(log_kernel, "Almacenando la IP de la Memoria");

			arc_config->ip_memoria = config_get_string_value(configFile,"IP_MEMORIA");

			log_info(log_kernel, "La Ip de la memoria es: %s", arc_config->ip_memoria);


		}else{

			log_error(log_kernel,
					"El archivo de configuracion no contiene la IP de la Memoria");

		}

		if(config_has_property(configFile,"QUANTUM")){

			log_info(log_kernel, "Almancenando el Quantum del planificador");

			arc_config->quantum = config_get_int_value(configFile,"QUANTUM");

			log_info(log_kernel, "El Quantum del planificador es: %d", arc_config->quantum);


		}else{

			log_error(log_kernel,
					"El archivo de configuracion no contiene el Quantum del planificador");

		}

		if(config_has_property(configFile,"MULTIPROCESAMIENTO")){

			log_info(log_kernel,"Almacenando el valor del Multiprocesamiento para el Planificador");

			arc_config->multiprocesamiento = config_get_int_value(configFile,"MULTIPROCESAMIENTO");

			log_info(log_kernel, "El grado de multiprocesamiento del planificador es: %d", arc_config->multiprocesamiento);

		}else{

			log_error(log_kernel,
					"El archivo de configuracion no el grado de multiprocesamiento del planificador");

		}

		if(config_has_property(configFile,"METADATA_REFRESH")){

			log_info(log_kernel,"Almacenando el valor del Metadata Refresh para el Kernel");

			arc_config->metadata_refresh = config_get_int_value(configFile,"METADATA_REFRESH");

			log_info(log_kernel, "El valor del Metadata Refresh es: %d", arc_config->metadata_refresh);
		}else{

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Metadata refresh");
		
		}

		if(config_has_property(configFile,"SLEEP_EJECUCION")){

			log_info(log_kernel,"Almacenando el valor del Sleep Ejecucion para el Kernel");

			arc_config->sleep_ejecucion = config_get_int_value(configFile,"SLEEP_EJECUCION");

			log_info(log_kernel, "El valor del Sleep Ejecucion es: %d", arc_config->sleep_ejecucion);
		}else{

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Sleep Ejecucion");

		}

		
	}else{
		
		log_error(log_kernel,"No se encontro el archivo de configuracion para cargar la estructura de Kernel");

	}

	log_info(log_kernel,"Cargamos todo lo que se encontro en el archivo de configuracion. Liberamos la variable config que fue utlizada para navegar el archivo de configuracion");

	free(configFile);

}

