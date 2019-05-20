
#include "memoria.h"

void terminar_memoria(t_log* g_log);

int main() {

    // LOGGING
	printf("INICIANDO EL MODULO MEMORIA");
	inicioLogYConfig();

//	crearConexionesConOtrosProcesos(); // conecta con LFS y puede que con kernel.
	printf("HACIENDO MEMORIA");
	aux_pagina=malloc(sizeof(pagina));
	aux_segmento=malloc(sizeof(segmento));
	aux_tabla_paginas =malloc(sizeof(pagina_referenciada));
	aux_tabla_paginas2 =malloc(sizeof(pagina_referenciada));
//	aux_tabla_paginas=malloc(sizeof(tabla_pagina));
//    ejecutarHiloConsola();
    armarMemoriaPrincipal();

    iniciarSemaforosYMutex();

    //ESTE ES SOLO TEST, DESPUES SE BORRA A LA MIERDA
    max_valor_key=15;
//    infoPagina* infoPag = malloc(sizeof(infoPag));

    void* informacion = malloc(sizeof(pagina)+max_valor_key);
    int i =0;

    pagina* nuevaPag =malloc(sizeof(pagina));

    char stringValor[max_valor_key];
    strcpy(stringValor, "hola gente");
    for(i=0; i<10; i++){

    	tabla_pagina_crear(100+i, stringValor, false);
    //	sleep(5);

    }
    strcpy(stringValor, "");
    for(i=0; i<10; i++){
    	informacion = accederYObtenerInfoDePaginaEnPosicion(i, informacion);
    	memcpy(stringValor, informacion+sizeof(pagina)-1, max_valor_key);
    	memcpy(nuevaPag, informacion, sizeof(pagina));
    	printf("ALGO %d  %d  %f\n", nuevaPag->nroPosicion, nuevaPag->key, nuevaPag->timestamp);
    	printf("OTRO %s\n\n", stringValor);
    //	printf("TIMESTAMP|NRO|KEY %d|%d|%d\n", nuevaPag->timestamp, nuevaPag->nroPosicion, nuevaPag->key);
    //		printf("VALUE: %s\n\n", stringValor);
    }

    free(nuevaPag);
    printf("TERMINADO");
    free(informacion);

//    ejecutarHiloConsola();
    liberar_todo_por_cierre_de_modulo();
    return 0;
} // MAIN.



void liberar_todo_por_cierre_de_modulo(){

	log_info(log_memoria, "[LIBERAR] Por liberar Segmentos y sus tablas de paginas");
	liberar_todo_segmento();


	//LIBERA LA RAM.
	log_info(log_memoria, "[LIBERAR] Empiezo a liberar todos los elementos que se han inicializado");
	if(aux_pagina!=NULL){
		log_info(log_memoria, "[LIBERAR] Por liberar aux_pagina");
		free(aux_pagina);
		log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
	}

	if(aux_segmento!=NULL){
		log_info(log_memoria, "[LIBERAR] Por liberar aux_segmento");
		free(aux_segmento);
		log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
	}
	if(aux_tabla_paginas!=NULL){
			log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas");
			free(aux_tabla_paginas);
			log_info(log_memoria, "[LIBERAR] aux_tabla_paginas");
		}
	if(aux_tabla_paginas2!=NULL){
		log_info(log_memoria, "[LIBERAR] Por liberar aux_tabla_paginas2");
		free(aux_tabla_paginas2);
		log_info(log_memoria, "[LIBERAR] aux_pagina liberado");
	}

	if(memoriaArmada==1){
		log_info(log_memoria, "[LIBERAR] Por liberar memoria");
		free(bloque_memoria);
		log_info(log_memoria, "[LIBERAR] memoria Liberada");
	}



	log_info(log_memoria, "[LIBERAR] Por liberar Struct configuracion");
	free(arc_config);
	log_info(log_memoria, "[LIBERAR] Struct configuracion Liberada");
	if (log_memoria != NULL) {
		log_info(log_memoria, "[LIBERAR] Liberando log_memoria");
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
	int i = 0;
	log_info(log_memoria, "[ARMAR MEMORIA] Armo el bloque de memoria, guardo su tamaño");


	printf("HACIENDO MEMORIA");

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
	int tamanioMemoria = arc_config->tam_mem;

	//CON ESTO CONOCENERE CUANTO DEBERIA MOVERME POR BYTE PARA LLEGAR A UNA POSICION DE PAGINA

	//AGREGO EL MAX VALUE UNICAMENTE PORQUE EL CHAR* YA CUENTA 1 BYTE, Y EL MAX VALUE EL TAMAÑO REAL
	//EN BYTE QUE TENDRA EL VALOR EN CHAR* POR LO TANTO EL 1 ORIGINAL PASA A REPRESENTAR EL \0 QUE ES 1
	//BYTE MAS
	arc_config->max_value_key = 4;
			//HARDCODEADO PERO BUENO, NO SE QUE LE PASA AL CONFIG DE FILESYSTEM

	int tamanioPagina = sizeof(pagina) + arc_config->max_value_key;
	cantPaginasDisponibles = tamanioMemoria/tamanioPagina;
//memoria->paginasTotales = cantPaginasDisponibles;
	cantPaginasTotales = cantPaginasDisponibles;
	//AQUI SE HAN INICIALIZADO LOS SEMAFOROS DE PAGINAS DISPONIBLES

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de memoria guardada, MEMORIA REAL: %d", tamanioMemoria);

	log_info(log_memoria, "[ARMAR MEMORIA] Tamaño de pagina: %d", tamanioPagina);

	log_info(log_memoria, "[ARMAR MEMORIA] Cantidad maxima de paginas en memoria: %d", cantPaginasDisponibles);

	//log_info(log_memoria, "[ARMAR MEMORIA] Procedo a guardar los datos administrativos de memoria en el bloque de memoria");

	//memcpy(0, sizeof(memoria_principal), memoria);


	imprimirVerde(log_memoria, "[ARMAR MEMORIA] Memoria inicializada de forma correcta");




	printf("MEMORIA TERMINADA");

	//PONGO ESTOS SEMAFOROS LISTOS PARA EMPEZAR A OPERAR
	memoriaArmada = 1;
	limpiandoMemoria = 0;
}

void iniciarSemaforosYMutex() {
	log_info(log_memoria, "[SEMAFOROS] Iniciando semaforos y mutexs");
	mutexIniciar(&memoria_mutex_paginas_disponibles);
	mutexIniciar(&mutex_tabla_pagina_en_modificacion);
	mutexIniciar(&mutex_segmento_en_modificacion);
	mutexIniciar(&mutex_limpiando_memoria);
	mutexIniciar(&mutex_pagina_auxiliar);
	mutexIniciar(&LRUMutex);
	mutexIniciar(&mutex_pagina_referenciada_aux);
	mutexIniciar(&mutex_pagina_referenciada_aux2);
	mutexIniciar(&mutex_crear_pagina_nueva);
	semaforoIniciar(&paginasSinUsar, cantPaginasTotales);

	log_info(log_memoria, "[SEMAFOROS] Semaforos y mutex inicializados");
	log_info(log_memoria, "[SEMAFOROS] Semaforo paginasSinUsar iniciada con valor '%d'",
			cantPaginasTotales);
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
	pthread_join(hiloConsola, NULL);
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
		/*
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

	int i=0;


	//while (i < salir && strcmp(comandosPermitidos[i], comando)) {

		//i++;

	//}

	for (i=0;i <= salir && strcmp(comandosPermitidos[i], comando);i++) {


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

//PROTOTIPO
void pasarValoresALisandra(char* datos){

}
/*-----------------------------------------------------
 * FUNCIONES PARA LA ADMINISTRACION DE MEMORIA
 *-----------------------------------------------------*/

//SE CREA UN SEGMENTO NUEVO CON 1 PAGINA ASOCIADA Y EL NOMBRE DE LA TABLA TAMBIEN
void segmento_crear(char* pathNombreTabla, pagina_referenciada* paginaRef) {
	log_info(log_memoria, "[CREANDO SEGMENTO] Creando segmento para la tabla '%s' asociando al nro de pagina '%d'",
			pathNombreTabla, paginaRef->nropagina);
	segmento* auxSeg = (segmento*)malloc(sizeof(segmento));

	auxSeg->path_tabla = (char*)malloc(strlen(pathNombreTabla)*sizeof(char));
	auxSeg->siguienteSegmento=tablaSegmentos;
	auxSeg->paginasAsocida = paginaRef;
	strcpy(tablaSegmentos->path_tabla, pathNombreTabla);


	if(tablaSegmentos==NULL){
		log_info(log_memoria, "[CREANDO SEGMENTO] ES EL PRIMER SEGMENTO EN SER CREADO '%s'", pathNombreTabla);
		//CREO EL PRIMER SEGMENTO DE LA TABLA
		tablaSegmentos = (segmento*)malloc(sizeof(segmento));
		tablaSegmentos->paginasAsocida = (int*)malloc(sizeof(int));
		tablaSegmentos->path_tabla = (char*)malloc(strlen(pathNombreTabla)*sizeof(char));
		strcpy(tablaSegmentos->path_tabla, pathNombreTabla);

	} else {
		log_info(log_memoria, "[CREANDO SEGMENTO] CREANDO OTRO SEGMENTO A LA TABLA SEGMENTOS '%s'", pathNombreTabla);
		mutexBloquear(&mutex_segmento_modificando);
		tablaSegmentos=auxSeg;
		mutexDesbloquear(&mutex_segmento_modificando);
	}
	log_info(log_memoria, "[CREANDO SEGMENTO]SEGMENTO '%s' CREADO!!!");
}

void segmento_asociar_nueva_pagina(segmento* unSegmento, pagina_referenciada* ref){
	mutexBloquear(&mutex_segmento_modificando);
	unSegmento->cantPaginasAsociadas +=1;
	int cantidad = unSegmento->cantPaginasAsociadas;
	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Por asociar la pagina '%d' al segmento '%s'",
			ref->nropagina, unSegmento->path_tabla);

	mutexBloquear(&mutex_pagina_referenciada_aux);
	aux_tabla_paginas = unSegmento->paginasAsocida;
	while(aux_tabla_paginas->sig!=NULL){
		//ME PONGO A BUSCAR LA ULTIMA POSICION
		aux_tabla_paginas = aux_tabla_paginas->sig;
	}
	aux_tabla_paginas->sig = ref;
	log_info(log_memoria, "[ASOCIANDO NUEVA PAGINA A SEGMENTO] Segmento '%s' se le ha asignado con exito el nro de pagina %d",
				unSegmento->path_tabla, aux_tabla_paginas->nropagina);
	mutexDesbloquear(&mutex_pagina_referenciada_aux);
	mutexDesbloquear(&mutex_segmento_modificando);
}

void segmento_eliminar_nro_pagina(segmento* unSegmento, int nroAQuitar){

	log_info(log_memoria, "[ELIMINANDO PAGINA A SEGMENTO] Sacando nro de pagina '%d' del segmento '%s'",
			nroAQuitar, unSegmento->path_tabla);

	mutexBloquear(&mutex_segmento_aux);
	mutexBloquear(&mutex_segmento_modificando);
	//BUSCO NRO DE PAGINA->LA ELMINO->HAGO REALLOC DEL ARRAY
	aux_segmento = unSegmento;
	while(aux_segmento->paginasAsocida->nropagina != nroAQuitar){
		aux_tabla_paginas = aux_segmento->paginasAsocida;
		aux_segmento->paginasAsocida = aux_segmento->paginasAsocida->sig;
	}

	aux_tabla_paginas->sig = aux_segmento->paginasAsocida;
	free(aux_segmento->paginasAsocida);

	log_info(log_memoria, "[ELIMINANDO PAGINA A SEGMENTO] NRO de pagina '%d' eliminada del segmento '%s'",
			nroAQuitar, unSegmento->path_tabla);
	mutexDesbloquear(&mutex_segmento_modificando);
	mutexBloquear(&mutex_segmento_aux);
}

pagina_a_devolver* segmentoBuscarInfoEnTablaDePagina(char* nombreTabla,
	uint16_t key, bool comandoInsert){

	log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] Por buscar KEY '%d' y PATH '%s'",
			key, nombreTabla);
	segmento* aux = tablaSegmentos;
	while(aux!=NULL){
		if(strcmp(nombreTabla, obtenerNombreTablaDePath(aux->path_tabla))){
			int nroDePagina;
			log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', voy a buscar la key",
					nombreTabla);
			nroDePagina = buscarEntreTodasLasTablaPaginasLaKey(aux->paginasAsocida, key);
			if(nroDePagina>ERROR){
				log_info(log_memoria, "[SEGMENTO BUSCAR TABLA Y KEY] SEGMENTO ENCONTRADO de '%s', KEY encontrada '%d'",
						nombreTabla, key);
				if(comandoInsert){
					//SE DEBE REALIZAR MODIFICACIONES
		//			funcionInsert(nombreTabla, 0, key, valorAPoner);
					return NULL;
				} else {
					//SOLO SE DEBE CONSULTAR LOS DATOS
					return selectObtenerDatos(nroDePagina);
				}
			}

		}
	}
	return NULL;

}

pagina_a_devolver* selectObtenerDatos(int nroDePaginaAIr){
	pagina_a_devolver* pag = malloc(sizeof(pagina_a_devolver));
	pag->value = malloc(sizeof(char)*max_valor_key);
	void* informacion = malloc(sizeof(pagina)+max_valor_key);
	informacion = accederYObtenerInfoDePaginaEnPosicion(nroDePaginaAIr, informacion);
	memcpy(pag->value, informacion+sizeof(pagina)-1, max_valor_key);
	mutexBloquear(&mutex_pagina_auxiliar);
	memcpy(aux_pagina, informacion, sizeof(pagina));
	pag->key=aux_pagina->key;
	pag->timestamp=aux_pagina->timestamp;
	mutexDesbloquear(&mutex_pagina_auxiliar);
	free(informacion);
	return pag;
}

//ESTE, CADA VEZ QUE SE LO INVOCA SE DEBE PONER UN SEMAFORO
void* obtenerInfoDePagina(int i, void* informacion){
	memcpy(informacion, bloque_memoria+i*(sizeof(pagina)+max_valor_key), sizeof(pagina)+max_valor_key);
	return informacion;
}

void* accederYObtenerInfoDePaginaEnPosicion(int posicion, void* info){
	log_info(log_memoria, "[ACCEDIENDO A DATOS] Por acceder a la memoria a la posicion '%d'", posicion);
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	info = obtenerInfoDePagina(posicion, info);
	log_info(log_memoria, "[ACCEDIENDO A DATOS] Datos obtenidos de la posicion '%d'", posicion);
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);

	return info;
}



//CUANDO SE CREA UNA NUEVA PAGINA TAMBIEN SE CREA SU PAGINA Y SE ALMACENA
//EN BLOQUE MEMORIA MIENTRAS LA NUEVA TABLA DE PAGINA EN BLOQUE TABLA PAGINA
//ESTO EN EL CASO DE 1 INSERT O SELECT NUEVO
//
//ESTA FUNCION ES LLAMADA SOLO CUANDO SE TIENE QUE CREAR UNA NUEVA PAGINA
//DEVUELVE EL NRO DONDE SE ALOJO LA NUEVA TABLA
//int * tabla_pagina_crear(int16_t key, long timestampNuevo, char* valor, bool flag_modificado)

int tabla_pagina_crear(int16_t key, char* valor, bool flag_modificado) {
	char valor_string[max_valor_key];
	log_info(log_memoria, "[Crear Tabla y pagina] En crear Tabla de pagina y pagina nueva porque no estan con la key %d", key);

	mutexBloquear(&mutex_pagina_auxiliar);
	aux_tabla_paginas->flag=flag_modificado;
	aux_tabla_paginas->key=key;
	aux_tabla_paginas->vecesAccedido=0;
	aux_tabla_paginas->sig=NULL;
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	int posicionLibreAsignada = cantPaginasTotales - cantPaginasDisponibles;
	log_info(log_memoria, "[Crear Tabla y pagina] La posicion a la cual le voy a asignar es %d", posicionLibreAsignada);
	log_info(log_memoria, "[Crear Tabla y pagina] Tabla pagina creada y procedo a crear la pagina misma");

	aux_pagina = crear_pagina(key, valor);

	strcpy(valor_string, valor);
	log_info(log_memoria, "[Crear Tabla y pagina] Pagina creada con: timestamp '%f'; KEY '%d'; y VALOR: '%s'", aux_pagina->timestamp, key, valor_string);
	//MIENTRAS EST{E BLOQUEADO, NO SE PUEDE QUITAR O PONER NUEVAS PAGINAS
	log_info(log_memoria, "[Crear Tabla y pagina] Verifico si hay espacio libre en Memoria");
	if(cantPaginasDisponibles<=0){
		log_info(log_memoria, "[Crear Tabla y pagina] NO hay espacio libre por lo tanto activo el LRU");
		//SIGNIFICA QUE LLEGO AL TOPE DE PAGINAS EN MEMORIA
		//O SEA ESTAN TODAS OCUPADAS, APLICO LRU
		mutexBloquear(&LRUMutex);
		LRU(aux_pagina);
		mutexDesbloquear(&LRUMutex);
	} else {
		log_info(log_memoria, "[Crear Tabla y pagina] Existen '%d' espacios LIBRES", cantPaginasDisponibles);
		//AUN HAY ESPACIO, GAURDO ESTA NUEVA PAGINA EN ALGUNA POSICION LIBRE
		aux_tabla_paginas->nropagina=posicionLibreAsignada;
		log_info(log_memoria, "[Crear Tabla y pagina] Asigno el espacio libre a la tabla pagina y a la pagina");
		aux_pagina = actualizarPosicionAPagina(aux_pagina, posicionLibreAsignada);
		log_info(log_memoria, "[Crear Tabla y pagina] Procedo a asignar la tabla y la pagina a dicha posicion");
		asignarNuevaTablaAPosicionLibre(aux_tabla_paginas, posicionLibreAsignada, aux_pagina, valor_string);
		log_info(log_memoria, "[Crear Tabla y pagina] ASIGNACION COMPLETADA");
	}
	log_info(log_memoria, "[Crear Tabla y pagina] Desactivo el mutex mutex_tabla_pagina_en_modificacion");



	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
	mutexDesbloquear(&mutex_pagina_auxiliar);
	return posicionLibreAsignada;
}

pagina* actualizarPosicionAPagina(pagina* unaPagina, int nuevaPos){
	unaPagina->nroPosicion= (short)nuevaPos;
	return unaPagina;
}

void asignarNuevaTablaAPosicionLibre(pagina_referenciada* tabla_pagina_nueva, int posLibre, pagina* pagina_nueva, char valor_a_poner[max_valor_key]){
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Por guardar los datos de el valor y la nueva pagina en memoria");


	memcpy(bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key), pagina_nueva, sizeof(pagina));
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Pagina guardada");

	memcpy(bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1,valor_a_poner, max_valor_key);
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] El valor de la pagina fue guardada");
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Decremento el semaforo de cantPaginasDisponibles en 1 unidad");

	cantPaginasDisponibles -= 1;
	log_info(log_memoria, "[asignarNuevaTablaAPosicionLibre] Se han guardado exitosamente los datos\nAhora mismo hay '%d' espacios libres", cantPaginasDisponibles);
	pagina* pagNew = malloc(sizeof(pagina));
	char valorString[max_valor_key];
	memcpy(pagNew, bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key), sizeof(pagina));
	memcpy(valorString, bloque_memoria+posLibre*(sizeof(pagina)+max_valor_key)+sizeof(pagina)-1, max_valor_key);

	printf("VALORES: POSICION|KEY|TIMESTAMP  %d|%d|%f\n\n", pagNew->nroPosicion, pagNew->key, pagNew->timestamp);
	printf("VALUE: %s\n\n", valorString);
	printf("VALOR ORIGINAL %s \nTIMESTAMP: %f\n\n", valor_a_poner, pagina_nueva->timestamp);
	free(pagNew);
}

pagina* crear_pagina(int16_t key, char * valor) {
	log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Por crear pagina y valor");
	aux_pagina->nroPosicion = 0;
	aux_pagina->key = key;
//	aux_pagina->timestamp = timestamp();
	//SI TIENE TIMESTAMP EN 0 LE ASIGNAMOS EL ACTUAL
	double algo =timestamp();
	aux_pagina->timestamp = algo;

	log_info(log_memoria, "[CREANDO PAGINA Y VALOR]KEY|VALOR|TIMESTAMP: %d|%f|%s", key, algo, valor);
	log_info(log_memoria, "[CREANDO PAGINA Y VALOR] Pagina creada y tambien su valor");
	return aux_pagina;
}

//void buscarKeyPorTablaPagina(tabla_pagina* tabla_pag, pagina* pag, bool aniadirSiNoHay, bool modificar) {
int buscarKeyPorTablaPagina(pagina_referenciada* tabla_pagina_auxx, int16_t keyBuscada) {
	mutexBloquear(&mutex_pagina_referenciada_aux2);
//	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Por buscar la KEY '%d' en la pagina '%d'",
//			posicionABuscar, keyBuscada);
	aux_tabla_paginas2 = tabla_pagina_auxx;
	while(aux_tabla_paginas2!=NULL){
		if(aux_tabla_paginas2->key==keyBuscada){
			log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] SE ENCONTRO LA PAGINA PARA LA KEY EN LA POSICION PARA LA KEY|POSICION %d|%d",
						keyBuscada, aux_tabla_paginas2->nropagina);
			mutexDesbloquear(&mutex_pagina_referenciada_aux2);
			return aux_tabla_paginas2->nropagina;
		}
		aux_tabla_paginas2 = aux_tabla_paginas2->sig;
	}

	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] NO SE ENCONTRO LA PAGINA PARA LA KEY EN LA POSICION PARA LA KEY%d",
			keyBuscada);

	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
	return -1;
}

//INTENTA BUSCAR EN QUE POSICION DE LA MEMORIA CONTIGUA SE ENCUNETRA LA PAGINA CON ESA KEY
int buscarEntreTodasLasTablaPaginasLaKey(pagina_referenciada* tablasAsociadasASegmento,	int16_t keyBuscada){
	mutexBloquear(&mutex_pagina_referenciada_aux2);
	//	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] Por buscar la KEY '%d' en la pagina '%d'",
	//			posicionABuscar, keyBuscada);
	aux_tabla_paginas2 = tablasAsociadasASegmento;
	while(aux_tabla_paginas2!=NULL){
		if(aux_tabla_paginas2->key==keyBuscada){
			log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] SE ENCONTRO LA PAGINA PARA LA KEY EN LA POSICION PARA LA KEY|POSICION %d|%d",
				keyBuscada, aux_tabla_paginas2->nropagina);
			mutexDesbloquear(&mutex_pagina_referenciada_aux2);
			return aux_tabla_paginas2->nropagina;
		}
		aux_tabla_paginas2 = aux_tabla_paginas2->sig;
	}
	log_info(log_memoria, "[Buscar Key Por Tabla PAGINA] NO SE ENCONTRO LA PAGINA PARA LA KEY EN LA POSICION PARA LA KEY%d",
			keyBuscada);

	mutexDesbloquear(&mutex_pagina_referenciada_aux2);
	return -1;
}

//Trata de buscar entre todos los segmentos la key buscada a partir de la key  y nombre tabla
int buscarEntreLosSegmentosLaPosicionXNombreTablaYKey(char* nombreTabla, int16_t keyBuscada, segmento** segmentoBuscado, int* nroDePagina){
	log_info(log_memoria, "[Buscar Key] Empiezo a buscar la key '%d' perteneciente a la tabla '%s'", keyBuscada, nombreTabla);
	segmento* seg_aux = buscarSegmentoPorNombreTabla(nombreTabla);
	segmentoBuscado = seg_aux;
	if(seg_aux!=NULL){
		//EXISTE EL SEGMENTO
		log_info(log_memoria, "[Buscar Key] Se encontro el segmento con nombre '%s'", nombreTabla);
		return buscarEntreTodasLasTablaPaginasLaKey(seg_aux->paginasAsocida, keyBuscada);
	}
	//NO EXISTE EL SEGMENTO
	log_info(log_memoria, "[Buscar Key] NO EXISTE NINGUN SEGMENTO ASOCIADA A '%s'; Devuelvo ERROR", nombreTabla);
	return ERROR;
}

void funcionInsert(char* nombreTabla, int16_t keyBuscada, char* valorAPoner){
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

			nroTablaCreada = tabla_pagina_crear(keyBuscada, valorAPoner, true);
			log_info(log_memoria, "[INSERT] Se ha creado una tabla de pagina, el nro de su posicion es '%d'", nroTablaCreada);

			segmento_crear(nombreTabla, nroTablaCreada);
			log_info(log_memoria, "[INSERT] Se ha creado un segmento para la tabla '%s'", nombreTabla);
		} else {
			log_info(log_memoria, "[INSERT] Se encontro un segmento asociado a la tabla '%s' iniciada con la posicion '%d'", nombreTabla, nroTablaCreada);
			//EXISTE EL SEGMENTO, SOLO CREO LA TABLA Y LA PAGINA
			nroTablaCreada = tabla_pagina_crear(keyBuscada, valorAPoner, true);
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

void modificarValoresDeTablaYMemoriaAsociadasAKEY(int posAIr, char* valorNuevo, int nroPosicion) {
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	pagina* aux;
	char value_de_pagina [max_valor_key];
	memcpy(aux, bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key), sizeof(pagina));
	memcpy(value_de_pagina, bloque_memoria+posAIr*(sizeof(pagina)+max_valor_key)+sizeof(pagina), max_valor_key);
	log_info(log_memoria, "[Modificar valor pagina] Para la pagina con key '%d';  TIMESTAMP '%d'; VALOR '%s'",
										aux->key, aux->timestamp, value_de_pagina);

	aux->timestamp= timestamp();

	strcpy(value_de_pagina, valorNuevo);

	log_info(log_memoria, "[Modificar valor pagina] Pagina modificada con key '%d' VALORES NUEVOS;  TIMESTAMP '%d'; VALOR '%s'",
											aux->key, aux->timestamp, value_de_pagina);

	log_info(log_memoria, "[MOdificar valor pagina] Guardando los datos actualizados la pagina con key: %d", aux->key);
	memcpy(bloque_memoria+posAIr*(sizeof(aux)+max_valor_key), aux, sizeof(aux));
	log_info(log_memoria, "[MOdificar valor pagina] key: '%d', VALOR NUEVO: %s", aux->key, value_de_pagina);


	log_info(log_memoria, "[MOdificar valor tabla pagina] Actualizar FLAG de tabla pagina asociada a la key: %d", aux->key);
	actualizarFlagDeLaKey(aux->key);

	log_info(log_memoria, "[MOdificar valor tabla pagina] FLAG ACTUALIZADO EN MODIFICADO PARA LA TABLA DE LA KEY|NRO DE PAGINA: %d|%d", aux->key, nroPosicion);

	log_info(log_memoria, "[Modificar valor pagina] Se ham modificado el FLAG de la tabla KEY|NRO DE PAGINA: %d|%d", aux->key,nroPosicion);

	log_info(log_memoria, "[Modificar valor pagina] Desbloqueo el MUTEX mutex_tabla_pagina_en_modificacion");
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);
}

void actualizarFlagDeLaKey(u_int16_t keyBuscada){
	log_info(log_memoria, "[Modificar valor tabla pagina] Procedo a buscar Segmento y Tabla de pagina que contiene a key: %d", keyBuscada);

	segmento* nuevoSeg = tablaSegmentos;
	pagina_referenciada* ref;

	while(nuevoSeg!=NULL){
		//PROCEDO A BUSCAR LA TABLA DE PAGINA CON ESA KEY
		ref =nuevoSeg->paginasAsocida;
		while(ref!=NULL){
			//BUSCO EN LAS PAGS ASOCIADAS
			if(ref->key==keyBuscada){
				log_info(log_memoria, "[Modificar valor tabla pagina] Key '%d' encontrada en el segmento y nro de pagina '%s'|'%d'",
						keyBuscada, nuevoSeg->path_tabla, ref->nropagina);
				ref->flag=true;
				return;
			}
			ref = ref->sig;
		}
		nuevoSeg = nuevoSeg->siguienteSegmento;
	}
}


void segmentoAgregarNroTabla(segmento* segmentoAModificar, int posNueva) {
	//FALTA DESARROLLAR ESTA
//	aniadirNuevaPosicionAArray(&segmentoAModificar, posNueva);
}



void LRU(pagina* paginaCreada){
	/*ALGORITMO, BUSCO LA PAGINA ENTRE TODAS QUE TIENEN MENOR CANTIDAD DE USOS
		LUEGO VERIFICO SI ESTA MODIFICADA O NO
		CASO MODIFICADO: VUELVO A AJECTUTAR LRU SIN TENER EN CUENTA ESE BLOQUE
		CASO NO MODIFICADO: REEMPLAZO
	 */

}


void limpiar_segmento_x_nombre_tabla(char* nombreTabla){
	//ESTO SE DEBE REVISAR, TIENE ERRORES
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

void liberar_toda_tabla_paginas(pagina_referenciada* pag){
	pagina_referenciada* aux;
	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
	while(pag!=NULL){
		aux = pag->sig;
		log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] Libero pagina nro '%d'", pag->nropagina);
		free(pag);
		pag = aux;
	}
	log_info(log_memoria, "[LIBERAR TABLA DE PAGINAS] TERMINADO");
}

void vaciar_tabla_paginas_y_memoria(){
	pagina* pag;
	pag->key=-1;
	pag->nroPosicion=-1;
	pag->timestamp=-1;
	char valor_a_nulo [max_valor_key];
	log_info(log_memoria, "[LIBERAR PAGINAS] BLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexBloquear(&mutex_tabla_pagina_en_modificacion);
	int i;


	for(i=0; i<cantPaginasTotales;i++){
			memcpy(bloque_memoria+i*(sizeof(pagina)+max_valor_key), pag, sizeof(pagina)+max_valor_key);
		}
		log_info(log_memoria, "[LIBERAR PAGINAS] PAGINAS VACIADA");

	//EMPIEZA A BORRAR DATOS, QUIZAS DEBAMOS PONER 1 SEMAFORO MUTEX PARA INDICAR QUE YA SE BORRARON LAS PAGINAS

	log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Por liberar la tabla de paginas de todo segmento");

	segmento* segaux = tablaSegmentos;
	while(segaux!=NULL){
		pagina_referenciada* ref = segaux->paginasAsocida;
		pagina_referenciada* otro;
		log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Liberando tablas de paginas del segmento '%s'", segaux->path_tabla);
		while(ref!=NULL){
			otro = ref->sig;
			log_info(log_memoria, "[LIBERANDO TABLA DE PAGINAS] Pagina nro '%d' con key '%d' LIBERADO", ref->nropagina, ref->key);
			free(ref);
			ref = otro;
		}
	}

	log_info(log_memoria, "[LIBERAR PAGINAS] TABLA DE PAGINAS VACIADA");

	log_info(log_memoria, "[LIBERAR PAGINAS] PONGO EL SEMAFORO cantPaginasDisponibles = cantPaginasTotales");
	cantPaginasDisponibles=cantPaginasTotales;
	log_info(log_memoria, "[LIBERAR PAGINAS] DESBLOQUEO EL MUTEX mutex_tabla_pagina_en_modificacion");
	mutexDesbloquear(&mutex_tabla_pagina_en_modificacion);


}

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
