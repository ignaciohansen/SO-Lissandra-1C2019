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
	imprimirMensaje(log_memoria, " \n ========== Iniciación de Pool de Memoria ========== \n \n ");

/*
 *
 */
	// CONFIG
	cargarConfiguracion();

	// Lectura desde consola de Query-LQL de "Pool de Memorias"
	/*
	char* comando = lectura_consola();
	log_info(log_memoria,"Se lee de consola la línea: "); log_info(log_memoria,comando);
*/
	/*
	comando = stringRemoverVaciosIzquierda(comando); // depurado

	switch (true){
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
		case stringContiene(comando,"SELECT"):
	}
	*/
	/*
	if (stringContiene(comando,"SELECT")) {
		log_info(log_memoria,"Se encontró comando SELECT");
	} else {
		log_info(log_memoria,"No se encontró ningún comando");
	}
	*/

	//terminar_memoria(log_memoria);
//	log_destroy(log_memoria);
//	free(log_memoria);
//	log_memoria = NULL;

/*
 *
 */
	log_info(log_memoria,
			"Ya creado el Log, coninuamos cargando la estructura de configuracion.");
	
	cargarConfiguracion();

	log_info(log_memoria,
			"Se cargaron los parametros de memoria.");


	/*

	socketEscuchaKernel = nuevoSocket(log_memoria);

	if(socketEscuchaKernel == ERROR){

		log_error(log_memoria,"Hubo un problema al querer crear el socket de escucha para memoria. Salimos del Proceso");

		return 0;
	}

	log_info(log_memoria,
			"El socket de escucha se creo con exito, con valor %d: .", socketEscuchaKernel);

	log_info(log_memoria,
			"Por asociar el socket con el puerto de escucha.");

	log_info(log_memoria, "El puerto que vamos a asociar es %d:", arc_config->puerto);
	
	asociarSocket(socketEscuchaKernel,arc_config->puerto,log_memoria);

	log_info(log_memoria,
			"Ya asociado al puerto lo ponemos a la escucha.");

	socketEscuchar(socketEscuchaKernel,10,log_memoria);

	while(1){

		log_info(log_memoria,
			"En el while esperando conexiones.");

		conexionEntrante = aceptarConexionSocket(socketEscuchaKernel,log_memoria);

		if(conexionEntrante == ERROR){

			log_error(log_memoria,"Se produjo un error al aceptar la conexion, salimos");
			return -1;
		}

		buffer = malloc(10*sizeof(char));

		recibiendoMensaje = socketRecibir(conexionEntrante,buffer,10);

		printf("Recibimos por socket %s",buffer);

		log_info(log_memoria,"El mensaje que se recibio fue %s", buffer);

	}


*/
	int a = conexionKernel();

	log_info(log_memoria,
				"Fin del proceso.");
/*
 *
 */

	return 0;

}

int conexionKernel(){

	imprimirAviso(log_memoria, "INICIAMOS LA CONEXION CON LFS");
	sockeConexionLF = nuevoSocket(log_memoria);


	if(sockeConexionLF == ERROR){

		imprimirError(log_memoria, "ERROR al crear un socket");
		log_error(log_memoria,"Hubo un problema al querer crear el socket desde Kernel. Salimos del Proceso");

		return ERROR;
	}

	imprimirVerde(log_memoria, "Se ha creado un socket correctamente");

	log_info(log_memoria,
				"El Socket creado es: %d .",sockeConexionLF);

	log_info(log_memoria,
				"por llamar a la funcion connectarSocket() para conectarnos con Memoria");

	log_info(log_memoria,"PRUEBA: %d ",8001);

	int resultado_Conectar = conectarSocket(sockeConexionLF, "0", 8001,log_memoria);

	if(resultado_Conectar == ERROR){

		imprimirError(log_memoria,"Hubo un problema al querer Conectarnos con LFS. Salimos del proceso");

		return -1;
	}else{

		imprimirVerde1(log_memoria,
				"Nos conectamos con exito, el resultado fue %d",resultado_Conectar);


	char * mensaje = "hola";
	resultado_sendMsj = socketEnviar(sockeConexionLF,mensaje,strlen(mensaje),log_memoria);

	imprimirMensaje(log_memoria,
					"Se ha intentado mandar un mensaje al server");

	if(resultado_sendMsj == ERROR){

		imprimirError(log_memoria,"Error al enviar mensaje a LSF. Salimos");
			return ERROR;
		}

	imprimirVerde1(log_memoria,"El mensaje se envio correctamente\n\NMENSAJE ENVIADO: %s", mensaje);

	return 10;

	/*char* msj = malloc(10*sizeof(char));
	msj = "PruebaK\n";

	resultado_sendMsj = socketEnviar(socket_CMemoria,msj,strlen(msj),log_kernel);

	if(resultado_sendMsj == ERROR){

		log_error(log_kernel,"Error al enviar mensaje a memoria. Salimos");
		return;
	}

	log_info(log_kernel,"El mensaje se envio correctamente");*/
	}
}

int enviarComando(char** comando,t_log* logger){

	log_info(logger,"En funcion enviarComando");

	char* msj = malloc(7*sizeof(char));

	msj = comando;

	log_info(logger,"El mensaje que vamos a enviar es: %s", msj);

	sockeConexionLF = conexionKernel();

	log_info("Vamos a enviar a memoria por el socket %d", sockeConexionLF);

	resultado_sendMsj = socketEnviar(sockeConexionLF,msj,strlen(msj),log_memoria);

	if(resultado_sendMsj == ERROR){

		log_error(log_memoria,"Error al enviar mensaje a memoria. Salimos");

		return ERROR;
	}

	log_info(log_memoria,"El mensaje se envio correctamente: %s",msj);

	return 0;

}

char* lectura_consola() {
	char* linea = (char*)readline(">");
	return linea;
}

void cargarConfiguracion() {

	log_info(log_memoria,
			"Por reservar memoria para variable de configuracion.");

	arc_config = malloc(sizeof(t_memoria_config));

	t_config* configFile;

	log_info(log_memoria,
			"Por crear el archivo de config para levantar archivo con datos.");

	configFile = config_create(PATH_MEMORIA_CONFIG);

	if (configFile != NULL) {

		log_info(log_memoria, "Memoria: Leyendo Archivo de Configuracion...");

		if(config_has_property(configFile,"PUERTO")){
			
			log_info(log_memoria, "Almacenando el puerto");

			arc_config->puerto = config_get_int_value(configFile,
				"PUERTO");

			log_info(log_memoria, "El puerto de la memoria es: %d", arc_config->puerto);

		}else {
			log_error(log_memoria,
					"El archivo de configuracion no contiene el PUERTO de la Memoria");

		}

		if(config_has_property(configFile,"IP_FS")){

			log_info(log_memoria, "Almacenando la IP del FS");

			arc_config->ip_fs = config_get_string_value(configFile,"IP_FS");

			log_info(log_memoria, "La Ip del FS es: %s", arc_config->ip_fs);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no contiene la IP del FS");

		}

		if(config_has_property(configFile,"PUERTO_FS")){

			log_info(log_memoria, "Almancenando el Puerto del FS");

			arc_config->puerto_fs = config_get_int_value(configFile,"PUERTO_FS");

			log_info(log_memoria, "El Puerto del FS es: %d", arc_config->puerto_fs);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no contiene el Puerto del FS");
		}

		if(config_has_property(configFile,"IP_SEEDS")){

			log_info(log_memoria,"Almacenando los valores de los seeds");

			arc_config->ip_seeds = config_get_array_value(configFile,"IP_SEEDS");

			log_info(log_memoria, "Los valores de las IP de seeds son: %s", arc_config->ip_fs);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no contiene las Ips de los seeds");

		}

		if(config_has_property(configFile,"PUERTO_SEEDS")){

			log_info(log_memoria,"Almacenando el valor de los puertos de SEEDS");

		//	arc_config->puerto_seeds = config_get_array_value(configFile,"PUERTO_SEEDS");

			log_info(log_memoria, "El valor de los puertos son: %d", arc_config->puerto_seeds);
		
		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor de los puertos de los seeds");
		
		}

		if(config_has_property(configFile,"RETARDO_MEM")){

			log_info(log_memoria,"Almacenando el valor del Retardo de la memoria");

			arc_config->retardo_mem = config_get_int_value(configFile,"RETARDO_MEM");

			log_info(log_memoria, "El valor del Retardo de Memoria es: %d", arc_config->retardo_mem);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del Retardo de Memoria");
		}

		if(config_has_property(configFile,"RETARDO_FS")){

			log_info(log_memoria,"Almacenando el valor del Retardo del FS");

			arc_config->retardo_fs = config_get_int_value(configFile,"RETARDO_FS");

			log_info(log_memoria, "El valor del Retardo del FS es: %d", arc_config->retardo_fs);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del Retardo del FS");
		}

		if(config_has_property(configFile,"TAM_MEM")){

			log_info(log_memoria,"Almacenando el valor del Tamaño de la memoria");

			arc_config->tam_mem = config_get_int_value(configFile,"TAM_MEM");

			log_info(log_memoria, "El valor del tamaño de la Memoria es: %d", arc_config->tam_mem);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del tamaño de la Memoria");
		}

		if(config_has_property(configFile,"RETARDO_JOURNAL")){

			log_info(log_memoria,"Almacenando el valor del Retardo del JOURNAL");

			arc_config->retardo_journal = config_get_int_value(configFile,"RETARDO_MEM");

			log_info(log_memoria, "El valor del Retardo del JOURNAL es: %d", arc_config->retardo_journal);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del Retardo del JOURNAL");
		}
		
		if(config_has_property(configFile,"RETARDO_GOSSIPING")){

			log_info(log_memoria,"Almacenando el valor del Retardo del Gossiping");

			arc_config->retardo_gossiping = config_get_int_value(configFile,"RETARDO_GOSSIPING");

			log_info(log_memoria, "El valor del Retardo del GOSSIPING: %d", arc_config->retardo_gossiping);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del Retardo del GOSSIPING");
		}

		if(config_has_property(configFile,"MEMORY_NUMBER")){

			log_info(log_memoria,"Almacenando el valor del Memory number");

			arc_config->memory_number = config_get_int_value(configFile,"MEMORY_NUMBER");

			log_info(log_memoria, "El valor del Memory number es: %d", arc_config->retardo_mem);

		}else{

			log_error(log_memoria,
					"El archivo de configuracion no tiene el valor del Memory Number");
		}

	}else{
		
		log_error(log_memoria,"No se encontro el archivo de configuracion para cargar la estructura de Kernel");

	}

}

void terminar_memoria(t_log* g_log) {
	log_destroy(g_log);
	free(g_log);
	g_log = NULL;


}

