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

    liberar_todo_por_cierre_de_modulo();
    return 0;
} // MAIN.

void liberar_todo_por_cierre_de_modulo(){
	//LIBERA LA RAM.

	if(memoriaArmada==1){
		log_info(log_memoria, "[LIBERAR] Por liberar memoria");
		free(bloque_memoria);
		log_info(log_memoria, "[LIBERAR] memoria Liberada");
	}
	if(tablaPaginaArmada){
		log_info(log_memoria, "[LIBERAR] Por liberar tabla_paginada");
				free(tabla_paginada);
				log_info(log_memoria, "[LIBERAR] tabla_paginada Liberada");
	}

		log_info(log_memoria, "[LIBERAR] Por liberar Struct configuracion");
		free(arc_config);
		log_info(log_memoria, "[LIBERAR] Struct configuracion Liberada");
		if (log_memoria != NULL) {
			log_info(log_memoria, ">>>>>>>>>>>>>>>FIN DE PROCESO MEMORIA<<<<<<<<<<<<<<<");
			log_destroy   (log_memoria);
			log_memoria  = NULL;
		}
}

void inicioLogYConfig() {
	memoriaArmada = 0;
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
	tablaPaginaArmada = 0;
	memoriaArmada =0;
	log_info(log_memoria, "[ARMAR MEMORIA] Armo el bloque de memoria, guardo su tamaño");
	bloque_memoria = malloc(arc_config->tam_mem);
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria contigua");
	if(bloque_memoria == NULL){
		log_info(log_memoria, "[ARMAR MEMORIA] NO se ha creado la memoria");
		liberar_todo_por_cierre_de_modulo();
		abortarProcesoPorUnErrorImportante(log_memoria, "[ARMAR MEMORIA] NO se ha guardado correctamente el tamaño a memoria");
	}
	log_info(log_memoria, "[ARMAR MEMORIA] Se ha creado la memoria");

	log_info(log_memoria, "[ARMAR MEMORIA] Guardo tamaño de memoria: %d", sizeof(int)*arc_config->tam_mem);
	//ESTO ES PARA SABER CUANTA MEMORIA REAL TIENE DISPONIBLE MEMORIA SIN CONTAR LO ADMINISTRATIVO
	//USANDO EL SIZEOF(MEMORIA) QUEDO PARADO EN LA BASE DONDE COMENZARA A HABER PAGINAS SIENDO
	//ESTA LA POSICION 0 o 1
	memoria->tamanioMemoria = arc_config->tam_mem - sizeof(memoria);

	//CON ESTO CONOCENERE CUANTO DEBERIA MOVERME POR BYTE PARA LLEGAR A UNA POSICION DE PAGINA

	//AGREGO EL MAX VALUE UNICAMENTE PORQUE EL CHAR* YA CUENTA 1 BYTE, Y EL MAX VALUE EL TAMAÑO REAL
	//EN BYTE QUE TENDRA EL VALOR EN CHAR* POR LO TANTO EL 1 ORIGINAL PASA A REPRESENTAR EL \0 QUE ES 1
	//BYTE MAS
	arc_config->max_value_key = 4;
			//HARDCODEADO PERO BUENO, NO SE QUE LE PASA AL CONFIG DE FILESYSTEM

	memoria->tamanioPagina = sizeof(pagina) + arc_config->max_value_key;
	cantPaginasDisponibles = memoria->tamanioMemoria/memoria->tamanioPagina;
	memoria->paginasTotales = cantPaginasDisponibles;
	cantPaginasTotales = cantPaginasDisponibles;
	//AQUI SE HAN INICIALIZADO LOS SEMAFOROS DE PAGINAS DISPONIBLES

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de memoria guardada, MEMORIA REAL: %d", memoria->tamanioMemoria);

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de pagina: %d", memoria->tamanioPagina);

	log_info(log_memoria, "[ARMAR MEMORIA] Cantidad maxima de paginas en memoria: %d", cantPaginasDisponibles);

	//AHORA DEBO INICIALIZAR EL BLOQUE DE TABLA PAGINAS
	log_info(log_memoria, "[ARMAR MEMORIA] Procedo a iniciar el bloque de memoria TABLA PAGINADA");
	int tamanioTablaPaginada = cantPaginasDisponibles*sizeof(tabla_pagina);
	tabla_paginada = malloc(tamanioTablaPaginada);
	if(tabla_paginada == NULL){
			log_info(log_memoria, "[ARMAR MEMORIA] NO se ha creado la tabla de paginas");
			liberar_todo_por_cierre_de_modulo();
			abortarProcesoPorUnErrorImportante(log_memoria, "[ARMAR MEMORIA] NO se ha guardado correctamente el tamaño de Tabla de paginas");
		}
	tablaPaginaArmada =1;
	log_info(log_memoria, "[ARMAR MEMORIA] Tabla paginada creada");

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño deL BLOQUE TABLA PAGINAs: %d", tamanioTablaPaginada);
	imprimirVerde(log_memoria, "Se ha creado el bloque de memoria de Tablas de paginas");

	//log_info(log_memoria, "[ARMAR MEMORIA] Procedo a guardar los datos administrativos de memoria en el bloque de memoria");

	//memcpy(0, sizeof(memoria_principal), memoria);

	imprimirVerde(log_memoria, "[ARMAR MEMORIA] Memoria inicializada de forma correcta");

	mutexIniciar(&memoria_mutex_paginas_disponibles);
	mutexIniciar(&mutex_tabla_pagina_en_modificacion);

	//PONGO ESTOS SEMAFOROS LISTOS PARA EMPEZAR A OPERAR
	memoriaArmada = 1;
	limpiandoMemoria = 0;
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
			liberar_todo_por_cierre_de_modulo();
			abortarProcesoPorUnErrorImportante(log_memoria, "[CONEXION LSF]Hubo un problema al querer Conectarnos con LFS. Salimos del proceso");

		}else{

			imprimirVerde1(log_memoria,
					"[CONEXION LSF]Nos conectamos con exito, el resultado fue %d",resultado_Conectar);


			char * mensaje = "hola Lisandra";

			resultado_sendMsj = socketEnviar(sockeConexionLF,mensaje,strlen(mensaje) +1,log_memoria);

			log_info(log_memoria, "[CONEXION LSF]Se ha intentado mandar un mensaje al server");

			if(resultado_sendMsj == ERROR){
				liberar_todo_por_cierre_de_modulo();
				abortarProcesoPorUnErrorImportante(log_memoria, "[CONEXION LSF]Error al enviar mensaje a LSF. Salimos");
			}

			imprimirVerde1(log_memoria,"[CONEXION LSF]El mensaje se envio correctamente\n\nMENSAJE ENVIADO: %s", mensaje);


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
		max_valor_key = arc_config->max_value_key;

		log_info(log_memoria, "[CONEXION LSF]max_value_key guardado: %i", arc_config->max_value_key);

		log_info(log_memoria, "[CONEXION LSF]Por liberar BUFFER");
		free(buffer);
		log_info(log_memoria, "[CONEXION LSF]BUFFER liberado");

		while (1) {


			mensaje = readline(">");

		} // while (1)
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
/*
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

	/* DEBE REVISAR LA MEMORIA PARA BUSCAR EL FLAG DE MODIFICADO
	if(unidad_de_memoria->flagModificado){
		//LA PAGINA FUE MODIFICADA, PROCEDO A HACER 1 JOURNAL
		log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] La pagina fue modificada, hago JOURNAL");
		JOURNAL(unaPagina, elSegmento->path_tabla);
		log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] Se ha hecho 1 journal");
	}

	log_info(log_memoria, "[VERIFICAR SI PASO VALORES A LFS] No esta modificada la pagina, asi que no hago nada aqui");
}
*/
/*-----------------------------------------------------
 * FUNCIONES PARA LA ADMINISTRACION DE MEMORIA
 *-----------------------------------------------------*/


segmento * segmento_crear(char* pathNombreTabla, int nroPagina) {
	segmento * segmentoNuevo = malloc(sizeof(segmento));
	segmentoNuevo->path_tabla = pathNombreTabla;
	/*
	 * A ESTO LE FALTA QUE SE INICIALICE BIEN EL ARRAY DINAMICO
	 */
//	segmentoNuevo->tablaPaginasAsociadas

	//SE QUE ESTO ESTA MAL PERO LO DEJO ASI POR AHORA
//	segmentoNuevo->tablaPaginasAsociadas = nroPagina
	segmentoNuevo->siguienteSegmento=tablaSegmentos->siguienteSegmento;
	tablaSegmentos = segmentoNuevo;
	return segmentoNuevo;
}

//CUANDO SE CREA UNA NUEVA PAGINA TAMBIEN SE CREA SU PAGINA Y SE ALMACENA
//EN BLOQUE MEMORIA MIENTRAS LA NUEVA TABLA DE PAGINA EN BLOQUE TABLA PAGINA
//ESTO EN EL CASO DE 1 INSERT O SELECT NUEVO
//
//ESTA FUNCION ES LLAMADA SOLO CUANDO SE TIENE QUE CREAR UNA NUEVA PAGINA
//DEVUELVE EL NRO DONDE SE ALOJO LA NUEVA TABLA
int * tabla_pagina_crear(int16_t key, long timestampNuevo, char* valor, bool flag_modificado) {
	log_info(log_memoria, "[Crear Tabla y pagina] En crear Tabla de pagina y pagina nueva porque no estan con la key %d", key);
	tabla_pagina* tabla_pagina_nueva;
	pagina* pagina_nueva_a_poner;
	tabla_pagina_nueva->flag=flag_modificado;
	tabla_pagina_nueva->key=key;
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	int posicionLibreAsignada = cantPaginasTotales - cantPaginasDisponibles;
	log_info(log_memoria, "[Crear Tabla y pagina] La posicion a la cual le voy a asignar es %d", posicionLibreAsignada);
	log_info(log_memoria, "[Crear Tabla y pagina] Tabla pagina creada y procedo a crear la pagina misma");
	pagina_nueva_a_poner = crear_pagina(timestampNuevo, key, valor);
	log_info(log_memoria, "[Crear Tabla y pagina] Pagina creada con: timestamp '%d'; KEY '%d'; y VALOR: '%s'", pagina_nueva_a_poner->timestamp, key, valor);
	//MIENTRAS EST{E BLOQUEADO, NO SE PUEDE QUITAR O PONER NUEVAS PAGINAS
	log_info(log_memoria, "[Crear Tabla y pagina] Verifico si hay espacio libre en Memoria");
	if(cantPaginasDisponibles<=0){
		log_info(log_memoria, "[Crear Tabla y pagina] NO hay espacio libre por lo tanto activo el LRU");
		//SIGNIFICA QUE LLEGO AL TOPE DE PAGINAS EN MEMORIA
		//O SEA ESTAN TODAS OCUPADAS, APLICO LRU
		LRU(pagina_nueva_a_poner);
	} else {
		log_info(log_memoria, "[Crear Tabla y pagina] Existen '%d' espacios LIBRES", cantPaginasDisponibles);
		//AUN HAY ESPACIO, GAURDO ESTA NUEVA PAGINA EN ALGUNA POSICION LIBRE
		tabla_pagina_nueva->posicion=posicionLibreAsignada;
		log_info(log_memoria, "[Crear Tabla y pagina] Asigno el espacio libre a la tabla pagina y a la pagina");
		pagina_nueva_a_poner = actualizarPosicionAPagina(pagina_nueva_a_poner, posicionLibreAsignada);
		log_info(log_memoria, "[Crear Tabla y pagina] Procedo a asignar la tabla y la pagina a dicha posicion");
		asignarNuevaTablaAPosicionLibre(tabla_pagina_nueva, posicionLibreAsignada, pagina_nueva_a_poner);
		log_info(log_memoria, "[Crear Tabla y pagina] ASIGNACION COMPLETADA");
	}
	log_info(log_memoria, "[Crear Tabla y pagina] Desactivo el mutex mutex_tabla_pagina_en_modificacion");
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
	return posicionLibreAsignada;
}

pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos){
	unaPagina->nroPosicion= (short)nuevaPos;
	return unaPagina;
}

void asignarNuevaTablaAPosicionLibre(tabla_pagina* tabla_pagina_nueva, int posLibre, pagina* pagina_nueva){
	memcpy(tabla_paginada+posLibre*sizeof(tabla_pagina), tabla_pagina_nueva, sizeof(tabla_pagina_nueva));
	memcpy(bloque_memoria+posLibre*sizeof(pagina), pagina_nueva, sizeof(pagina_nueva)+max_valor_key);
	cantPaginasDisponibles -= 1;
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Se han guardado exitosamente los datos\nAhora mismo hay '%d' espacios libres", cantPaginasDisponibles);
}

pagina* crear_pagina(long timestampNUevo, int16_t key, char * valor) {
	pagina* pag_nueva;
	pag_nueva->nroPosicion = 0;
	pag_nueva->key = key;
	strcpy(pag_nueva->value, valor);
	//SI TIENE TIMESTAMP EN 0 LE ASIGNAMOS EL ACTUAL
	if(timestampNUevo <= 0) {
		pag_nueva->timestamp = timestamp();
	} else {
		pag_nueva->timestamp = timestampNUevo;
	}
	return pag_nueva;
}

/*
pagina* pagina_crear(long timestampNuevo, int16_t key, char * valor, char* nombreTabla){
	//DEBO CHEQUEAR PRIMERO SI NO SE OCUPARON TODAS LAS PAGINAS
	pagina* nuevaPag = crear_pagina(timestampNuevo, key, valor);
	log_info(log_memoria, "[CREAR PAGINA] Veo de crear una pagina o reemplazar 1");
	mutexBloquear(&memoria_mutex_paginas_disponibles);
	if(memoria->cantPaginasDisponibles>0){
		mutexDesbloquear(&memoria_mutex_paginas_disponibles);
		log_info(log_memoria, "[CREAR PAGINA] Hay espacio, creo una pagina, pongo en el siguiente bloque libre");
		int posicionAPoner = memoria->paginasTotales - memoria->cantPaginasDisponibles;

		memcpy(memoria + sizeof(memoria) + posicionAPoner*memoria->tamanioPagina, &nuevaPag, memoria->tamanioPagina);
		log_info(log_memoria, "[CREAR PAGINA] Se ha guardado la nueva pagina, procedo a crear Segmento y tabla de pagina");

		aniadirNuevaPaginaASegmento(&nuevaPag, nombreTabla);
	} else {
		mutexDesbloquear(&memoria_mutex_paginas_disponibles);
		log_info(log_memoria, "[CREAR PAGINA] NO hay espacio, ejecuto LRU");
		LRU(&nuevaPag);

	}

	return nuevaPag;
}
*/

//void buscarKeyPorTablaPagina(tabla_pagina* tabla_pag, pagina* pag, bool aniadirSiNoHay, bool modificar) {
int buscarKeyPorTablaPagina(int posicionABuscar, int16_t keyBuscada) {
	tabla_pagina* tabla_pagina_aux;
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Busco en la pagina '%d'", posicionABuscar);
	memcpy(tabla_pagina_aux, tabla_paginada+posicionABuscar*sizeof(tabla_pagina), sizeof(tabla_pagina));
	//RETORNA LA POSICION DONDE SE ENCUNETRA LA PAGINA SINO DA -1 PASA A LA SIGUIENTE
	return tabla_pagina_aux->key==keyBuscada?tabla_pagina_aux->posicion:ERROR;
	/*
	tabla_pagina* paginas = tabla_pag;
	tabla_pagina* primero = tabla_pag;
	tabla_pagina* anterior;
			while(paginas!=NULL){
				//BUSCO EN SU REGISTOR SI ESTA LA KEY BUSCADA
				if(pag->key==paginas->valor_pagina->key){
					//ENCONTRADA
					if(modificar){
						//AQUI SE MODIFICA LA TABLA
					}
					return;
				}
				anterior = paginas;
				paginas = paginas->siguientePagina;
			}
	if(paginas == NULL && aniadirSiNoHay) {
		//NO SE ENCONTRO, PROCEDO A CREARLA Y PONERLA A LO ULTIMO
		paginas = tabla_pagina_crear(pag, anterior->numero+1);
		while(primero->siguientePagina!=NULL){
			primero = primero->siguientePagina;
		}
		primero->siguientePagina = paginas;
	}
*/
}

//INTENTA BUSCAR EN QUE POSICION DE LA MEMORIA CONTIGUA SE ENCUNETRA LA PAGINA CON ESA KEY
int buscarEntreTodasLasTablaPaginasLaKey(int tablasAsociadasASegmento[], int cantTablas,
							int16_t keyBuscada, int** nroDePagina){
	int i, posiblePosicion;
	nroDePagina = -1;
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Por buscar la posicion de la pagina que contiene la key '%d'", keyBuscada);
	for(i=0; i<cantTablas; i++){
		posiblePosicion=buscarKeyPorTablaPagina(tablasAsociadasASegmento[i], keyBuscada);
		if(posiblePosicion>ERROR){
			nroDePagina = tablasAsociadasASegmento[i];
			log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Se encontro la pagina buscada, paso su ubicacion");
			return posiblePosicion;
		}
		log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] No se la encontro, busco en la siguiente tabla");
	}
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] NO SE ENCONTRO LA KEY, SALIMOS POR ERROR");
	//SALIO DEL CICLO POR LO TANTO NO LO ENCONTRO
	return ERROR;
}

//Trata de buscar entre todos los segmentos la key buscada a partir de la key  y nombre tabla
int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(char* nombreTabla, int16_t keyBuscada, segmento** segmentoBuscado, int* nroDePagina){
	log_info(log_memoria, "[Buscar Key] Empiezo a buscar la key '%d' perteneciente a la tabla '%s'", keyBuscada, nombreTabla);
	segmento* seg_aux = buscarSegmentoPorNombreTabla(nombreTabla);
	segmentoBuscado = seg_aux;
	if(seg_aux!=NULL){
		//EXISTE EL SEGMENTO
		int cantidadElementos = sizeof(seg_aux->tablaPaginasAsociadas) / sizeof(int);
		log_info(log_memoria, "[Buscar Key] Se encontro el segmento con nombre '%s', contiene '%d' elementos", nombreTabla, cantidadElementos);
		return buscarEntreTodasLasTablaPaginasLaKey(seg_aux->tablaPaginasAsociadas, cantidadElementos, keyBuscada, &nroDePagina);
	}
	//NO EXISTE EL SEGMENTO
	log_info(log_memoria, "[Buscar Key] NO EXISTE NINGUN SEGMENTO ASOCIADA A '%s'; Devuelvo ERROR", nombreTabla);
	return ERROR;
}

void funcionInsert(char* nombreTabla, long timestamp, int16_t keyBuscada, char* valorAPoner){
	/*
	 * 1* Buscar la posicion donde se encuentra la pagina
	 * 		a* Encontrado, accedo a ella, modifico valor y timestamp y de paso la tabla pagina el flag
	 * 		b* NO encontado, creo el la tabla, pagina y segmento (en este orden)
	 */
	int nroPosicion;
	log_info(log_memoria, "[INSERT] EN funcion INSERT");
	segmento* segmentoBuscado;
	log_info(log_memoria, "[INSERT] Me pongo a buscar el segmento y la tabla en base a '%s' y '%d'", nombreTabla, keyBuscada);
	int posicionAIr = buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(nombreTabla, keyBuscada, &segmentoBuscado, &nroPosicion);
	int nroTablaCreada;
	if(posicionAIr==ERROR){
		log_info(log_memoria, "[INSERT] NO se encontro la posicion a donde debo ir");
		//CASO B, verifico si se encontro el segmento, caso contrario debo tambien crearlo
		if(segmentoBuscado==NULL){
			log_info(log_memoria, "[INSERT] Tampoco se encontro que existe un segmento asociado a la tabla '%s'", nombreTabla);
			//NO SE ENCONTRO NINGUN SEGMENTO CON EL NOMBRE DE LA TABLA BUSCADA POR LO TANTO DEBO CREARLA

			nroTablaCreada = tabla_pagina_crear(keyBuscada, timestamp, valorAPoner, true);
			log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina, el nro de su posicion es '%d'", nroTablaCreada);

			segmento_crear(nombreTabla, nroTablaCreada);
			log_info(log_memoria, "[INSERT] Se ha creado un segmento para la tabla '%s'", nombreTabla);
		} else {
			log_info(log_memoria, "[INSERT] Se encontro un segmento asociado a la tabla '%s' iniciada con la posicion '%d'", nombreTabla, nroTablaCreada);
			//EXISTE EL SEGMENTO, SOLO CREO LA TABLA Y LA PAGINA
			nroTablaCreada = tabla_pagina_crear(keyBuscada, timestamp, valorAPoner, true);
			log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina, el nro de su posicion es '%d'", nroTablaCreada);
			segmentoAgregarNroTabla(&segmentoBuscado, nroTablaCreada);
			log_info(log_memoria, "[INSERT] Se ha actualizado segmento para la tabla '%s' con la posicion '%d'", nombreTabla, nroTablaCreada);
		}
	} else {
		/*
		 * SE CREO PERFECTAMENTE, CONOSCO EL SEGMENTO Y EL NRO DE TABLA ASOCIADO, PROCEDO A MODIFICAR Y ACCEDER
		 * A LA MEMORIA PARA LA MODIFICACION DE LOS CAMPOS
		 */
		modificarValoresDeTablaYMemoriaAsociadasAKEY(posicionAIr, timestamp, valorAPoner, nroPosicion);
	}
}

void modificarValoresDeTablaYMemoriaAsociadasAKEY(int posAIr, long timestampNuevo, char* valorNuevo, int nroPosicion) {
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	pagina* aux;
	tabla_pagina* tabla_pag;
	memcpy(aux, tabla_paginada+posAIr*(sizeof(pagina)+max_valor_key), sizeof(pagina)+max_valor_key);
	log_info(log_memoria, "[Modificar valor pagina] Para la pagina con key '%d';  TIMESTAMP '%d'; VALOR '%s'",
										aux->key, aux->timestamp, aux->value);

	if(timestampNuevo<=0) {
		aux->timestamp= timestamp();
	} else {
		aux->timestamp = timestampNuevo;
	}
	strcpy(aux->value, valorNuevo);

	log_info(log_memoria, "[Modificar valor pagina] Pagina modificada con key '%d';  TIMESTAMP '%d'; VALOR '%s'",
											aux->key, aux->timestamp, aux->value);

	memcpy(tabla_paginada+posAIr*(sizeof(pagina)+max_valor_key), aux, sizeof(pagina)+max_valor_key);

	memcpy(tabla_pag, tabla_paginada+nroPosicion*sizeof(tabla_pagina), sizeof(tabla_pagina));

	tabla_pag->flag=true;
	memcpy(tabla_paginada+nroPosicion*sizeof(tabla_pagina),tabla_pag, sizeof(tabla_pagina));

	log_info(log_memoria, "[Modificar valor pagina] Se ham modificado el FLAG de la tabla NRO DE PAGINA '%d'", nroPosicion);

	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
}

void segmentoAgregarNroTabla(segmento* segmentoAModificar, int posNueva) {
	//FALTA DESARROLLAR ESTA
//	aniadirNuevaPosicionAArray(&segmentoAModificar, posNueva);
}


/*
void aniadirNuevaPaginaASegmento(pagina* nuevaPag, char* nombreTabla){
	if(tablaSegmentos == NULL) {
		//ESTA VACIA HAY QUE INICIARLA
		tablaSegmentos = segmento_crear(nombreTabla, tabla_pagina_crear(nuevaPag, 0), nuevaPag);
		return;
	}
	segmento* tablaAux = tablaSegmentos;
	//YA HAY SEGMENTOS CREADOS, PROCEDO A BUSCAR EL QUE TIENE EL NOMBRE DE TABLA
	while(tablaAux!= NULL){
		if(strcmp(nombreTabla, obtenerNombreTablaDePath(tablaAux->path_tabla))){
			//SE ENCONTRO LA TABLA, PROCEDO A VER SI EXISTE SU TABLA DE PAGINA
			buscarKeyPorTablaPagina(tablaAux->reg_pagina, nuevaPag, true, false);
			return;
		}
		tablaAux = tablaAux->siguienteSegmento;
	}
	//NO SE ENCONTRO EL SEGMENTO, PROCEDO A CREAR UNA NUEVA Y LA PONGO AL PRINCIPIO
	tablaAux = segmento_crear(nombreTabla, tabla_pagina_crear(nuevaPag, 0), nuevaPag);
	tablaAux->siguienteSegmento= tablaSegmentos;
	tablaSegmentos = tablaAux;
}
*/

void LRU(pagina* paginaCreada){
	/*ALGORITMO, BUSCO LA PAGINA ENTRE TODAS QUE TIENEN MENOR CANTIDAD DE USOS
		LUEGO VERIFICO SI ESTA MODIFICADA O NO
		CASO MODIFICADO: VUELVO A AJECTUTAR LRU SIN TENER EN CUENTA ESE BLOQUE
		CASO NO MODIFICADO: REEMPLAZO
	 */

}


/*
bool chequear_si_memoria_tiene_espacio(int espacioAOcupar){
	return (memoria->tamanioMemoria) > (espacioAOcupar + memoria->);
}
*/

/*
void segmento_agregar_pagina(segmento* seg, tabla_pagina* pag) {
	tabla_pagina* aux = seg->reg_pagina;

	 ESTO HAY QUE CAMBIARLO
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
}*/

/*
//SE PUEDE MODIFICAR ESTO
bool chequear_si_memoria_tiene_espacio(int tamanio) {
	return memoria->tamanioMemoria > tamanio;
}

void pagina_agregar_valores(tabla_pagina* pag, pagina* valor) {
	pag->valor_pagina=valor;
}

//ESTE SE USA PARA AQUELLOS VALORES QUE YA SE LOS IDENTIFICO
void valores_reemplazar_items(pagina* valor_pag, int timestamp, char* valor){
	valor_pag->timestamp=timestamp;
	valor_pag->key=valor;
}

void pagina_poner_estado_modificado(tabla_pagina* pag) {
	pag->flag=true;
}
*/
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
/*
void limpiar_memoria(){
	//SE DEBE USAR UN SEMAFORO AQUI
	limpiandoMemoria=1;
	free(bloque_memoria);
	/*
	while(mem->unidad!=NULL){
		unidadTemporal= mem->unidad->siguienteUnidad;
		limpiarUnidad(&(mem->unidad));

		mem->unidad= unidadTemporal;
	}

	limpiandoMemoria=0;
//	mem->memoriaDisponible = mem->tamanioMemoria;
}
*/

//ESTO SOLO OCURRE CUANDO SE HACE JOURNAL
//O ESO FUE LO QUE HE ENTENDIDO
/*
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

tabla_pagina* buscarPaginaPorNumero(int numeroABuscar, segmento* seg){
	tabla_pagina* paginaAux = seg->reg_pagina;
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
*/

void limpiar_segmento_x_nombre_tabla(char* nombreTabla){
	log_info(log_memoria, "[LIBERAR SEGMENTO] Liberando segmento de la tabla '%s'", nombreTabla);
	segmento* seg_aux;
	while(seg_aux->path_tabla){

	}
	free(buscarSegmentoPorNombreTabla(nombreTabla));

	log_info(log_memoria, "[LIBERAR SEGMENTO] SEGMENTO LIBERADO");
}

void liberar_todo_segmento(){
	segmento* aux;
	log_info(log_memoria, "[LIBERAR SEGMENTO] BLOQUEO EL MUTEX mutex_segmento_en_modificacion PARA QUE NADIE PUEDA AGREGAR MIENTRAS");
	mutexBloquear(&mutex_segmento_en_modificacion);
	log_info(log_memoria, "[LIBERAR SEGMENTO] EMPIEZO A LIBERAR TODOS LOS SEGMENTOS");
	while(tablaSegmentos!=NULL){
		aux = tablaSegmentos->siguienteSegmento;
		log_info(log_memoria, "[LIBERAR SEGMENTO] LIBERANDO SEGMENTO DE TABLA '%s'", tablaSegmentos->path_tabla);
		free(tablaSegmentos);
		tablaSegmentos = aux;
		log_info(log_memoria, "[LIBERAR SEGMENTO] SEGMENTO LIBERADO");
	}

	mutexDesbloquear(&mutex_segmento_en_modificacion);
	log_info(log_memoria, "[LIBERAR SEGMENTO] DESBLOQUEO EL MUTEX mutex_segmento_en_modificacion PARA MODIFICACIONES");
}

void vaciar_tabla_paginas_y_memoria(){
	tabla_pagina* tabla_pag;
	tabla_pag->flag = false;
	tabla_pag->key=-1;
	tabla_pag->posicion=-1;
	pagina* pag;
	pag->key=-1;
	pag->nroPosicion=-1;
	pag->timestamp=-1;
	pag->value="";
	log_info(log_memoria, "[LIBERAR PAGINAS] BLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	int i;
	for(i=0; i<cantPaginasTotales;i++){
		memcpy(tabla_paginada+i*sizeof(tabla_pagina), tabla_pag, sizeof(tabla_pagina));
	}

	log_info(log_memoria, "[LIBERAR PAGINAS] TABLA DE PAGINAS VACIADA");

	for(i=0; i<cantPaginasTotales;i++){
		memcpy(bloque_memoria+i*(sizeof(pagina)+max_valor_key), pag, sizeof(pagina)+max_valor_key);
	}
	log_info(log_memoria, "[LIBERAR PAGINAS] PAGINAS VACIADA");

	log_info(log_memoria, "[LIBERAR PAGINAS] PONGO EL SEMAFORO cantPaginasDisponibles = cantPaginasTotales");
	cantPaginasDisponibles=cantPaginasTotales;
	log_info(log_memoria, "[LIBERAR PAGINAS] DESBLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);


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

String obtenerNombreTablaDePath(String path){
	int i;
	int posUltimoBarra;
	for(i=0; i < stringLongitud(path); i++){
		//ME PONGO A BUSCAR EL ULTIMO /
		if(strcmp(path[i], '/')){
			posUltimoBarra=i+1;
		}
	}
	//YA OBTUVE LA POSICION DE LA ULTIMA BARRA
	int longitudDeNombre = stringLongitud(path) - posUltimoBarra;
	char nombre [longitudDeNombre];
	for(i=0; i < longitudDeNombre; i++) {
		nombre[i] = path[posUltimoBarra+i];
	}
	return nombre;
}

segmento* buscarSegmentoPorNombreTabla(char* nombreTabla){
	segmento* todoSegmento = tablaSegmentos;
	char* nombreEnSegmento;
	log_info(log_memoria, "[BUSCANDO KEY] Buscando el segmento referido para la tabla %s", nombreTabla);
	while(todoSegmento!=NULL){
		nombreEnSegmento = obtenerNombreTablaDePath(todoSegmento->path_tabla);
		if(strcmp(nombreEnSegmento, nombreTabla)){
			//SE ENCONTRO EL SEGMENTO BUSCADO
			log_info(log_memoria, "[BUSCANDO SEGMENTO X NOMBRETABLA] Se encontro el segmento buscado");
			return todoSegmento;
		}
		todoSegmento = todoSegmento->siguienteSegmento;
	}
	log_info(log_memoria, "[BUSCANDO SEGMENTO X KEY] NO se encontro el segmento buscado, devuelvo NULL");
	return NULL;
}

/*
bool buscarKeyEnRegistro(char* nombreTabla, unidad_memoria* unidadAAnalizar, pagina** unidadADevolver, int16_t key){
	log_info(log_memoria, "[BUSCANDO KEY] Buscando 1 segmento");

	segmento* nuevoSegmento = buscarSegmentoPorNumero(unidadAAnalizar->nroSegmento);
//	segmento* nuevoSegmento = buscarSegmentoPorNombreTabla(nombreTabla);
	tabla_pagina* pagAux = nuevoSegmento->reg_pagina;

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
int obtener_valores(char* nombreTabla, int16_t key, unidad_memoria* unidadExtra) {
	pagina* valor_pag;
	memoria_principal* memoriaAuxiliar = memoria;
	log_info(log_memoria, "[BUSCANDO VALORES] Empiezo a buscar el valor con key %d", key);

	//BUSCO PRIMERO REGISTOR POR REGISTRO, EMPEZANDO POR EL MAS ALTO DE LA COLA

	bool encontrado = false;

	/*
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
	*/

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

	return -1;
}
*/
