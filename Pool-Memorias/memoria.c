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
//#include "administrarMemoria.c"

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

	crearConexionesConOtrosProcesos(); // conecta con LFS y puede que con kernel.

    ejecutarHiloConsola();

    armarMemoriaPrincipal();

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
        buffer = malloc(sizeof(t_header));
        recibiendoMensaje = socketRecibir(conexionEntrante,buffer,sizeof(t_header),log_memoria);

        printf("Recibimos por socket el comando: %d\n",buffer->comando);
        log_info(log_memoria,"El mensaje que se recibio fue con el comando %d", buffer->comando);

		printf("Recibimos por socket el tamanio que vendra en el body: %d\n",buffer->tamanio);
        log_info(log_memoria,"Recibimos un tamanio que vendra en el body de: %d", buffer->tamanio);

		printf("Recibimos por socket la cantidad de argumentos que vendran en el body: %d\n",buffer->cantArgumentos);
        log_info(log_memoria,"Recibimos la cantidad de argumentos que vendran en el body de: %d", buffer->cantArgumentos);

		log_info(log_memoria,"El valor de retorno de la funcion que recibio el mensaje fue: %d",recibiendoMensaje);
		log_info(log_memoria,"El tamanio de la estructura t_header es: %d",sizeof(t_header));
		if(recibiendoMensaje == sizeof(t_header)){

			log_info(log_memoria,"Por enviar confirmacion a Kernel de que recibimos correctamente");

			log_info(log_memoria,"El tamanio de la confirmacion que enviamos es de: %d",sizeof(recibiendoMensaje));
			int resultadoEnvio = socketEnviar(conexionEntrante,&recibiendoMensaje,sizeof(recibiendoMensaje),log_memoria);

			log_info(log_memoria,"Por hacer un malloc de: %d para guardar el body. ",buffer->tamanio);
			argumentosComando = malloc(buffer->tamanio);
			
			memset(argumentosComando,'\0',buffer->tamanio);

			recibiendoMensaje = socketRecibir(conexionEntrante,argumentosComando,buffer->tamanio,log_memoria);

			log_info(log_memoria, "Recibimos el/los argumentos: %s",argumentosComando);
			printf("Recibimos el/los argumentos: %s \n",argumentosComando);			

			log_info(log_memoria, "Por parsear los argumentos.");

			argumentosParseados = string_split(argumentosComando, SEPARADOR);

			for (int i = 0; argumentosParseados[i] != NULL; i++) {
			
				log_info(log_memoria, "Parseando queda en la posicion %i: el valor: %s",i,argumentosParseados[i]);
				printf("Parseando queda en la posicion %i: el valor: %s \n",i,argumentosParseados[i]);

			}

			log_info(log_memoria,"Fin de parseo");
			printf("Fin de parseo. \n");

		}
    }

*/

    // FIN DE BLOQUE DE RED.

	if (log_memoria != NULL) {

		log_info(log_memoria, ">>>>>>>>>>>>>>>FIN DE PROCESO MEMORIA<<<<<<<<<<<<<<<");
		log_destroy   (log_memoria);
		log_memoria  = NULL        ;

	}



    return 0;

} // MAIN.

void inicioLogYConfig() {
	log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
	log_info(log_memoria, " \n ========== Iniciación de Pool de Memoria ========== \n \n ");

	log_info(log_memoria, "[LOGYCONFIG](1) LOG CREADO. ");

	cargarConfiguracion();
	log_info(log_memoria, "[LOGYCONFIG] *** CONFIGURACIÓN DE MEMORIA CARGADA. *** ");
}

/*-----------------------------------------------------------------------------
 * MEMORIA PRINCIPAL
 *-----------------------------------------------------------------------------*/

void armarMemoriaPrincipal(){

	log_info(log_memoria, "[ARMAR MEMORIA] Armo la memoria, guardo su tamaño");
	memoria = malloc(arc_config->tam_mem);
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria");
	if(memoria == NULL){
		log_info(log_memoria, "[ARMAR MEMORIA] NO se ha creado la memoria");
		abortarProcesoPorUnErrorImportante(log_memoria, "[ARMAR MEMORIA] NO se ha guardado correctamente el tamaño a memoria");
	}
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria");

	log_info(log_memoria, "[ARMAR MEMORIA] Guardo tamaño de memoria: %d", sizeof(int)*arc_config->tam_mem);
	//ESTO ES PARA SABER CUANTA MEMORIA REAL TIENE DISPONIBLE MEMORIA SIN CONTAR LO ADMINISTRATIVO
	memoria->tamanioMemoria = arc_config->tam_mem - sizeof(memoria);
	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de memoria guardada, MEMORIA REAL: %d", memoria->tamanioMemoria);
	limpiandoMemoria = 0;
	/*	NO SE SI ESTO ESTA BIEN
	log_info(log_memoria, "[ARMAR MEMORIA] Inicializo la tabla de segmentos, comienza en NULL");
	memoria->segmentoMemoria = [];
	log_info(log_memoria, "[ARMAR MEMORIA] Tabla de segmentos inicializada");
	*/

	imprimirVerde(log_memoria, "[ARMAR MEMORIA] Memoria inicializada de forma correcta");

	/*Creo que esto esta al pedo
	// CREACION DE ESTRUCTURAS.
	segmento* p_segmento_inicio;
	segmento* p_segmento_fin   ;
	p_segmento_inicio = malloc(sizeof(p_segmento_inicio));
	p_segmento_fin    = p_segmento_inicio;

	p_segmento_inicio->nro_segmento = 1;
	p_segmento_inicio->pagina       =

*/
	//LIBERA LA RAM.
	log_info(log_memoria, "[LIBERAR MEMORIA] Por liberar memoria");
	free(memoria);
	log_info(log_memoria, "[LIBERAR MEMORIA] memoria Liberada");

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
		imprimirError(log_memoria, "[MODIFICAR TIEMPO RETARDO]NO se abrio el archivo de configuracion para MODIFICAR TIEMPO RETARDO");
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

	log_info(log_memoria, "[HILOS] (+)");

	pthread_t hiloClienteLFS;
	pthread_create(&hiloClienteLFS, NULL, (void*) conectarConServidorLisandraFileSystem, NULL);

	pthread_detach(hiloClienteLFS);
	log_info(log_memoria, "[HILOS] LANZADO CLIENTE LFS");

	pthread_t hiloServidorKernel;
	pthread_create(&hiloServidorKernel, NULL, (void*) levantarServidor, NULL);

	pthread_detach(hiloServidorKernel);

	log_info(log_memoria, "[HILOS] LANZADO  SERVIDOR KERNEL");

	//conectarConServidorLisandraFileSystem();
	//levantarServidor();
	log_info(log_memoria, "[HILOS] (-)");
	// while(1);
}

void conectarConServidorLisandraFileSystem() {
	imprimirAviso(log_memoria, "[CONEXION LSF]INICIAMOS LA CONEXION CON LFS");
		sockeConexionLF = nuevoSocket(log_memoria);


		if(sockeConexionLF == ERROR){

			imprimirError(log_memoria, "[CONEXION LSF]ERROR al crear un socket");
			abortarProcesoPorUnErrorImportante(log_memoria, "NO se creo el socket con LisandraFS. Salimos del Proceso");
		}

		imprimirVerde(log_memoria, "[CONEXION LSF]Se ha creado un socket correctamente");

		log_info(log_memoria,
					"[CONEXION LSF]El Socket creado es: %d .",sockeConexionLF);

		log_info(log_memoria,
					"[CONEXION LSF]por llamar a la funcion connectarSocket() para conectarnos con Memoria");

		log_info(log_memoria,"[CONEXION LSF]PUERTO A CONECTAR: %d ",arc_config->puerto_fs);
		log_info(log_memoria,"[CONEXION LSF]PRUEBA: %d ",arc_config->puerto_fs);
		char* ipLFS = "127.0.0.1";
		int resultado_Conectar = conectarSocket(sockeConexionLF, "0", arc_config->puerto_fs,log_memoria);
		// ## Acá IP de LFS Hardcodeada (#001#)

		if(resultado_Conectar == ERROR){
			abortarProcesoPorUnErrorImportante(log_memoria, "[CONEXION LSF]Hubo un problema al querer Conectarnos con LFS. Salimos del proceso");

		}else{

			imprimirVerde1(log_memoria,
					"[CONEXION LSF]Nos conectamos con exito, el resultado fue %d",resultado_Conectar);


			char * mensaje = "hola Lisandra";

			resultado_sendMsj = socketEnviar(sockeConexionLF,mensaje,strlen(mensaje) +1,log_memoria);

			log_info(log_memoria, "[CONEXION LSF]Se ha intentado mandar un mensaje al server");

			if(resultado_sendMsj == ERROR){
					abortarProcesoPorUnErrorImportante(log_memoria, "[CONEXION LSF]Error al enviar mensaje a LSF. Salimos");
			}

			imprimirVerde1(log_memoria,"[CONEXION LSF]El mensaje se envio correctamente\n\nMENSAJE ENVIADO: %s", mensaje);

			while (1) {


				mensaje = readline(">");

			} // while (1)


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
					imprimirError(log_memoria, "[CONEXION LSF]Error al recibir mensaje de LSF. salimos");
					return;
				}

		imprimirVerde1(log_memoria,"[CONEXION LSF]Se ha recibido el mensaje MAX VALUE de LISANDRA\n\nMENSAJE RECIBIDO: %s", buffer);


		//GUARDO ESTE VALOR QUE VA A SER IMPORTANTE PARA ARMAR LAS TABLAS
		log_info(log_memoria, "[CONEXION LSF]Guardando el valor MAX_VALUE_KEY: %s", buffer);

		arc_config->max_value_key = atoi(buffer);

		log_info(log_memoria, "[CONEXION LSF]max_value_key guardado: %i", arc_config->max_value_key);

		log_info(log_memoria, "[CONEXION LSF]Por liberar BUFFER");
		free(buffer);
		log_info(log_memoria, "[CONEXION LSF]BUFFER liberado");

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
	        return;
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
	            return;
	        }
	        buffer = malloc(sizeof(t_header));
	        recibiendoMensaje = socketRecibir(conexionEntrante,buffer,sizeof(t_header),log_memoria);

	        printf("Recibimos por socket el comando: %d\n",buffer->comando);
	        log_info(log_memoria,"El mensaje que se recibio fue con el comando %d", buffer->comando);

			printf("Recibimos por socket el tamanio que vendra en el body: %d\n",buffer->tamanio);
	        log_info(log_memoria,"Recibimos un tamanio que vendra en el body de: %d", buffer->tamanio);

			printf("Recibimos por socket la cantidad de argumentos que vendran en el body: %d\n",buffer->cantArgumentos);
	        log_info(log_memoria,"Recibimos la cantidad de argumentos que vendran en el body de: %d", buffer->cantArgumentos);

			log_info(log_memoria,"El valor de retorno de la funcion que recibio el mensaje fue: %d",recibiendoMensaje);
			log_info(log_memoria,"El tamanio de la estructura t_header es: %d",sizeof(t_header));
			if(recibiendoMensaje == sizeof(t_header)){

				log_info(log_memoria,"Por enviar confirmacion a Kernel de que recibimos correctamente");

				log_info(log_memoria,"El tamanio de la confirmacion que enviamos es de: %d",sizeof(recibiendoMensaje));
				int resultadoEnvio = socketEnviar(conexionEntrante,&recibiendoMensaje,sizeof(recibiendoMensaje),log_memoria);

				log_info(log_memoria,"Por hacer un malloc de: %d para guardar el body. ",buffer->tamanio);
				argumentosComando = malloc(buffer->tamanio);

				memset(argumentosComando,'\0',buffer->tamanio);

				recibiendoMensaje = socketRecibir(conexionEntrante,argumentosComando,buffer->tamanio,log_memoria);

				log_info(log_memoria, "Recibimos el/los argumentos: %s",argumentosComando);
				printf("Recibimos el/los argumentos: %s \n",argumentosComando);

				log_info(log_memoria, "Por parsear los argumentos.");

				argumentosParseados = string_split(argumentosComando, SEPARADOR);

				for (int i = 0; argumentosParseados[i] != NULL; i++) {

					log_info(log_memoria, "Parseando queda en la posicion %i: el valor: %s",i,argumentosParseados[i]);
					printf("Parseando queda en la posicion %i: el valor: %s \n",i,argumentosParseados[i]);

				}

				log_info(log_memoria,"Fin de parseo");
				printf("Fin de parseo. \n");

			}
	    }
}

void crearHIloEscucharKernel() {
	pthread_t hiloEscucharKernel;
	log_info(log_memoria, "[HILO SERVER] *** HILO CREADO PARA ESCUCHA PERMANENTE *** ");
	hiloCrear(&hiloEscucharKernel, (void*)escucharConexionKernel, NULL);

	// NO SE SI DEBE ESTAR ASI, LO DEJO POR SI ACASO
	pthread_detach(hiloEscucharKernel);
	log_info(log_memoria, "[HILO SERVER] ESCUCHANDO A LOS CLIENTES");
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

	log_info(log_memoria, "[HILO CONSOLA]Inicializando HILO CONSOLA");

	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	log_info(log_memoria, "[HILO CONSOLA]Se crea HILO CONSOLA");
	//DUDAS RESPECTO A ESTE HILO, SI PONGO ESTO EMPIEZA A EJECUTAR Y NO PERMITIRA QUE OTROS ENTREN O QUE?
	pthread_detach(hiloConsola);
	log_info(log_memoria, "[HILO CONSOLA]HILO CONSOLA en ejecucion");
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

/*-----------------------------------------------------
 * FUNCIONES DE JOURNAL
 *-----------------------------------------------------*/

void JOURNAL(pagina* paginaAPasar, char* pathTabla) {
	log_info(log_memoria, "[JOURNAL] EN JOURNAL");

	log_info(log_memoria, "[JOURNAL] PROCEDO A ENVIAR LA INFORAMCION A LISANDRA");

	log_info(log_memoria, "[JOURNAL] ENVIO LA CANTIDAD EXACTA DE CARACTERES QUE LE VOY A ENVIAR");

	log_info(log_memoria, "[JOURNAL] TAMAÑO ENVIADO");

	log_info(log_memoria, "[JOURNAL] Lisandra responde que se puede enviar todo, procedo a hacerlo");

	log_info(log_memoria, "[JOURNAL] JOURNAL HECHO, LISANDRA LA HA RECIBIDO BIEN");
}

void pasar_valores_modificados_a_Lisandra(segmento* elSegmento, unidad_memoria* unidad_de_memoria){
	//DEBO IDENTIFICAR PRIMERO LA PAGINA QUE QUIERO PASAR LA INFORMACION
	pagina* unaPagina;
	int i=0;
	log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] En pasar_valores_modificados_a_Lisandra");
	log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] Me pongo a buscar la pagina referida a unidad de memoria");
	while(i==0){
		unaPagina = elSegmento->reg_pagina->siguientePagina;
		if(elSegmento->reg_pagina->numero == unidad_de_memoria->nroPagina) {
			//SE ENCONTROLA PAGINA BUSCADA
			unaPagina = elSegmento->reg_pagina;
			i++;
		} else {
			elSegmento->reg_pagina = unaPagina;
		}
	}
	log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] Se ha encontrado la pagina referida");
	if(unidad_de_memoria->flagModificado){
		//LA PAGINA FUE MODIFICADA, PROCEDO A HACER 1 JOURNAL
		log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] La pagina fue modificada, hago JOURNAL");
		JOURNAL(unaPagina, elSegmento->path_tabla);
		log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] Se ha hecho 1 journal");
	}
	log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] No esta modificada la pagina, asi que no hago nada aqui");
}

/*-----------------------------------------------------
 * FUNCIONES PARA LA ADMINISTRACION DE MEMORIA
 *-----------------------------------------------------*/


segmento * segmento_crear(char* path,  char* nombreTabla, pagina* pag) {
	segmento * segmentoNuevo = malloc(sizeof(segmento));
	segmentoNuevo->path_tabla = path;
	segmentoNuevo->reg_pagina=pag;
	segmentoNuevo->siguienteSegmento=NULL;
	segmentoNuevo->nombreTabla=nombreTabla;
//	segmentoNuevo->tamanio_segmento = sizeof(segmentoNuevo);
	return segmentoNuevo;
}

pagina * pagina_crear(valor_pagina* valor, int numero) {
	pagina* pag = malloc(sizeof(pagina));
	pag->numero=numero;
	pag->flag=false;
	pag->valor_pagina=valor;
	pag->siguientePagina = NULL;
	return pag;
}

valor_pagina * valor_pagina_crear(int timestamp, int16_t key, char * valor){
	valor_pagina * var = malloc(sizeof(valor_pagina));
	var->key = key;
	var->value = valor;

	//NO SE SI ESTA BIEN ESTO
	if(timestamp==0) {
		var->timestamp = (unsigned)time(NULL);
	} else {
		var->timestamp = timestamp;
	}

	return var;
}

/*
bool chequear_si_memoria_tiene_espacio(int espacioAOcupar){
	return (memoria->tamanioMemoria) > (espacioAOcupar + memoria->);
}
*/

void segmento_agregar_pagina(segmento* seg, pagina* pag) {
	pagina* aux = seg->reg_pagina;
	if(chequear_si_memoria_tiene_espacio(sizeof(pagina)+sizeof(valor_pagina))){
		//HAY ESPACIO SUFICIENTE
		pag->siguientePagina = aux;
		pag->numero = (seg->reg_pagina->numero)+1;
		seg->reg_pagina = pag;
		//ESTE ATRIBUTO YA NO EXISTE
	//	seg->tamanio_segmento += sizeof(pag);
	}
	//SE SUPONE QUE LA PAGINA YA TIENE LOS VALORES
	//CARGADO
}

//SE PUEDE MODIFICAR ESTO
bool chequear_si_memoria_tiene_espacio(int tamanio) {
	return memoria->tamanioMemoria > tamanio;
}

void pagina_agregar_valores(pagina* pag, valor_pagina* valor) {
	pag->valor_pagina=valor;
}

//ESTE SE USA PARA AQUELLOS VALORES QUE YA SE LOS IDENTIFICO
void valores_reemplazar_items(valor_pagina* valor_pag, int timestamp, char* valor){
	valor_pag->timestamp=timestamp;
	valor_pag->key=valor;
}

void pagina_poner_estado_modificado(pagina* pag) {
	pag->flag=true;
}

/*
int segmento_agregar_inicio_pagina(segmento* seg, pagina* pag){

}
*/


/*
int segmento_esta_vacio(segmento* seg){

	return
}
*/

//CUANDO HABLAMOS DE LIMPIAR MEMORIA, NOS REFERIMOS DE LIMPIAR TODA MEMORIA
void limpiar_memoria(memoria_principal* mem){
	//SE DEBE USAR UN SEMAFORO AQUI
	limpiandoMemoria=1;
	unidad_memoria* unidadTemporal;
	while(mem->unidad!=NULL){
		unidadTemporal= mem->unidad->siguienteUnidad;
		limpiarUnidad(&(mem->unidad));

		mem->unidad= unidadTemporal;
	}
	limpiandoMemoria=0;
//	mem->memoriaDisponible = mem->tamanioMemoria;
}

//ESTO SOLO OCURRE CUANDO SE HACE JOURNAL
//O ESO FUE LO QUE HE ENTENDIDO
int limpiarUnidad(unidad_memoria* unidad_de_memoria){
	//DEBE IDENTIFICAR SEGMENTO Y PAGINA A LA QUE LE CORRESPONDE
	log_info(log_memoria, "[LIMPIAR UNIDAD] Entro a limpiar Unidad, procedo a buscar el segmento por su numero");
	segmento* elSegmento = buscarSegmentoPorNumero(unidad_de_memoria->nroSegmento);
	log_info(log_memoria, "[LIMPIAR UNIDAD] Realizo la operacion Journal");
	pasar_valores_modificados_a_Lisandra(elSegmento, unidad_de_memoria);
	log_info(log_memoria, "[LIMPIAR UNIDAD] JOURNAL DE UNIDAD REALIZADA");
	log_info(log_memoria, "[LIMPIAR UNIDAD] Procedo a limpiar la unidad y tambien la unidad");
	free(unidad_de_memoria);
	return limpiar_segmento(&elSegmento);
}

segmento* buscarSegmentoPorNumero(int numeroABuscar){
	segmento* segmentoAux = tablaSegmentos;
	bool encontrado = false;
	log_info(log_memoria, "[BUSCANDO SEGMENTO X NRO] En Buscar segmento por numero");
	while(segmentoAux!=NULL){
		//SI O SI LO VA A ENCONTRAR PORQUE NO PUEDE ESTAR EN MEMORIA Y NO EXISTIR EN LA TABLA
		log_info(log_memoria, "[BUSCANDO SEGMENTO X NRO] Buscando segmento");
		if(segmentoAux->nro_segmento == numeroABuscar){
			log_info(log_memoria, "[BUSCANDO SEGMENTO X NRO] SEGMENTO ENCONTRADO");
			return segmentoAux;
		} else {
			log_info(log_memoria, "[BUSCANDO SEGMENTO X NRO] No se encontro, paso al siguiente segmento");
			segmentoAux = segmentoAux->siguienteSegmento;
		}
	}
	//NO LO ENCONTRO, DEVUELVO NULL
	log_info(log_memoria, "[BUSCANDO SEGMENTO X NRO] NO se encontro el segmento buscado");
	return NULL;
}

pagina* buscarPaginaPorNumero(int numeroABuscar, segmento* seg){
	pagina* paginaAux = seg->reg_pagina;
	bool encontrado = false;
	log_info(log_memoria, "[BUSCANDO PAGINA X NRO] En Buscar pagina por numero");
	while(paginaAux!=NULL){
		//SI O SI LO VA A ENCONTRAR PORQUE NO PUEDE ESTAR EN MEMORIA Y NO EXISTIR EN LA TABLA
		log_info(log_memoria, "[BUSCANDO PAGINA X NRO] Buscando pagina");
		if(paginaAux->numero == numeroABuscar){
			log_info(log_memoria, "[BUSCANDO PAGINA X NRO] PAGINA ENCONTRADO");
			return paginaAux;
		} else {
			log_info(log_memoria, "[BUSCANDO PAGINA X NRO] No se encontro, paso a la siguiente pagina");
			paginaAux = paginaAux->siguientePagina;
		}
	}
	//NO LO ENCONTRO, DEVUELVO NULL
	log_info(log_memoria, "[BUSCANDO PAGINA X NRO] NO se encontro la pagina buscado");
	return NULL;
}

int limpiar_segmento(segmento * seg) {
	int cantidadLiberada=0;
	log_info(log_memoria, "[LIMPIAR SEGMENTO] En limpiar segmento, procedo a limpiar todas las paginas");
	cantidadLiberada+=limpiar_paginas(&seg->reg_pagina);
	log_info(log_memoria, "[LIMPIAR SEGMENTO] Todas las paginas fueron limpiadas");
	cantidadLiberada+=sizeof(seg);
	free(seg);
	log_info(log_memoria, "[LIMPIAR SEGMENTO] El segmento fue liberado");
	return cantidadLiberada;
}

int limpiar_paginas(pagina* pag){
	pagina* otraPagina;
	int cantidadLiberada =0;
	log_info(log_memoria, "[LIMPIAR PAGINAS] En limpiar tabla de paginas");
	while(pag!= NULL ){
		log_info(log_memoria, "[LIMPIAR PAGINAS] Limpiando 1 pagina");
		otraPagina=pag->siguientePagina;

//		cantidadLiberada +=limpiar_valores_pagina(&(pag->valor_pagina));
		cantidadLiberada += sizeof(pag) + sizeof(pag->valor_pagina);
		log_info(log_memoria, "[LIMPIAR PAGINAS] Liberando el valor de dicha pagina");
		free(pag->valor_pagina);
		log_info(log_memoria, "[LIMPIAR PAGINAS] Se ha liberado con exito el valor");
		log_info(log_memoria, "[LIMPIAR PAGINAS] Liberando la pagina");
		free(pag);
		log_info(log_memoria, "[LIMPIAR PAGINAS] Se ha liberado con exito la pagina");
		pag = otraPagina;
	}
	return cantidadLiberada;
}

/* CREO QUE ESTO ES INNECESARIO
int limpiar_valores_pagina(valor_pagina* valores){
	int cantidadLiberada = sizeof(valores);
	free(valores);
	return cantidadLiberada;
}
*/
/*
int buscar_tabla_especifica(char* nombreTablaABuscar, segmento* segmentoBuscado) {
	segmento* segTemporal;
	memoria_principal* memTemporal = memoria;
	while(memTemporal->segmentoMemoria!=NULL){
		//EMPIEZO A BUSCAR
		if(strcmp(
			memTemporal->segmentoMemoria->nombreTabla,
				nombreTablaABuscar))
		{
			//SE ENCONTRO LA TABLA BUSCADA
			segmentoBuscado = memTemporal->segmentoMemoria;
			return 1;
		}
		segTemporal = memTemporal->segmentoMemoria->siguienteSegmento;
		memTemporal->segmentoMemoria = segTemporal;
		//SE SIGUE BUSCANDO
	}
	return -1;
}
*/

segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	segmento* todoSegmento = tablaSegmentos;
	log_info(log_memoria, "[BUSCANDO KEY] Buscando el segmento referido para la tabla %s", nombreTabla);
	while(todoSegmento!=NULL){
		if(strcmp(todoSegmento->nombreTabla, nombreTabla)){
			//SE ENCONTRO EL SEGMENTO BUSCADO
			log_info(log_memoria, "[BUSCANDO SEGMENTO X NOMBRETABLA] Se encontro el segmento buscado");
			return todoSegmento;
		}
	}
	log_info(log_memoria, "[BUSCANDO KEY] NO se encontro el segmento buscado");
	return NULL;
}


bool buscarKeyEnRegistro(char* nombreTabla, unidad_memoria* unidadAAnalizar, valor_pagina** unidadADevolver, int16_t key){
	log_info(log_memoria, "[BUSCANDO KEY] Buscando 1 segmento");

	segmento* nuevoSegmento = buscarSegmentoPorNumero(unidadAAnalizar->nroSegmento);
//	segmento* nuevoSegmento = buscarSegmentoPorNombreTabla(nombreTabla);
	pagina* pagAux = nuevoSegmento->reg_pagina;

	log_info(log_memoria, "[BUSCANDO KEY] Obtengo el segmento del registro");

	if(nuevoSegmento!= NULL) {
		log_info(log_memoria, "[BUSCANDO KEY] Procedo a buscar entre sus paginas si esta la key");
		while(pagAux!=NULL) {
			if(pagAux->valor_pagina->key == key){
				log_info(log_memoria, "[BUSCANDO KEY] KEY ENCONTRADA");
				unidadAAnalizar = pagAux->valor_pagina;
				return true;
			}
			pagAux = pagAux->siguientePagina;
		}
	}
	log_info(log_memoria, "[BUSCANDO KEY] NO se encontro la key para este segmento");
	return false;
}

//ESTO SE HACE SIEMPRE DESPUES DE QUE BUSCA LA TABLA
//Y SOLO SI SE ENCUNETRA
int obtener_valores(char* nombreTabla, int16_t key, valor_pagina* unidadExtra) {
	valor_pagina* valor_pag;
	memoria_principal* memoriaAuxiliar = memoria;
	log_info(log_memoria, "[BUSCANDO VALORES] Empiezo a buscar el valor con key %d", key);

	//BUSCO PRIMERO REGISTOR POR REGISTRO, EMPEZANDO POR EL MAS ALTO DE LA COLA

	bool encontrado = false;

	while(memoriaAuxiliar->unidad!=NULL){
		unidadExtra = memoriaAuxiliar->unidad->siguienteUnidad;
	//	encontrado = buscarKeyEnRegistro(nombreTabla, unidadAAnalizar, unidadADevolver, key)
		encontrado = buscarKeyEnRegistro(nombreTabla, memoriaAuxiliar->unidad, &unidadExtra, key);
		if(encontrado){
			//SE ENCONTRO LA UNIDAD BUSCADA
			return 1;
		} else {
			memoriaAuxiliar->unidad = unidadExtra;
		}
	}

	/*
	pagina* otraPagina;

	segmento* otroSegmento = segmentoHost;
	while(otroSegmento->pagina!=NULL){
		if(key==otroSegmento->pagina->valor_pagina->key){
			//SE ENCONTRO LA COSA ESTA
			valorADevolver = otraPagina->valor_pagina;
			return 1;
		}
		otraPagina = otroSegmento->pagina->siguientePagina;
		otroSegmento->pagina = otraPagina;
	}
	*/
	return -1;
}
