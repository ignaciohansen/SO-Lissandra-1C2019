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
	LisandraSetUP();


}

/********************************************************************************************
 * 							SET UP LISANDRA, FILE SYSTEM Y COMPRIMIDOR
 ********************************************************************************************
 */

void LisandraSetUP() {
	imprimirMensajeProceso("Iniciando el modulo LISSANDRA FILE SYSTEM\n");
	log_lfilesystem = archivoLogCrear(LOG_PATH, "Proceso Lissandra File System");

	imprimirVerde(log_lfilesystem,
					"LOG CREADO, continuamos cargando la estructura de configuracion.");

	if(cargarConfiguracion()) {
		//SI SE CARGO BIEN LA CONFIGURACION ENTONCES PROCESO DE ABRIR UN SERVIDOR
		imprimirMensajeProceso("Levantando el servidor del proceso Lisandra");
		abrirServidorLissandra();
	}


}

int abrirServidorLissandra() {

	imprimirMensaje(log_lfilesystem, "Creamos un nuevo socket");

	socketEscuchaMemoria = nuevoSocket(log_lfilesystem);

		if(socketEscuchaMemoria == ERROR){

			imprimirError(log_lfilesystem, "Hubo un problema al querer crear el socket de escucha para memoria. Salimos del Proceso");

			return 0;
		}
		configFile = config_create("../LISANDRAFS.txt");
		int puerto_a_escuchar = config_get_int_value(configFile,
									"PUERTO_ESCUCHA");

		imprimirVerde1(log_lfilesystem, "Se ha creado el socket con exito con valor %d: .", socketEscuchaMemoria);

		imprimirMensaje1(log_lfilesystem,
				"El socket de escucha se creo con exito, con valor %d: .", socketEscuchaMemoria);
		imprimirMensaje(log_lfilesystem,
				"Por asociar el socket con el puerto de escucha.");

		imprimirMensaje1(log_lfilesystem,
				"El puerto que vamos a asociar es %i:", puerto_a_escuchar);




		asociarSocket(socketEscuchaMemoria,puerto_a_escuchar,log_lfilesystem);

		imprimirMensaje(log_lfilesystem,
						"Ya asociado al puerto lo ponemos a la escucha.");

		socketEscuchar(socketEscuchaMemoria,10,log_lfilesystem);
		int i =1;
		while(1)
		{
			imprimirMensaje(log_lfilesystem, "En el while esperando conexiones.");

			int estado = listen(socketEscuchaMemoria, 10);

				if(estado == ERROR){

					imprimirError(log_lfilesystem, "Error al poner el Socket en escucha");

			return 0;
				}
				imprimirVerde(log_lfilesystem, "EL socket ya esta en escucha");

			conexionEntrante = aceptarConexionSocket(socketEscuchaMemoria,log_lfilesystem);

			if(conexionEntrante == ERROR){

				imprimirError(log_lfilesystem,"Se produjo un error al aceptar la conexion, salimos");
				return -1;
			} else {
				imprimirVerde(log_lfilesystem, "Se ha conectado un cliente");
			}

			buffer = malloc(10*sizeof(char));

			recibiendoMensaje = socketRecibir(conexionEntrante,buffer,10);

			printf("Recibimos por socket %s",buffer);



			imprimirMensaje(log_lfilesystem, "Mensaje recibido:");
			imprimirVerde(log_lfilesystem, buffer);
			i++;
		}
		imprimirMensajeProceso("FIN PROCESO SOCKET");
		log_info(log_lfilesystem,
					"Fin del proceso.");

		return 1;

}

bool cargarConfiguracion() {

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
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el PUERTO_ESCUCHA");
			ok--;

		}

		if(config_has_property(configFile,"PUNTO_MONTAJE")){

			log_info(log_lfilesystem, "Almacenando el PUNTO DE MONTAJE");

			//Por lo que dice el texto
			arc_config->punto_montaje = config_get_int_value(configFile,
				"PUNTO_MONTAJE");

			log_info(log_lfilesystem, "El puerto de montaje es: %d", arc_config->punto_montaje);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el PUNTO_MONTAJE");
			ok--;

		}

		if(config_has_property(configFile,"RETARDO")){

			log_info(log_lfilesystem, "Almacenando el retardo");

			//Por lo que dice el texto
			arc_config->retardo = config_get_int_value(configFile,
				"RETARDO");

			log_info(log_lfilesystem, "El retardo de respuesta es: %d", arc_config->retardo);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene RETARDO");
			ok--;

		}

		if(config_has_property(configFile,"TAMANIO_VALUE")){

			log_info(log_lfilesystem, "Almacenando el tamaño del valor de una key");

			//Por lo que dice el texto
			arc_config->tamanio_value = config_get_int_value(configFile,
				"TAMANIO_VALUE");

			log_info(log_lfilesystem, "El tamaño del valor es: %d", arc_config->tamanio_value);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el TAMANIO_VALUE");
			ok--;

		}

		if(config_has_property(configFile,"TIEMPO_DUMP")){

			log_info(log_lfilesystem, "Almacenando el puerto");

			//Por lo que dice el texto
			arc_config->tiempo_dump = config_get_int_value(configFile,
				"TIEMPO_DUMP");

			log_info(log_lfilesystem, "El tiempo de dumpeo es: %d", arc_config->tiempo_dump);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el TIEMPO_DUMP");
			ok--;

		}


		if(ok>0) {
			imprimirVerde(log_lfilesystem,"Se ha cargado todos los datos del archivo de configuracion");
		//	log_info(log_lfilesystem, "Se ha cargado todos los datos del archivo de configuracion");
			return true;

		} else {
			imprimirError(log_lfilesystem, "ERROR: No Se han cargado todos o algunos los datos del archivo de configuracion\n");
	//		imprimirMensajeProceso("ERROR: No Se han cargado todos los datos del archivo de configuracion\n");
			return false;
		}

	}


}

void consola(){

	log_info(log_lfilesystem, "En el hilo de consola");

	menu();

	char bufferComando[MAXSIZE_COMANDO];
	char **comandoSeparado;
	char **comandoSeparado2;
	char *separador2="\n";
	char *separator=" ";

	while(1){

		printf(">");

		fgets(bufferComando,MAXSIZE_COMANDO, stdin);

		add_history(linea);

		free(linea);

		comandoSeparado=string_split(bufferComando, separator);

		//Tamanio del array
		for(int i = 0; comandoSeparado[i] != NULL; i++){

			tamanio = i +1;
		}

		log_info(log_lfilesystem, "El tamanio del vector de comandos es: %d", tamanio);

		if(strcmp(comandoSeparado[0],"select") == 0){

			printf("Se selecciono Select\n");

			log_info(log_lfilesystem,"Por llamar a enviarResultado");

			// FALTA ADAPTAR ESTA FUNCION //

			//int resultadoEnviarComando = enviarComando(comandoSeparado[0],log_lfilesystem);
			//break;

			// FALTA ADAPTAR ESTA FUNCION //
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

		if(strcmp(comandoSeparado[0],"salir") == 0){
			break;
		}
		printf("Comando mal ingresado. \n");
		log_error(log_lfilesystem,"Opcion mal ingresada por teclado en la consola");


	}
}

void menu(){

	printf("Los comandos que se pueden ingresar son: \n"
		"COMANDOS \n"
			"Insert \n"
			"Select \n"
			"Create \n"
			"Describe \n"
			"Drop \n"
			"SALIR \n"
"\n");

}

