/*
 * LISTA DE FUNCIONES
 *
 * int main()
 * void inicioLogYConfig()
 * void crearConexionesConOtrosProcesos()
 * void conectarConServidorLisandraFileSystem()
 * void levantarServidor()
 * void crearHIloEscucharKernel()
 * void escucharConexionKernel()
 * void ejecutarHiloConsola()
 * char* lectura_consola()
 * void consola()
 * void menu()
 * void validarComando(char* comando,t_log* logger)
 * int enviarComando(char** comando, t_log* logger)
 * int buscarComando(char* comando,t_log* logger)
 * void cargarConfiguracion()
 *
 * Comentarios de acceso rápido.
 *
 * ## Acá IP de LFS Hardcodeada (#001#)
 *
 */

#include "memoria.h"

void terminar_memoria(t_log* g_log);

int main() {
    /*
     * 1) Conectar a LFS, hacer handshake: obtener "tamaño máximo de value" p. admin de paginas
     * 2) Iniciar la memoria principal
     * 3) Ejecutar Gossiping
     * Más requerimientos a seguir:
     *  * correr concurrentemente : - consola
     *                              - red.
     *
     *  * debe soportar:            - hilos
     *                              - memoria compartida.
     *
     *  * idea: PROCESO PADRE : memoria compartida.
     *          PROCESOS HIJOS: leen consola y reciben mensajes.
     *                          los pasan al padre por memoria compartida.
     *
     *
     */

    // LOGGING
	inicioLogYConfig();



    // SECCIÓN DE CONSOLA.
    // Operaciones: SELECT, INSERT, CREATE, DROP, DESCRIBE.

	/*
    String    comando;
    comando = lectura_consola();
    log_info(log_memoria,"Se lee de consola la línea: ");
    log_info(log_memoria,comando);

    stringRemoverVaciosIzquierda(&comando);
    */

	crearConexionesConOtrosProcesos();

    ejecutarHiloConsola();

    armarMemoriaPrincipal();


    // depurado
    /*
    switch (true) {
        case stringContiene(comando,"SELECT"):
        case stringContiene(comando,"SELECT"):
        case stringContiene(comando,"SELECT"):
        case stringContiene(comando,"SELECT"):
        case stringContiene(comando,"SELECT"):
        case stringContiene(comando,"SELECT"):
    }
    */

    // FIN SECCIÓN DE CONSOLA.

    /* LA PARTE DESTINADA A COMUNICACIÓN POR RED QUEDA COMENTADA
     * SE LA VA A DESCOMENTAR CUANDO:
     * (1) TERMINE LA ELABORACIÓN DE CONSOLA.
     * (2) SEA CAPAZ DE PROCESAR Queries LQL
     * (3) SEA CAPAZ DE MANTENER EL HILO DE CONSOLA Y DE RED EN PARALELO.

    // SOCKET
    socketEscuchaKernel = nuevoSocket(log_memoria);  // CREAR SOCKET
    if(socketEscuchaKernel == ERROR){                // CASO DE ERROR.
        log_error(log_memoria," ¡¡¡ ERROR AL CREAR SOCKET. SE TERMINA EL PROCESO. !!! ");
        return -1;
    }
    log_info(log_memoria, "SOCKET CREADO.Valor: %d.", socketEscuchaKernel);

    // PUERTO
    log_info(log_memoria, " *** SE VA A ASOCIAR SOCKET CON PUERTO ... *** ");
    log_info(log_memoria, "PUERTO A USAR: %d.", arc_config->puerto);

    // ASOCIAR "SOCKET" CON "PUERTO".
    asociarSocket( socketEscuchaKernel     // SOCKET
                  , arc_config->puerto      // PUERTO
                 , log_memoria          ); // LOG
    log_info(log_memoria, " *** PUERTO ASOCIADO A SOCKET EXITOSAMENTE. *** ");

    // ESCUCHAR
    socketEscuchar( socketEscuchaKernel    // SOCKET
                    , 10
                  , log_memoria         ); // LOG
    while(1){
        log_info(log_memoria," +++ esperando conexiones... +++ ");
        conexionEntrante = aceptarConexionSocket(socketEscuchaKernel,log_memoria);
        if(conexionEntrante == ERROR){
            log_error(log_memoria,"ERROR AL CONECTAR.");
            return -1;
        }
        buffer = malloc( 8 * sizeof(char) );
        recibiendoMensaje = socketRecibir(conexionEntrante,buffer,7,log_memoria);
        buffer[7] = "\\0";
        printf("Recibimos por socket: %s\n",buffer);
        log_info(log_memoria,"El mensaje que se recibio fue %s", buffer);
    }*/

    // FIN DE BLOQUE DE RED.

	if (log_memoria != NULL) {

		log_info(log_memoria, "FIN DE PROCESO MEMORIA");
		log_destroy   (log_memoria);
		log_memoria  = NULL        ;

	}


    return 0;

} // MAIN.

void inicioLogYConfig() {
	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
	log_info(log_memoria, " \n ========== Iniciación de Pool de Memoria ========== \n \n ");

	log_info(log_memoria, "(1) LOG CREADO. ");

	cargarConfiguracion();
	log_info(log_memoria, " *** CONFIGURACIÓN DE MEMORIA CARGADA. *** ");
}

/*-----------------------------------------------------------------------------
 * MEMORIA PRINCIPAL
 *-----------------------------------------------------------------------------*/

void armarMemoriaPrincipal(){

}


/*-----------------------------------------------------------------------------
 * MODIFICAR CONFIGURACION
 *-----------------------------------------------------------------------------*/

void modificarTiempoRetardoMemoria(int nuevoCampo) {
	arc_config->retardo_mem = nuevoCampo;
}

void modificarTiempoRetardoFileSystem(int nuevoCampo) {
	arc_config->retardo_fs = nuevoCampo;
}

void modificarTiempoRetardoGossiping(int nuevoCampo) {
	arc_config->retardo_gossiping = nuevoCampo;
}

void modificarTiempoRetardoJournal(int nuevoCampo) {
	arc_config->retardo_journal = nuevoCampo;
}

void modificarTIempoRetardo(int nuevoCampo, char* campoAModificar) {
	t_config* configFile;
	configFile = config_create(PATH_MEMORIA_CONFIG);
	if(configFile == ERROR){
		log_error(log_memoria, "[MODIFICAR TIEMPO RETARDO]NO se abrio el archivo de configuracion");
		imprimirError(log_memoria, "NO se abrio el archivo de configuracion para MODIFICAR TIEMPO RETARDO");
		return;
	} else {
		if(config_has_property(configFile, campoAModificar)) {
			log_info(log_memoria, "[MODIFICAR TIEMPO RETARDO]Modificando el campo '%s' con el nuevo valor %d", campoAModificar, nuevoCampo);
			config_set_value(configFile, campoAModificar, nuevoCampo);
			log_info(log_memoria, "[MODIFICAR TIEMPO RETARDO][Hecho] Se ha modificando el campo '%s' con el nuevo valor %d", campoAModificar, nuevoCampo);
			log_info(log_memoria, "[MODIFICAR TIEMPO RETARDO]Guardar nuevo dato en la estructura Config del modulo");
			if(strcmp(campoAModificar, "RETARDO_MEM")) {
				modificarTiempoRetardoMemoria(nuevoCampo);
			}
			if(strcmp(campoAModificar, "RETARDO_JOURNAL")) {
				modificarTiempoRetardoJournal(nuevoCampo);
			}
			if(strcmp(campoAModificar, "RETARDO_GOSSIPING")) {
				modificarTiempoRetardoGossiping(nuevoCampo);
			}
			if(strcmp(campoAModificar, "RETARDO_FS")) {
				modificarTiempoRetardoFileSystem(nuevoCampo);
			}
			log_info(log_memoria, "[MODIFICAR TIEMPO RETARDO]Se ha guardado el nuevo dato: '%s'-> %d", campoAModificar, nuevoCampo);

		} else {
			imprimirError(log_memoria, "[MODIFICAR TIEMPO RETARDO] NO existe ese campo al que se quiere modificar algo");
			return;
		}

	}
}


/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CONEXION ENTRE PROCESOS
 *-----------------------------------------------------------------------------*/

void crearConexionesConOtrosProcesos(){

	conectarConServidorLisandraFileSystem();
//	levantarServidor();
}

void conectarConServidorLisandraFileSystem() {
	imprimirAviso(log_memoria, "INICIAMOS LA CONEXION CON LFS");
		sockeConexionLF = nuevoSocket(log_memoria);


		if(sockeConexionLF == ERROR){

			imprimirError(log_memoria, "ERROR al crear un socket");
			abortarProcesoPorUnErrorImportante(log_memoria, "NO se creo el socket con LisandraFS. Salimos del Proceso");
		}

		imprimirVerde(log_memoria, "Se ha creado un socket correctamente");

		log_info(log_memoria,
					"El Socket creado es: %d .",sockeConexionLF);

		log_info(log_memoria,
					"por llamar a la funcion connectarSocket() para conectarnos con Memoria");

		log_info(log_memoria,"PRUEBA: %d ",arc_config->puerto_fs);

		char* ipLFS = "127.0.0.1";
		int resultado_Conectar = conectarSocket(sockeConexionLF, ipLFS, arc_config->puerto_fs,log_memoria); // ## Acá IP de LFS Hardcodeada (#001#)

		if(resultado_Conectar == ERROR){
			abortarProcesoPorUnErrorImportante(log_memoria, "Hubo un problema al querer Conectarnos con LFS. Salimos del proceso");

		}else{

			imprimirVerde1(log_memoria,
					"Nos conectamos con exito, el resultado fue %d",resultado_Conectar);


		char * mensaje = "hola Lisandra";
		resultado_sendMsj = socketEnviar(sockeConexionLF,mensaje,strlen(mensaje),log_memoria);

		log_info(log_memoria,
						"Se ha intentado mandar un mensaje al server");

		if(resultado_sendMsj == ERROR){
			abortarProcesoPorUnErrorImportante(log_memoria, "Error al enviar mensaje a LSF. Salimos");

		}

		imprimirVerde1(log_memoria,"El mensaje se envio correctamente\n\nMENSAJE ENVIADO: %s", mensaje);

		buffer = malloc(sizeof(char));

		/*
		recibiendoMensaje = socketRecibir(sockeConexionLF, buffer, 13,  log_memoria);

		if(resultado_sendMsj == ERROR){
			imprimirError(log_memoria, "Error al recibir mensaje de LSF. salimos");
			return;
		}


		imprimirVerde1(log_memoria,"Se ha recibido un mensaje de LISANDRA\n\nMENSAJE RECIBIDO: %s", buffer);
		 */

		recibiendoMensaje = socketRecibir(sockeConexionLF, buffer, 3,  log_memoria);

				if(resultado_sendMsj == ERROR){
					imprimirError(log_memoria, "Error al recibir mensaje de LSF. salimos");
					return;
				}

		imprimirVerde1(log_memoria,"Se ha recibido el mensaje MAX VALUE de LISANDRA\n\nMENSAJE RECIBIDO: %s", buffer);


		//GUARDO ESTE VALOR QUE VA A SER IMPORTANTE PARA ARMAR LAS TABLAS
		log_info(log_memoria, "Guardando el valor MAX_VALUE_KEY: %s", buffer);

		arc_config->max_value_key = atoi(buffer);

		log_info(log_memoria, "max_value_key guardado: %i", arc_config->max_value_key);

		log_info(log_memoria, "Por liberar BUFFER");
		free(buffer);
		log_info(log_memoria, "BUFFER liberado");

	//	return;

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

void levantarServidor() {
	// SOCKET
	    socketEscuchaKernel = nuevoSocket(log_memoria);  // CREAR SOCKET
	    if(socketEscuchaKernel == ERROR){                // CASO DE ERROR.
	        log_error(log_memoria," ¡¡¡ ERROR AL CREAR SOCKET. SE TERMINA EL PROCESO. !!! ");
	        abortarProcesoPorUnErrorImportante(log_memoria, "NO se crea el socket correctamente");

	    }

	    log_info(log_memoria, "SOCKET CREADO.Valor: %d.", socketEscuchaKernel);

	    // PUERTO
	    log_info(log_memoria, " *** SE VA A ASOCIAR SOCKET CON PUERTO ... *** ");
	    log_info(log_memoria, "PUERTO A USAR: %d.", arc_config->puerto);

	    // ASOCIAR "SOCKET" CON "PUERTO".
	    asociarSocket( socketEscuchaKernel     // SOCKET
	                  , arc_config->puerto      // PUERTO
	                 , log_memoria          ); // LOG
	    log_info(log_memoria, " *** PUERTO ASOCIADO A SOCKET EXITOSAMENTE. *** ");

	    // ESCUCHAR
	    crearHIloEscucharKernel();
}

void crearHIloEscucharKernel() {
	pthread_t hiloEscucharKernel;
	hiloCrear(&hiloEscucharKernel, (void*)escucharConexionKernel, NULL);

	// NO SE SI DEBE ESTAR ASI, LO DEJO POR SI ACASO
	pthread_detach(hiloEscucharKernel);
}

void escucharConexionKernel() {
	socketEscuchar( socketEscuchaKernel    // SOCKET
			                    , 10
			                  , log_memoria         ); // LOG
	while(1){
		log_info(log_memoria," +++ esperando conexiones... +++ ");
		conexionEntrante = aceptarConexionSocket(socketEscuchaKernel,log_memoria);
		if(conexionEntrante == ERROR){
			log_error(log_memoria,"ERROR AL CONECTAR.");
			abortarProcesoPorUnErrorImportante(log_memoria, "El socket que escucha a Kernel no se conecto");

		}
		buffer = malloc( 10 * sizeof(char) );

		recibiendoMensaje = socketRecibir(conexionEntrante, buffer, 10, log_memoria);
		log_info(log_memoria, "Recibimos por socket %s",buffer);
		log_info(log_memoria,"El mensaje que se recibio fue %s", buffer);
	}
}

/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CONSOLA
 *-----------------------------------------------------------------------------*/

void ejecutarHiloConsola(){

	pthread_t hiloConsola;

	log_info(log_memoria, "Inicializando HILO CONSOLA");

	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);

	//DUDAS RESPECTO A ESTE HILO, SI PONGO ESTO EMPIEZA A EJECUTAR Y NO PERMITIRA QUE OTROS ENTREN O QUE?
	pthread_detach(hiloConsola);
}

char* lectura_consola() {
    char* linea = (char*)readline(">");
    return linea;
}

void consola() {

	log_info(log_memoria, "En el hilo de consola");

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

		add_history(linea);

		free(linea);

		//comandoSeparado = string_split(bufferComando, separador2);
		comandoSeparado = string_split(bufferComando, separator);


		//Tamanio del array

		for (int i = 0; comandoSeparado[i] != NULL; i++) {

			log_info(log_memoria,"En la posicion %d del array esta el valor %s",i,comandoSeparado[i]);

			tamanio = i + 1;
		}

		log_info(log_memoria, "El tamanio del vector de comandos es: %d",
				tamanio);

		switch (tamanio){

			case 1:
				{
					comandoSeparado = string_split(bufferComando, separador2);
					log_info(log_memoria,"%s",comandoSeparado[0]);
					log_info(log_memoria,"%d",strcmp(comandoSeparado[0],"salir"));
					if(strcmp(comandoSeparado[0],"salir") == 0){

						 printf("Salir seleccionado\n");
						log_info(log_memoria, "Se selecciono Salir");

						return;
		 			}else{
		 				printf("Comando mal ingresado. \n");
		 				log_error(log_memoria,
		 									"Opcion mal ingresada por teclado en la consola");

		 				break;
		 			}

				}
			case 2:
				validarComando(comandoSeparado[0],log_memoria);
				break;
			case 3:
				validarComando(comandoSeparado[0],log_memoria);
				break;
			default:
				{
				printf("Comando mal ingresado. \n");
				log_error(log_memoria,
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
		 /*	if(strcmp(comandoSeparado[0],"insert") == 0){

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

void menu() {

	printf("Los comandos que se pueden ingresar son: \n"
			"COMANDOS \n"
			"Insert \n"
			"Select \n"
			"Create \n"
			"Describe \n"
			"Drop \n"
			"Journal  \n"
			"SALIR \n"
			"\n");

}

void validarComando(char* comando,t_log* logger){

		int resultadoComando = buscarComando(comando,logger);

		switch (resultadoComando) {

			case Select: {
				printf("Se selecciono Select\n");

				log_info(logger, "Por llamar a enviarResultado");

				int resultadoEnviarComando = enviarComando(comando,logger);

			}
				break;

			case insert: {
				printf("Insert seleccionado\n");
				log_info(logger, "Se selecciono insert");

			}
				break;

			case create: {
				printf("Create seleccionado\n");
				log_info(logger, "Se selecciono Create");

			}
				break;

			case describe: {
				printf("Describe seleccionado\n");
				log_info(logger, "Se selecciono Describe");

			}
				break;

			case drop: {
				printf("Drop seleccionado\n");
				log_info(logger, "Se selecciono Drop");

			}
				break;

			case journal: {
				printf("Journal seleccionado\n");
				log_info(logger, "Se selecciono Journal");

			}
				break;


			default: {
				printf("Comando mal ingresado. \n");
				log_error(logger
						,
					"Opcion mal ingresada por teclado en la consola");
			}
				break;

		}
}

int enviarComando(char** comando, t_log* logger) {

	log_info(logger, "En funcion enviarComando");

	char* msj = malloc(7 * sizeof(char));

	msj = comando;

	log_info(logger, "El mensaje que vamos a enviar es: %s", msj);


	//AQUI TENGO UNA DUDA, LA CONEXION:

	//LA CONEXION SE INICIA EN
	//conectarConServidorLisandraFileSystem
	//POR LO TANTO YA TENGO EL INT DE ESO ASI QUE NO TENGO QUE VOLVER A
	//INICIARLA
//	sockeConexionLF = conexionKernel();

	log_info("Vamos a enviar a memoria por el socket %d", sockeConexionLF);

	resultado_sendMsj = socketEnviar(sockeConexionLF, msj, strlen(msj),log_memoria);

	if (resultado_sendMsj == ERROR) {

		log_error(log_memoria, "Error al enviar mensaje a memoria. Salimos");

		return ERROR;
	}

	log_info(log_memoria, "El mensaje se envio correctamente: %s", msj);

	return 0;

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

/*-----------------------------------------------------------------------------
 * FUNCIONALIDADES PARA LA CARGA DE LA CONFIGURACION Y EL LOG
 *-----------------------------------------------------------------------------*/
void cargarConfiguracion() {

    log_info(log_memoria, "[CONFIGURANDO MODULO] RESERVAR MEMORIA.");
    arc_config = malloc(sizeof(t_memoria_config));


    log_info(log_memoria, "[CONFIGURANDO MODULO] BUSCANDO CONFIGURACION.");
    t_config* configFile;
    configFile = config_create(PATH_MEMORIA_CONFIG);

    if (configFile != NULL) {

        log_info(log_memoria, "[CONFIGURANDO MODULO] LEYENDO CONFIGURACION...");

        if(config_has_property(configFile,"PUERTO")){

            arc_config->puerto = config_get_int_value( configFile , "PUERTO"   );
            log_info(log_memoria, "PUERTO PARA MODULO MEMORIA: %d", arc_config->puerto);

        }else {
            log_error(log_memoria, "[ERROR] NO HAY PUERTO CONFIGURADO");
        } // PUERTO

        if(config_has_property(configFile,"IP_FS")){

            arc_config->ip_fs = config_get_string_value( configFile , "IP_FS" );
            log_info(log_memoria, "[CONFIGURANDO MODULO] IP DE FILESYSTEM: %s", arc_config->ip_fs);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY IP CONFIGURADA" );
        } // IP FS

        if(config_has_property(configFile,"PUERTO_FS")){

            arc_config->puerto_fs = config_get_int_value(configFile,"PUERTO_FS");
            log_info(log_memoria, "[CONFIGURANDO MODULO] PUERTO DE FILESYSTEM: %d", arc_config->puerto_fs);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY PUERTO PARA MODULO FS");
        } // PUERTO FS

        if(config_has_property(configFile,"IP_SEEDS")){

            arc_config->ip_seeds = config_get_array_value(configFile,"IP_SEEDS");
            log_info(log_memoria, "[CONFIGURANDO MODULO] IP DE SEEDS: %s", arc_config->ip_fs);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY IPS PARA SEEDS");
        } // IP SEEDS

        if(config_has_property(configFile,"PUERTO_SEEDS")){
        	arc_config->puerto_seeds = config_get_array_value(configFile,"PUERTO_SEEDS");
            log_info(log_memoria, "[CONFIGURANDO MODULO] PUERTOS PARA SEEDS: %d", arc_config->puerto_seeds);

        }else{
            log_error(log_memoria, "[ERROR] NO SE ENCONTRARON LOS PUERTOS DE SEEDS");
        } // PUERTOS SEEDS


        if(config_has_property(configFile,"RETARDO_MEM")){

            arc_config->retardo_mem = config_get_int_value(configFile,"RETARDO_MEM");
            log_info(log_memoria, "[CONFIGURANDO MODULO] RETARTDO MEMORIA: %d", arc_config->retardo_mem);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY RETARDO CONFIGURADO");
        } // RETARDO DE MEMORIA

        if(config_has_property(configFile,"RETARDO_FS")){

            arc_config->retardo_fs = config_get_int_value(configFile,"RETARDO_FS");
            log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO DEL FS: %d", arc_config->retardo_fs);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY RETARDO DE FS CONFIGURADO");
        } // RETARDO FS

        if(config_has_property(configFile,"TAM_MEM")){

            arc_config->tam_mem = config_get_int_value(configFile,"TAM_MEM");
            log_info(log_memoria, "[CONFIGURANDO MODULO] TAMAÑO DE MEMORIA: %d", arc_config->tam_mem);

        }else{
            log_error(log_memoria, "[ERROR]NO HAY TAMAÑO DE MEMORIA CONFIGURADO");
        } // TAMAÑO DE MEMORIA

        if(config_has_property(configFile,"RETARDO_JOURNAL")){

            arc_config->retardo_journal = config_get_int_value(configFile,"RETARDO_MEM");
            log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO DEL JOURNALING: %d", arc_config->retardo_journal);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY RETARDO DE JOURNALING CONFIGURADO");
        } // RETARDO JOURNALING

        if(config_has_property(configFile,"RETARDO_GOSSIPING")){

            arc_config->retardo_gossiping = config_get_int_value(configFile,"RETARDO_GOSSIPING");
            log_info(log_memoria, "[CONFIGURANDO MODULO] RETARDO DE GOSSIPING: %d", arc_config->retardo_gossiping);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY RETARDO DE GOSSIPING CONFIGURADO");
        } // RETARDO GOSSIPING

        if(config_has_property(configFile,"MEMORY_NUMBER")){

            arc_config->memory_number = config_get_int_value(configFile,"MEMORY_NUMBER");
            log_info(log_memoria, "[CONFIGURANDO MODULO] NUMERO DE MEMORIA: %d", arc_config->retardo_mem);

        }else{
            log_error(log_memoria, "[ERROR] NO HAY NUMERO DE MEMORIA CONFIGURADO");
        } // MEMORY NUMBER

    }else{

        log_error(log_memoria,"[WARNING] NO HAY ARCHIVO DE CONFIGURACION DE MODULO MEMORIA"); // ERROR: SIN ARCHIVO CONFIGURACION

    }

}
