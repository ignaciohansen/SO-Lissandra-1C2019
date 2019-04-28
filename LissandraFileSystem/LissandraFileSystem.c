/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
 */

#include "LissandraFileSystem.h"
#include "commons/string.h"

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

	imprimirVerde(log_lfilesystem, "[LOG CREADO] continuamos cargando la estructura de configuracion.");

	if(cargarConfiguracion()) {
		//SI SE CARGO BIEN LA CONFIGURACION ENTONCES PROCESO DE ABRIR UN SERVIDOR
		imprimirMensajeProceso("Levantando el servidor del proceso Lisandra");
		abrirServidorLissandra();
	}


}

int abrirServidorLissandra() {

	socketEscuchaMemoria = nuevoSocket(log_lfilesystem);

	if(socketEscuchaMemoria == ERROR){
		imprimirError(log_lfilesystem, "Hubo un problema al querer crear el socket de escucha para memoria. Salimos del Proceso");
		return -1;
	}


	log_info(log_lfilesystem, "\n ========== LFS Listening ========== \n");

	int puerto_a_escuchar = configFile->puerto_escucha;

	imprimirVerde1  (log_lfilesystem      ,"Se ha creado el socket con exito con valor %d: .", socketEscuchaMemoria);

	imprimirMensaje1(log_lfilesystem      ,"El puerto que vamos a asociar es %i:", puerto_a_escuchar);

	asociarSocket   (socketEscuchaMemoria ,puerto_a_escuchar,log_lfilesystem);

	imprimirMensaje (log_lfilesystem      , "Asociado. OK!");

	socketEscuchar  (socketEscuchaMemoria ,10 ,log_lfilesystem);

	int estado = listen(socketEscuchaMemoria, 10);

	if(estado == ERROR){
		imprimirError(log_lfilesystem, "Error al poner el Socket en escucha");
		return -1;
	}

	int i =1;
	while(1){ // recibe mensajes y responde

	imprimirMensaje(log_lfilesystem, " \n ====== LFS: waiting for connections ====== \n ");

	conexionEntrante = aceptarConexionSocket(socketEscuchaMemoria,log_lfilesystem);

	if(conexionEntrante == ERROR){
		imprimirError(log_lfilesystem,"Se produjo un error al aceptar la conexion, salimos");
		return -1;
	} else {
		char* msg = string_new();
		string_append(&msg   , "Connection number "     );
		string_append(&msg   , stringConvertirEntero(i) );
		string_append(&msg   , " established."          );
		imprimirVerde(log_lfilesystem, msg);
		free(msg);
	}

	log_info(log_lfilesystem, "[DEBUG] Antes de crear puntero."  );
	Puntero buffer = (void*)string_new(); // malloc(sizeof(char)*100);
	log_info(log_lfilesystem, "[DEBUG] Despues de crear puntero."  );

	log_info(log_lfilesystem, "[DEBUG] Antes de recibir mensaje."  );
	recibiendoMensaje = socketRecibir(conexionEntrante, buffer, 13,  log_lfilesystem);
	log_info(log_lfilesystem, "[DEBUG] Despues de recibir mensaje.");

	char* msg = string_new();
	string_append(&msg,"Mensaje recibido: \"");
	string_append(&msg,buffer                );
	string_append(&msg,"\"."                 );
	imprimirVerde(log_lfilesystem, msg);
	free(buffer);
	i++;

		/*
		 	log_info(log_lfilesystem, "Por liberar el buffer");
			free(buffer);
			log_info(log_lfilesystem, "buffer liberado");
			*/


			/**
			char* holaMemoria = "Hola memoria";


			log_info(log_lfilesystem, "POr enviar un mensaje a Memoria");
			int resultado_sendMsj = socketEnviar(conexionEntrante,holaMemoria,strlen(holaMemoria),log_lfilesystem);
			log_info(log_lfilesystem, "Mensaje enviado");


			if(resultado_sendMsj == 0) {
				imprimirError(log_lfilesystem,"Se produjo un error al aceptar la conexion, salimos");
								return -1;
			} else {
				imprimirVerde(log_lfilesystem, "Se ha devuelto el saludo a Memoria");
			}
*/
			char* mensajeValorValue = stringConvertirEntero(configFile->tamanio_value);
//			strcpy(mensajeMAXVALUE, stringConvertirEntero(configFile->tamanio_value));

		//	strcpy(mensajeMAXVALUE, stringConvertirEntero(10));
			int res = socketEnviar(conexionEntrante,mensajeValorValue, 3,log_lfilesystem);

			if(res == 0) {
				imprimirError(log_lfilesystem,"Se produjo un error al aceptar la conexion, salimos");
				return -1;
			} else {
				imprimirVerde(log_lfilesystem, "Se ha enviado el MAX VALUE a Memoria");
			}

		//	free(buffer);
		//	send(conexionEntrante, "Hola memoria", 13, 0);

		} // while(1)

		imprimirMensajeProceso("FIN PROCESO SOCKET");
		log_info(log_lfilesystem, "Fin del proceso.");

		return 1;

}

bool cargarConfiguracion() {

	log_info(log_lfilesystem,
			"Por reservar memoria para variable de configuracion.");

	configFile = malloc(sizeof(t_lfilesystem_config));

	t_config* archivoCOnfig;

	log_info(log_lfilesystem,
			"Por crear el archivo de config para levantar archivo con datos.");


	archivoCOnfig = config_create("LISANDRAFS.txt");

	if(archivoCOnfig == NULL)
	{
		imprimirMensajeProceso("NO se ha encontrado el archivo de configuracion\n");
		log_info(log_lfilesystem, "NO se ha encontrado el archivo de configuracion");
	}

	if (archivoCOnfig != NULL) {
		int ok = 1;
		imprimirMensajeProceso("Se ha encontrado el archivo de configuracion\n");

		log_info(log_lfilesystem, "LissandraFS: Leyendo Archivo de Configuracion...");

		if(config_has_property(archivoCOnfig, "IP_FS")){
			log_info(log_lfilesystem, "Almacenando IP de LIsandra File Sytem");

						//Por lo que dice el texto
			configFile->ip = config_get_int_value(archivoCOnfig,
							"IP_FS");

						log_info(log_lfilesystem, "La IP al cual se conectara Lisandra es: %d", configFile->ip);
		} else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el IP_FS");
			ok--;
		}

		if(config_has_property(archivoCOnfig,"PUERTO_ESCUCHA")){

			log_info(log_lfilesystem, "Almacenando el puerto");

			//Por lo que dice el texto
			configFile->puerto_escucha = config_get_int_value(archivoCOnfig,
				"PUERTO_ESCUCHA");

			log_info(log_lfilesystem, "El puerto de escucha es: %d", configFile->puerto_escucha);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el PUERTO_ESCUCHA");
			ok--;

		}

		if(config_has_property(archivoCOnfig,"PUNTO_MONTAJE")){

			log_info(log_lfilesystem, "Almacenando el PUNTO DE MONTAJE");

			//Por lo que dice el texto
			configFile->punto_montaje = config_get_int_value(archivoCOnfig,
				"PUNTO_MONTAJE");

			log_info(log_lfilesystem, "El puerto de montaje es: %d", configFile->punto_montaje);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el PUNTO_MONTAJE");
			ok--;

		}

		if(config_has_property(archivoCOnfig,"RETARDO")){

			log_info(log_lfilesystem, "Almacenando el retardo");

			//Por lo que dice el texto
			configFile->retardo = config_get_int_value(archivoCOnfig,
				"RETARDO");

			log_info(log_lfilesystem, "El retardo de respuesta es: %d", configFile->retardo);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene RETARDO");
			ok--;

		}

		if(config_has_property(archivoCOnfig,"TAMANIO_VALUE")){

			log_info(log_lfilesystem, "Almacenando el tamaño del valor de una key");

			//Por lo que dice el texto
			configFile->tamanio_value = config_get_int_value(archivoCOnfig,"TAMANIO_VALUE");

			log_info(log_lfilesystem, "El tamaño del valor es: %d", configFile->tamanio_value);

		}else {
			imprimirError(log_lfilesystem, "El archivo de configuracion no contiene el TAMANIO_VALUE");
			ok--;

		}

		if(config_has_property(archivoCOnfig,"TIEMPO_DUMP")){

			log_info(log_lfilesystem, "Almacenando el puerto");

			//Por lo que dice el texto
			configFile->tiempo_dump = config_get_int_value(archivoCOnfig,
				"TIEMPO_DUMP");

			log_info(log_lfilesystem, "El tiempo de dumpeo es: %d", configFile->tiempo_dump);

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

