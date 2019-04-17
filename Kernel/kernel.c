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

	log_info(log_kernel, "Creamos thread para Consola");

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);

	//pthread_t hiloConexion;
	//pthread_create(&hiloConexion,NULL,(void*) conexionKernel,NULL);

	//pthread_join(hiloConexion,NULL);
	pthread_join(hiloConsola, NULL);

	log_info(log_kernel, "Salimoooos");

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

		if (config_has_property(configFile, "PUERTO_MEMORIA")) {

			log_info(log_kernel, "Almacenando el puerto");
			
			arc_config->puerto_memoria = config_get_int_value(configFile,
					"PUERTO_MEMORIA");

			log_info(log_kernel, "El puerto de la memoria es: %d",
					arc_config->puerto_memoria);

		} else {
			log_error(log_kernel,
					"El archivo de configuracion no contiene el PUERTO de la Memoria");

		}

		if (config_has_property(configFile, "IP_MEMORIA")) {

			log_info(log_kernel, "Almacenando la IP de la Memoria");

			arc_config->ip_memoria = config_get_string_value(configFile,
					"IP_MEMORIA");

			log_info(log_kernel, "La Ip de la memoria es: %s",
					arc_config->ip_memoria);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no contiene la IP de la Memoria");

		}

		if (config_has_property(configFile, "QUANTUM")) {

			log_info(log_kernel, "Almancenando el Quantum del planificador");

			arc_config->quantum = config_get_int_value(configFile, "QUANTUM");

			log_info(log_kernel, "El Quantum del planificador es: %d",
					arc_config->quantum);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no contiene el Quantum del planificador");

		}

		if (config_has_property(configFile, "MULTIPROCESAMIENTO")) {

			log_info(log_kernel,
					"Almacenando el valor del Multiprocesamiento para el Planificador");

			arc_config->multiprocesamiento = config_get_int_value(configFile,
					"MULTIPROCESAMIENTO");

			log_info(log_kernel,
					"El grado de multiprocesamiento del planificador es: %d",
					arc_config->multiprocesamiento);

		} else {

			log_error(log_kernel,
					"El archivo de configuracion no el grado de multiprocesamiento del planificador");

		}

		if (config_has_property(configFile, "METADATA_REFRESH")) {

			log_info(log_kernel,
					"Almacenando el valor del Metadata Refresh para el Kernel");

			arc_config->metadata_refresh = config_get_int_value(configFile,
					"METADATA_REFRESH");

			log_info(log_kernel, "El valor del Metadata Refresh es: %d",
					arc_config->metadata_refresh);
		} else {

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Metadata refresh");

		}

		if (config_has_property(configFile, "SLEEP_EJECUCION")) {

			log_info(log_kernel,
					"Almacenando el valor del Sleep Ejecucion para el Kernel");

			arc_config->sleep_ejecucion = config_get_int_value(configFile,
					"SLEEP_EJECUCION");

			log_info(log_kernel, "El valor del Sleep Ejecucion es: %d",
					arc_config->sleep_ejecucion);
		} else {

			log_error(log_kernel,
					"El archivo de configuracion no tiene el valor del Sleep Ejecucion");

		}

	} else {

		log_error(log_kernel,
				"No se encontro el archivo de configuracion para cargar la estructura de Kernel");

	}

	log_info(log_kernel,
			"Cargamos todo lo que se encontro en el archivo de configuracion. Liberamos la variable config que fue utlizada para navegar el archivo de configuracion");

	free(configFile);

}

void consola() {

	log_info(log_kernel, "En el hilo de consola");

	menu();

	char bufferComando[MAXSIZE_COMANDO];
	char **comandoSeparado;
	char **comandoSeparado2;
	char *separador2 = "\n";
	char *separator = " ";
	int comando;

	while (1) {

		printf(">");

		fgets(bufferComando, MAXSIZE_COMANDO, stdin);

		strtok(bufferComando, "\n");
		
		add_history(linea);

		free(linea);
	
		comandoSeparado = string_split(bufferComando, separator);

		//Tamanio del array
		
		for (int i = 0; comandoSeparado[i] != NULL; i++) {
			
			log_info(log_kernel,"En la posicion %d del array esta el valor %s",i,comandoSeparado[i]);
			
			tamanio = i + 1;
		}

		log_info(log_kernel, "El tamanio del vector de comandos es: %d",
				tamanio);

		switch (tamanio){

			case 1:
				{	
					comandoSeparado = string_split(bufferComando, separador2);
					
					log_info(log_kernel,"%s",comandoSeparado[0]);

					log_info(log_kernel,"%d",strcmp(comandoSeparado[0],"salir"));
					
					if(strcmp(comandoSeparado[0],"salir") == 0){
		 				
						printf("Salir seleccionado\n");
						
						log_info(log_kernel, "Se selecciono Salir");
				
						return;

		 			} else if(strcmp(comandoSeparado[0],"journal") == 0){

						 printf("Journal seleccionado\n");
						 
						 log_info(log_kernel, "Se selecciono el comando Journal");

							//Enviar Journal a todas las memorias

						int resultadoEnviarComando = enviarComando(comandoSeparado,log_kernel);


					 } else if(strcmp(comandoSeparado[0],"metrics") == 0){

						 printf("Metrics seleccionado\n");

						 log_info(log_kernel, "Se selecciono el comando Metrics");

							//Se muestra Metricas
					 }
					 else{
		 				printf("Comando mal ingresado. \n");
		 				
						 log_error(log_kernel,
		 									"Opcion mal ingresada por teclado en la consola");
		 				break;
		 			}break;
				}
			case 2:
				validarComando(comandoSeparado,tamanio,log_kernel);
				
				break;

			case 3:
				validarComando(comandoSeparado,tamanio,log_kernel);
				
				break;

			default:
				{
				printf("Comando mal ingresado. \n");
				
				log_error(log_kernel,
					"Opcion mal ingresada por teclado en la consola");
			}
				
				break;

		}
		//comando = validacionComando(comandoSeparado[0],log_kernel);

		//log_info(log_kernel, "El numero de comando ingresado es %d", comando);	
		


		//INTERPRETACION DE COMANDOS ANTERIORES
		//}

		/*switch(comandoIngresado){

		 case 1:
		 log_info(log_kernel, "select");
		 break;
		 case 2:
		 log_info(log_kernel, "insert");
		 break;
		 default:
		 printf("Comando mal ingresado. \n");
		 log_error(log_kernel,"Opcion mal ingresada por teclado en la consola");
		 break;
		 }

		 if(strcmp(comandoSeparado[0],"select") == 0){

		 printf("Se selecciono Select\n");

		 log_info(log_kernel,"Por llamar a enviarResultado");

		 int resultadoEnviarComando = enviarComando(comandoSeparado[0],log_kernel);
		 //break;
		 }
		 	if(strcmp(comandoSeparado[0],"insert") == 0){

		 printf("Insert seleccionado\n");
		 break;
		 }

		 if(strcmp(comandoSeparado[0],"create") == 0){
		 printf("Create seleccionado\n");
		 break;
		 }

		 if(strcmp(comandoSeparado[0],"describe") == 0){
		 printf("Describe seleccionado\n");
		 break;
		 }
		 if(strcmp(comandoSeparado[0],"drop") == 0){
		 printf("Drop seleccionado\n");
		 break;
		 }
		 if(strcmp(comandoSeparado[0],"journal") == 0){
		 printf("Journal seleccionado\n");
		 break;
		 }
		 if(strcmp(comandoSeparado[0],"add") == 0){
		 printf("add seleccionado\n");
		 break;
		 }

		 if(strcmp(comandoSeparado[0],"run") == 0){
		 printf("Run seleccionado\n");
		 break;
		 }

		 if(strcmp(comandoSeparado[0],"salir") == 0){
		 break;
		 }
		 printf("Comando mal ingresado. \n");
		 log_error(log_kernel,"Opcion mal ingresada por teclado en la consola");
		 */

	}
}

int conexionKernel() {

	socket_CMemoria = nuevoSocket(log_kernel);

	if (socket_CMemoria == ERROR) {

		log_error(log_kernel,
				"Hubo un problema al querer crear el socket desde Kernel. Salimos del Proceso");

		return ERROR;
	}

	log_info(log_kernel, "El Socket creado es: %d .", socket_CMemoria);

	log_info(log_kernel,
			"por llamar a la funcion connectarSocket() para conectarnos con Memoria");

	log_info(log_kernel, "PRUEBA: %d ", arc_config->puerto_memoria);

	resultado_Conectar = conectarSocket(socket_CMemoria, arc_config->ip_memoria,
			arc_config->puerto_memoria, log_kernel);

	if (resultado_Conectar == ERROR) {

		log_error(log_kernel,
				"Hubo un problema al querer Conectarnos con Memoria. Salimos del proceso");

		return -1;
	} else {

		log_info(log_kernel, "Nos conectamos con exito, el resultado fue %d",
				resultado_Conectar);
		return socket_CMemoria;		
	}
}

int enviarComando(char** comando, t_log* logger) {

	log_info(logger, "En funcion enviarComando: %s");
	
	log_info(logger, "El mensaje que vamos a enviar es: %s", comando[0]);

	socket_CMemoria = conexionKernel();

	log_info("Vamos a enviar a memoria por el socket %d", socket_CMemoria);

	resultado_sendMsj = socketEnviar(socket_CMemoria, comando[0], strlen(comando[0]),
			log_kernel);

	if (resultado_sendMsj == ERROR) {

		log_error(log_kernel, "Error al enviar mensaje a memoria. Salimos");

		return ERROR;
	}

	log_info(log_kernel, "El mensaje se envio correctamente: %s", comando[0]);

	return 0;

}

void menu() {

	printf("Los comandos que se pueden ingresar son: \n"
			"COMANDOS \n"
			"Insert \n"
			"Select \n"
			"Create \n"
			"Describe \n"
			"Drop \n"
			"Journal  \n"
			"add \n"
			"run \n"
			"SALIR \n"
			"\n");

}

int buscarComando(char* comando,t_log* logger) {

	log_info(logger,"Recibimos el comando: %s",comando);

	int i = 0;
	

	//while (i < salir && strcmp(comandosPermitidos[i], comando)) {
		
		//i++;
	
	//}

	for (i;i <= salir && strcmp(comandosPermitidos[i], comando);i++) {
		
			
	}	

	log_info(logger,"Se devuelve el valor %d",i);

	return i;

}

void validarComando(char** comando,int tamanio,t_log* logger){

		int resultadoComando = buscarComando(comando[0],logger);

		switch (resultadoComando) {

			case Select: {
				printf("Se selecciono Select\n");

				log_info(log_kernel, "Se selecciono select");

				if(tamanio == 3){
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando select");
					
					log_info(log_kernel, "Por llamar a enviarComando");

					mensaje = malloc(string_length(comando[1])+1);

					strcpy(mensaje,comando[1]);	

					log_info(log_kernel,"Por concatenar la cadena: %s ,con el separador",mensaje);

					string_append(&mensaje,&SEPARADOR);

					log_info(log_kernel,"Ahora la cadena es %s",mensaje);
					
					string_append(&mensaje,comando[2]);

					log_info(log_kernel,"Ahora la cadena es %s, con el ultimo argumento",mensaje);

					log_info(log_kernel,"Queriamos mandar esto: %s", mensaje);
					log_info(log_kernel,"Y se mando esto: %s",mensaje);

					int resultadoEnviarComando = enviarMensaje(Select,tamanio,mensaje,log_kernel);
				}

			}
				break;

			case insert: {
				printf("Insert seleccionado\n");

				log_info(log_kernel, "Se selecciono insert");

				if(tamanio == 4){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando insert");
					
					log_info(log_kernel, "Por llamar a enviarResultado");

					int resultadoEnviarComando = enviarComando(comando,log_kernel);
				}

			}
				break;

			case create: {
				printf("Create seleccionado\n");
				log_info(log_kernel, "Se selecciono Create");

				if(tamanio == 4){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando create");
					
					log_info(log_kernel, "Por llamar a enviarResultado");

					int resultadoEnviarComando = enviarComando(comando,log_kernel);
				}


			}
				break;

			case describe: {
				printf("Describe seleccionado\n");
				log_info(log_kernel, "Se selecciono Describe");

				if(tamanio == 1){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando Describe");
					
					log_info(log_kernel, "Por llamar a enviarResultado");

					int resultadoEnviarComando = enviarComando(comando,log_kernel);
				}


			}
				break;

			case drop: {
				printf("Drop seleccionado\n");
				log_info(log_kernel, "Se selecciono Drop");

				if(tamanio == 2){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando Drop");
					
					log_info(log_kernel, "Por llamar a enviarComando");
					
					mensaje = malloc(string_length(comando[1])+1);
					
					strcpy(mensaje,comando[1]);

					log_info(log_kernel,"Queriamos mandar esto: %s", comando[1]);
					log_info(log_kernel,"Y se mando esto: %s",mensaje);

					int resultadoEnviarComando = enviarMensaje(drop,tamanio,mensaje,log_kernel);
				}



			}
				break;
			
			case add: {
				printf("Add seleccionado\n");
				log_info(log_kernel, "Se selecciono Add");

				if(tamanio == 5){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando add");
					
					log_info(log_kernel, "Por llamar a enviarResultado");

					int resultadoEnviarComando = enviarComando(comando,log_kernel);
				}

			}
				break;
			
			case run: {
				printf("Run seleccionado\n");
				log_info(log_kernel, "Se selecciono Run");

				if(tamanio == 2){
					
					log_info(log_kernel, "Cantidad de parametros correctos ingresados para el comando run");
					
					log_info(log_kernel, "Por llamar a enviarResultado");

					int resultadoEnviarComando = enviarComando(comando,log_kernel);
				}

			}
				break;			

			default: {
				printf("Comando mal ingresado. \n");
				log_error(log_kernel,
					"Opcion mal ingresada por teclado en la consola");
			}
				break;

		}
}

int enviarMensaje(int comando,int tamanio, char* mensaje, t_log* logger){

	log_info(logger,"En funcion enviarMensaje.");

	t_header* headerParaEnviar = malloc(sizeof(t_header));

	log_info(logger,"Por guardar en la estructura del Header, el comando: %d", comando);
	headerParaEnviar->comando = comando;

	log_info(logger,"Por guardar en la estructura del Header, la cantidad de argumentos: %d", tamanio);
	headerParaEnviar->cantArgumentos = tamanio - 1;//Le restamos uno ya esta suma tiene en cuenta el comando
	
	log_info(logger,"Conectamos por socket con la memoria.");
	socket_CMemoria = conexionKernel();	

	int tamanioMensaje = string_length(mensaje);
	log_info(logger,"El tamanio del mensaje que se va a mandar es de: %d", tamanioMensaje);
	headerParaEnviar->tamanio = tamanioMensaje;
	
	log_info(logger,"tamanio del header a enviar: %d",sizeof(t_header));
	
	resultado_sendMsj = socketEnviar(socket_CMemoria, headerParaEnviar, sizeof(t_header),
			log_kernel);

	if (resultado_sendMsj == ERROR) {

		log_error(log_kernel, "Error al enviar mensaje a memoria. Salimos");

		return ERROR;
	}

	log_info(log_kernel, "El mensaje se envio correctamente");

	log_info(log_kernel,"Preparados para recibir %d",sizeof(confirmacionRecibida));
	int recibiendoMensaje = socketRecibir(socket_CMemoria,&confirmacionRecibida,sizeof(confirmacionRecibida),log_kernel);

	log_info(logger,"Lo que llego fue: %d", confirmacionRecibida);

	if(confirmacionRecibida == sizeof(t_header)){

		log_info(logger,"La confirmacion que llego fue correcta, se procedera a enviar el body");
		printf("La confirmacion que llego fue correcta, se procedera a enviar el body. \n");

		resultado_sendMsj = socketEnviar(socket_CMemoria,mensaje,strlen(mensaje),log_kernel);

		if(resultado_sendMsj == strlen(mensaje)){

			log_info(logger,"Se envio el body correctamente");
			printf("Se envio el body Correctamente. \n");
		}else{

			log_error(logger,"El body se envio Incorrectamente");
			printf("El body se envio InCorrectamente. \n");
		}
	}else{
		log_error(logger,"La confirmacion que llego no es correcta, no se enviar el body");
		printf("La confirmacion que llego no es correcta, no se envia el body. \n");

	}


	return 0;

}
