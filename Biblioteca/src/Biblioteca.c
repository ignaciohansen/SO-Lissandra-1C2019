/*
 ============================================================================
 Name        : BibliotecaGeneral.c
 Author      : mi_ultimo_segundo_tp
 Version     : 1.0
 Copyright   : Todos los derechos reservados papu
 Description : Biblioteca general para dummies
 ============================================================================
 */

#include "Biblioteca.h"
#include <arpa/inet.h>

/*
 * FUNCIONES PARA ABORTAR UN PROCESO
 */




void abortarProcesoPorUnErrorImportante(t_log* log, char* mensaje){
	imprimirError1(log, "ERROR: %s", mensaje);
	abort();
}

//--------------------------------------- Funcion timestamp -------------------------------------
/*
double  timestamp(void) {
	struct timeval t;
	gettimeofday(&t, NULL);
	unsigned long long result = (((unsigned long long)t.tv_sec)*1000+((unsigned long long)t.tv_usec)/1000);
	double a = result;
	return a;
//	return (unsigned)time(NULL);
}
*/

timestamp_t timestamp(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	timestamp_t result = (((unsigned long long)t.tv_sec)*1000+((unsigned long long)t.tv_usec)/1000);
	return result;
}
//--------------------------------------- Funciones para Socket -------------------------------------

void socketConfigurar(Conexion* conexion, String ip, String puerto,t_log* logger) {
	
	struct addrinfo hints;
	
	log_info(logger,"1");

	memset(&hints, NULO, sizeof(hints));
	
	log_info(logger,"2");

	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	
	log_info(logger,"3");
	
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	
	log_info(logger,"4");

	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	log_info(logger,"5");

	getaddrinfo(ip, puerto, &hints, &conexion->informacion); // Carga en serverInfo los datos de la conexion
	
}

Socket socketCrear(Conexion* conexion, String ip, String puerto, t_log* logger) {
	log_info(logger,"Por llamar a socketConfigurar");
	socketConfigurar(conexion, ip, puerto,logger);
	log_info(logger,"Despues de socketConfigurar");
	Socket unSocket = socket(conexion->informacion->ai_family, conexion->informacion->ai_socktype, conexion->informacion->ai_protocol);
	socketError(unSocket, "socket");
	return unSocket;
}

Socket nuevoSocket(t_log* logger){

	log_info(logger,"En funcion nuevoSocket en el Proyecto Biblioteca, por llamar a funcion Socket()");

	int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	
	if (fileDescriptor == ERROR){

		log_error(logger,"Socket nos devolvio -1, no se pudo crear");

		return fileDescriptor;		
	} 

	log_info(logger,"Por retornar el socket creado: %d", fileDescriptor);

	return fileDescriptor;
}

int conectarSocket(int fd_socket, const char* ipDestino, int puerto,t_log* logger){
	
	struct sockaddr_in direccionServidor;

	direccionServidor.sin_family = AF_INET;

	direccionServidor.sin_port = htons(puerto);

	direccionServidor.sin_addr.s_addr = inet_addr(ipDestino);

	memset(&(direccionServidor.sin_zero), '\0', 8);

	int resultadoConnect = connect(fd_socket, (struct sockaddr *) &direccionServidor, sizeof(struct sockaddr));

	if (resultadoConnect == ERROR) {
		
		log_info(logger,"[SOCKETS] No se pudo realizar la conexión entre el socket y el servidor.");
		
		return ERROR;

	} 

	log_info(logger, "El resultado de connect es: %d",resultadoConnect);
	
	return resultadoConnect;
	
}

struct sockaddr_in asociarSocket(int fd_socket, int puerto,t_log* logger){
	
	log_info(logger,"En funcion asociarSocket()");
	
	miDireccionSocket.sin_family = AF_INET;
	miDireccionSocket.sin_port = htons(puerto);
	miDireccionSocket.sin_addr.s_addr = 0; // Con htons(INADDR_ANY) (o bien, 0) usa la dirección IP de la máquina actual
	memset(&(miDireccionSocket.sin_zero), '\0', 8); // Rellena con ceros el resto de la estructura

// Si el puerto ya está siendo utilizado, lanzamos un error
	int enUso = 1;
	int puertoYaAsociado = setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &enUso, sizeof(enUso));
	if (puertoYaAsociado == ERROR) printf("[SOCKETS] El puerto a asociar ya está siendo utilizado.\n");

// Ya comprobado el error de puerto, llamamos a bind
	int retornoBind = bind(fd_socket, (struct sockaddr *) &miDireccionSocket, sizeof(struct sockaddr));
	if (retornoBind == ERROR) printf("[SOCKETS] No se pudo asociar el socket al puerto.\n");

	return miDireccionSocket;
}

void socketConectar(Conexion* conexion, Socket unSocket) {
	int estado = connect(unSocket, conexion->informacion->ai_addr, conexion->informacion->ai_addrlen);
	socketError(estado, "connect");
	freeaddrinfo(conexion->informacion);
}

void socketBindear(Conexion* conexion, Socket unSocket) {
	int estado = bind(unSocket, conexion->informacion->ai_addr, conexion->informacion->ai_addrlen);
	socketError(estado, "bind");
	freeaddrinfo(conexion->informacion);
}

void socketEscuchar(Socket unSocket, int clientesEsperando,t_log* logger) {
	
	log_info(logger,"En funcion socketEscuchar");
	
	int estado = listen(unSocket, clientesEsperando);
	
	if(estado == ERROR){

		imprimirError(logger, "Error al poner el Socket en escucha");

		return;
	}
	imprimirVerde(logger, "[  OK  ] Given socket is now listening.");
}

int aceptarConexionSocket(int fd_socket,t_log* logger) {

	log_info(logger,"[BIBLIO] (+) FUNCTION aceptarConexionSocket"); // BEGIN
	// printf("En funcion aceptarConexion \n");
	int addres_size = sizeof(struct sockaddr_in);

	struct sockaddr_storage unCliente; // sino: struct sockaddr_in unCliente;

	//unsigned int addres_size = sizeof(unCliente);

	log_info(logger,"Por llamar a accept");

	int fdCliente = accept(fd_socket, (struct sockaddr*) &unCliente, &addres_size);

	if(fdCliente == ERROR) {
		log_error(logger,"[SOCKETS] No se pudo aceptar la conexión entrante.");
	} else {
		log_info(logger,"Se acepto la conexion %d", fdCliente);
	} // int fdCliente

	log_info(logger,"[BIBLIO] (-) FUNCTION aceptarConexionSocket"); // BEGIN
	return fdCliente;
}

Socket socketAceptar(Socket unSocket, int idEsperada,t_log* logger) {
	Conexion conexion;
	conexion.tamanioAddress = sizeof(SockAddrIn);
	Socket nuevoSocket = accept(unSocket, (SockAddr)&conexion.address, &conexion.tamanioAddress);
	if(nuevoSocket != ERROR)
		if(handShakeRecepcionFallida(nuevoSocket, idEsperada,logger))
			handShakeError(nuevoSocket);
	return nuevoSocket;
}

void socketRedireccionar(Socket unSocket) {
	int yes = 1;
	int estado = setsockopt(unSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	socketError(estado, "setsockopt");
}

void socketSelect(int cantidadSockets, ListaSockets* listaSockets, int retry) {
	int estado = select(cantidadSockets + 1, listaSockets, NULL, NULL, NULL);
	if(estado==-1){
		if(retry>10){
			printf("select no pudo regenerarse");
			abort();
		}
		if(errno==EINTR)
			socketSelect(cantidadSockets,listaSockets,retry+1);
		else
			socketError(estado, "select");
	}
}

int socketRecibir(Socket socketEmisor, Puntero buffer, int tamanioBuffer,t_log* logger) {
	
	log_info(logger,"[socketRecibir] (+)");

	// MSG_WAITALL - Obliga a recibir "tamanioBuffer" Bytes
	// 0           - no obliga a esperar la recepción de todos los bytes
	// int estado = recv(socketEmisor, buffer, tamanioBuffer, MSG_WAITALL);
	int estado = recv(socketEmisor, buffer, tamanioBuffer, 0);

	// if(estado == ERROR){
	if( estado <= 0 ) {
		perror("recv");
		log_info(logger,"[ERROR] Cliente/Memoria desconectado.");
	}
	log_info(logger,"[socketRecibir] (-)");
	return estado;
}

int socketEnviar(Socket socketReceptor, Puntero mensaje, int tamanioMensaje,t_log* logger) {

	log_info(logger,"En funcion socketEnviar %d, %s, %d,",socketReceptor,mensaje,tamanioMensaje);

	int estado = send(socketReceptor, mensaje, tamanioMensaje, NULO);

	if(estado == ERROR)
		perror("send");
	return estado;
}

void socketCerrar(Socket unSocket) {
	close(unSocket);
}

bool socketSonIguales(Socket unSocket, Socket otroSocket) {
	return unSocket == otroSocket;
}

bool socketSonDistintos(Socket unSocket, Socket otroSocket) {
	return unSocket != otroSocket;
}

bool socketEsMayor(Socket unSocket, Socket otroSocket) {
	return unSocket > otroSocket;
}

void socketError(int estado, String error) {
	if(estado == ERROR) {
		perror(error);
		exit(EXIT_FAILURE);
	}
}

Socket socketCrearListener(String ip, String puerto, t_log* logger) {
	Conexion conexion;
	conexion.tamanioAddress = sizeof(conexion.address);
	Socket listener = socketCrear(&conexion, ip, puerto, logger);
	socketRedireccionar(listener);
	socketBindear(&conexion, listener);
	socketEscuchar(listener, LISTEN, logger);
	return listener;
}

Socket socketCrearCliente(String ip, String puerto, int idProceso, t_log* logger) {
	Conexion conexion;
	Socket unSocket = socketCrear(&conexion, ip, puerto, logger);
	socketConectar(&conexion, unSocket);
	if(handShakeEnvioFallido(unSocket, idProceso, logger))
		handShakeError(unSocket);
	return unSocket;
}

//--------------------------------------- Funciones para ListaSockets-------------------------------------

void listaSocketsAgregar(Socket unSocket, ListaSockets* listaSockets) {
	FD_SET(unSocket, listaSockets);
}

void listaSocketsEliminar(Socket unSocket, ListaSockets* listaSockets) {
	FD_CLR(unSocket, listaSockets);
}

bool listaSocketsContiene(Socket unSocket, ListaSockets* listaSockets) {
	return FD_ISSET(unSocket, listaSockets);
}

void listaSocketsLimpiar(ListaSockets* listaSockets) {
	FD_ZERO(listaSockets);
}

//--------------------------------------- Funciones para Mensaje -------------------------------------

void* mensajeCrear(int32_t operacion, void* dato, int32_t tamanioDato){
	Header header = headerCrear(operacion, tamanioDato);
	void* buffer = malloc(sizeof(Header)+tamanioDato);
	memcpy(buffer, &header, sizeof(Header));
	memcpy(buffer+sizeof(Header), dato, tamanioDato);
	return buffer;
}

int mensajeEnviar(int socketReceptor, int32_t operacion, void* dato, int32_t tamanioDato,t_log* logger) {
	void* buffer = mensajeCrear(operacion, dato, tamanioDato);
	int resultado = socketEnviar(socketReceptor, buffer, sizeof(Header)+tamanioDato,logger);
	free(buffer);
	return resultado;
}

void mensajeAvisarDesconexion(Mensaje* mensaje) {
	mensaje->header.operacion = DESCONEXION;
	mensaje->header.tamanio = NULO;
	mensaje->datos = NULL;
}

bool mensajeConexionFinalizada(int bytes) {
	return bytes == NULO || bytes == ERROR;
}

void mensajeRevisarConexion(Mensaje* mensaje, Socket socketReceptor, int bytes,t_log* logger) {
	if(mensajeConexionFinalizada(bytes))
		mensajeAvisarDesconexion(mensaje);
	else
		mensajeObtenerDatos(mensaje, socketReceptor,logger);
}

void mensajeObtenerDatos(Mensaje* mensaje, Socket socketReceptor,t_log* logger) {
	int tamanioDato = mensaje->header.tamanio;
	if(tamanioDato==0) {
		mensaje->datos=NULL;
		return;
	}
	mensaje->datos = malloc(tamanioDato);
	int bytes = socketRecibir(socketReceptor, mensaje->datos, tamanioDato,logger);
	if(mensajeConexionFinalizada(bytes))
		mensajeAvisarDesconexion(mensaje);
}

Mensaje* mensajeRecibir(int socketReceptor,t_log* logger) {
	Mensaje* mensaje = malloc(sizeof(Mensaje));
	int bytes = socketRecibir(socketReceptor, &mensaje->header, sizeof(Header),logger);
	mensajeRevisarConexion(mensaje, socketReceptor, bytes,logger);
	return mensaje;
}

void mensajeDestruir(Mensaje* mensaje) {
	if(mensaje->datos != NULL)
		free(mensaje->datos);
	free(mensaje);
}

bool mensajeOperacionIgualA(Mensaje* mensaje, int operacion) {
	return mensaje->header.operacion == operacion;
}

bool mensajeDesconexion(Mensaje* mensaje) {
	return mensajeOperacionIgualA(mensaje, DESCONEXION);
}

//--------------------------------------- Funciones de HandShake-------------------------------------

int handShakeEnvioExitoso(Socket unSocket, int32_t idProceso,t_log* logger) {
	int32_t id = idProceso;
	mensajeEnviar(unSocket, HANDSHAKE, &id, sizeof(int32_t),logger);
	Mensaje* mensaje = mensajeRecibir(unSocket,logger);
	int estado = handShakeRealizado(mensaje) && handShakeAceptado(mensaje);
	mensajeDestruir(mensaje);
	return estado;
}

int handShakeRecepcionExitosa(Socket unSocket, int idEsperada,t_log* logger) {
	Mensaje* mensaje = mensajeRecibir(unSocket,logger);
	int idProceso = (*(int32_t*)mensaje->datos);
	mensajeDestruir(mensaje);
	int32_t estado = handShakeIdsIguales(idProceso, idEsperada);
	mensajeEnviar(unSocket, HANDSHAKE, &estado, sizeof(int32_t),logger);
	return estado;
}

bool handShakeIdsIguales(int idEnviada, int idEsperada) {
	return idEnviada == idEsperada;
}

bool handShakeRealizado(Mensaje* mensaje) {
	return mensaje->header.operacion != DESCONEXION;
}

bool handShakeAceptado(Mensaje* mensaje) {
	return *((int*)mensaje->datos) == true;
}

int handShakeEnvioFallido(Socket unSocket, int idProceso,t_log* logger) {
	return !handShakeEnvioExitoso(unSocket, idProceso, logger);
}

int handShakeRecepcionFallida(Socket unSocket, int idEsperada,t_log* logger) {
	return !handShakeRecepcionExitosa(unSocket, idEsperada,logger);
}

void handShakeError(Socket unSocket) {
	socketCerrar(unSocket);
	imprimirMensajeProceso("[ERROR] No se pueden realizar conexiones con este proceso en este puerto");
	exit(EXIT_FAILURE);
}

//--------------------------------------- Funciones para Header -------------------------------------

Header headerCrear(int32_t operacion, int32_t tamanio) {
	Header header;
	header.operacion = operacion;
	header.tamanio = tamanio;
	return header;
}

//--------------------------------------- Funciones para Configuracion -------------------------------------

void* configuracionCrear(String rutaArchivo, void*(*configProcesoCrear)(ArchivoConfig archivoConfiguracion), String* campos) {
	ArchivoConfig archivo = archivoConfigCrear(rutaArchivo, campos);
	return configProcesoCrear(archivo);
}

//--------------------------------------- Funciones para ArchivoConfiguracion -------------------------------------

ArchivoConfig archivoConfigCrear(String path, String* campos) {
	ArchivoConfig archivoConfig = config_create(path);
	if(archivoConfigInvalido(archivoConfig, campos)) {
		puts("Archivo de configuracion invalido");
		exit(EXIT_FAILURE);
	}
	return archivoConfig;
}


bool archivoConfigTieneCampo(ArchivoConfig archivoConfig, String campo) {
	return config_has_property(archivoConfig, campo);
}

bool archivoConfigFaltaCampo(ArchivoConfig archivoConfig, String campo) {
	return !archivoConfigTieneCampo(archivoConfig, campo);
}

String archivoConfigStringDe(ArchivoConfig archivoConfig, String campo) {
	return config_get_string_value(archivoConfig, campo);
}

int archivoConfigEnteroDe(ArchivoConfig archivoConfig, String campo) {
	return config_get_int_value(archivoConfig, campo);
}

long archivoConfigLongDe(ArchivoConfig archivoConfig, String campo) {
	return config_get_long_value(archivoConfig, campo);
}

double archivoConfigDoubleDe(ArchivoConfig archivoConfig, String campo) {
	return config_get_double_value(archivoConfig, campo);
}

String* archivoConfigArrayDe(ArchivoConfig archivoConfig, String campo) {
	return config_get_array_value(archivoConfig, campo);
}

int archivoConfigCantidadCampos(ArchivoConfig archivoConfig) {
	return config_keys_amount(archivoConfig);
}

void archivoConfigDestruir(ArchivoConfig archivoConfig) {
	config_destroy(archivoConfig);
}

void archivoConfigSetearCampo(ArchivoConfig archivoConfig, String campo, String valor) {
	config_set_value(archivoConfig, campo, valor);
}

bool archivoConfigInvalido(ArchivoConfig archivoConfig, String* campos) {
	return archivoConfigIncompleto(archivoConfig, campos) || archivoConfigInexistente(archivoConfig) ;
}

bool archivoConfigInexistente(ArchivoConfig archivoConfig) {
	return archivoConfig == NULL;
}

bool archivoConfigIncompleto(ArchivoConfig archivoConfig, String* campos) {
	int indice;
	for(indice = 0; indice<archivoConfigCantidadCampos(archivoConfig); indice++)
		if(archivoConfigFaltaCampo(archivoConfig, campos[indice]))
			return true;
	return false;
}

//--------------------------------------- Funciones para ArchivoLog -------------------------------------

ArchivoLog archivoLogCrear(String rutaArchivo, String nombrePrograma) {
	archivoLogValidar(rutaArchivo);
	ArchivoLog archivoLog = log_create(rutaArchivo, nombrePrograma, false, 1);
	return archivoLog;
}

void archivoLogDestruir(ArchivoLog archivoLog) {
	log_destroy(archivoLog);
}

void archivoLogInformarMensaje(ArchivoLog archivoLog, String mensaje) {
	log_info(archivoLog, mensaje);
}

void archivoLogInformarAdvertencia(ArchivoLog archivoLog, String mensajePeligro, ...) {
	log_warning(archivoLog, mensajePeligro);
}

void archivoLogInformarError(ArchivoLog archivoLog, String mensajeError){
	log_error(archivoLog, mensajeError);

}

void archivoLogInformarTrace(ArchivoLog archivoLog, String mensajeTrace) {
	log_trace(archivoLog, mensajeTrace);
}

void archivoLogInformarDebug(ArchivoLog archivoLog, String mensajeDebug) {
	log_debug(archivoLog, mensajeDebug);
}

String archivoLogNivelLogAString(NivelLog nivelLog) {
	return log_level_as_string(nivelLog);
}

NivelLog archivoLogStingANivelLog(String stringNivelLog) {
	return log_level_from_string(stringNivelLog);
}

void archivoLogValidar(String rutaArchivo) {
	fileLimpiar(rutaArchivo);
}

//--------------------------------------- Funciones para Semaforo -------------------------------------

void semaforoIniciar(Semaforo* semaforo, unsigned int valor) {
	sem_init(semaforo, NULO, valor);
}

void semaforoWait(Semaforo* semaforo) {
	sem_wait(semaforo);
}

void semaforoSignal(Semaforo* semaforo) {
	sem_post(semaforo);
}

void semaforoDestruir(Semaforo* semaforo) {
	sem_destroy(semaforo);
}

void semaforoValor(Semaforo* semaforo, int* buffer) {
	sem_getvalue(semaforo, buffer);
}

//--------------------------------------- Funciones Mutex -------------------------------------

void mutexIniciar(Mutex* mutex) {
	pthread_mutex_init(mutex, NULL);
}

void mutexBloquear(Mutex* mutex) {
	pthread_mutex_lock(mutex);
}

void mutexDesbloquear(Mutex* mutex) {
	pthread_mutex_unlock(mutex);
}

//--------------------------------------- Funciones RWLock -------------------------------------

void rwLockIniciar(RWlock* rwLock) {
	pthread_rwlock_init(rwLock, NULL);
}

void rwLockLeer(RWlock* rwLock) {
	pthread_rwlock_rdlock(rwLock);
}

void rwLockEscribir(RWlock* rwLock) {
	pthread_rwlock_wrlock(rwLock);
}

void rwLockDesbloquear(RWlock* rwLock) {
	pthread_rwlock_unlock(rwLock);
}

//--------------------------------------- Funciones para Hilo -------------------------------------

void hiloCrear(Hilo* hilo, void*(*funcionHilo)(void*), void* parametroHilo) {
	pthread_create(hilo, NULL, funcionHilo, parametroHilo);
}

void hiloEsperar(Hilo hilo) {
	pthread_join(hilo, NULL);
}

void hiloSalir() {
	pthread_exit(NULL);
}

void hiloCancelar(Hilo hilo) {
	pthread_cancel(hilo);
}

void hiloDetach(Hilo hilo) {
	pthread_detach(hilo);
}

Hilo hiloId() {
	 return pthread_self();
}

//--------------------------------------- Funciones para Lista -------------------------------------

Lista listaCrear() {
	return list_create();
}

void listaDestruir(Lista lista) {
	if(lista != NULL)
		list_destroy(lista);
}

void listaDestruirConElementos(Lista lista, void(*funcion)(void*)) {
	if(lista != NULL)
		list_destroy_and_destroy_elements(lista, funcion);
}

int listaAgregarElemento(Lista lista, void* elemento) {
	return list_add(lista, elemento);
}

int list_addM(Lista lista,void* elemento,size_t tamanio){
	void* ptr=malloc(tamanio);
	memcpy(ptr,elemento,tamanio);
	return list_add(lista,ptr);
}

int listaAgregarElementoM(Lista lista, void* elemento,size_t tamanio) {
	return list_addM(lista, elemento,tamanio);
}

void listaAgregarEnPosicion(Lista lista, void* elemento, int posicion) {
	list_add_in_index(lista, posicion, elemento);
}
void listaAgregarOtraLista(Lista lista, Lista otraLista) {
	list_add_all(lista, otraLista);
}

void* listaObtenerElemento(Lista lista, int posicion) {
	return list_get(lista, posicion);
}

void* listaPrimerElemento(Lista lista) {
	return list_get(lista, 0);
}

Lista listaTomar(Lista lista, int cantidadElementos) {
	return list_take(lista, cantidadElementos);
}

Lista listaSacar(Lista lista, int cantidadElementos) {
	return list_take_and_remove(lista, cantidadElementos);
}

Lista listaFiltrar(Lista lista, bool(*funcion)(void*)) {
	return list_filter(lista, funcion);
}

Lista listaMapear(Lista lista, void*(*funcion)(void*)) {
	return list_map(lista, funcion);
}

void* listaReemplazarElemento(Lista lista, void* elemento, int posicion) {
	return list_replace(lista, posicion, elemento);
}

void listaReemplazarDestruyendoElemento(Lista lista, void* elemento, int posicion, void(*funcion)(void*)) {
	list_replace_and_destroy_element(lista, posicion, elemento, funcion);
}

void listaEliminarElemento(Lista lista, int posicion) {
	list_remove(lista, posicion);
}

void listaEliminarDestruyendoElemento(Lista lista, int posicion, void(*funcion)(void*)) {
	list_remove_and_destroy_element(lista, posicion, funcion);
}

void listaEliminarPorCondicion(Lista lista, bool(*funcion)(void*)) {
	list_remove_by_condition(lista, funcion);
}

void listaEliminarDestruyendoPorCondicion(Lista lista, bool(*funcion)(void*), void(*funcionDestruir)(void*)) {
	list_remove_and_destroy_by_condition(lista, funcion, funcionDestruir);
}

void listaLimpiar(Lista lista) {
	list_clean(lista);
}

void listaLimpiarDestruyendoElementos(Lista lista, void(*funcion)(void*)) {
	list_clean_and_destroy_elements(lista, funcion);
}
void listaIterar(Lista lista, void(*funcion)(void*)) {
	list_iterate(lista, funcion);
}
void* listaBuscar(Lista lista, bool(*funcion)(void*)) {
	return list_find(lista, funcion);
}

int listaCantidadElementos(Lista lista) {
	return list_size(lista);
}

bool listaEstaVacia(Lista lista) {
	return list_is_empty(lista);
}

bool listaTieneElementos(Lista lista) {
	return !list_is_empty(lista);
}

void listaOrdenar(Lista lista, bool(*funcion)(void*, void*)) {
	list_sort(lista, funcion);
}

int listaCuantosCumplen(Lista lista, bool(*funcion)(void*)) {
	return list_count_satisfying(lista, funcion);
}

bool listaCumpleAlguno(Lista lista, bool(*funcion)(void*)) {
	return list_any_satisfy(lista, funcion);
}

bool listaCumplenTodos(Lista lista, bool(*funcion)(void*)) {
	return list_all_satisfy(lista, funcion);
}

//--------------------------------------- Funciones para String -------------------------------------

String stringCrear(int tamanio) {
	String string = malloc(tamanio);
	memset(string, '\0', tamanio);
	return string;
}

bool stringContiene(String unString, String otroString) {
	return string_contains(unString, otroString);
}

String stringConvertirEntero(int entero) {
	return string_itoa(entero);
}

void stringLimpiar(String string, int tamanioString) {
	memset(string, '\0', tamanioString);
}

String stringRepetirCaracter(char caracter, int repeticiones) {
	return string_repeat(caracter, repeticiones);
}

void stringConcatenar(String unString, String otroString) {
	strcat(unString, otroString);
}

String stringDuplicar(String string) {
	return string_duplicate(string);
}

void stringPonerEnMayuscula(String string) {
	string_to_upper(string);
}

void stringPonerEnMinuscula(String string) {
	string_to_lower(string);
}

void stringPonerEnCapital(String string) {
	string_capitalized(string);
}

void stringRemoverVacios(String* string) {
	string_trim(string);
}

void stringRemoverVaciosIzquierda(String* string) {
	string_trim_left(string);
}

void stringRemoverVaciosDerecha(String* string) {
	string_trim_right(string);
}

int stringLongitud(String string) {
	return string_length(string);
}

bool stringEstaVacio(String string) {
	return string_is_empty(string);
}

bool stringEmpiezaCon(String string, String stringComienzo) {
	return string_starts_with(string, stringComienzo);
}

bool stringTerminaCon(String string, String stringTerminacion) {
	return string_ends_with(string, stringTerminacion);
}

String stringDarVuelta(String string) {
	return string_reverse(string);
}

String stringTomarCantidad(String string, int desde, int cantidad) {
	return string_substring(string, desde, cantidad);
}

String stringTomarDesdePosicion(String string, int posicion) {
	return string_substring_from(string, posicion);
}

String stringTomarDesdeInicio(String string, int cantidad) {
	return string_substring_until(string, cantidad);
}


String stringCopiar(String stringReceptor, const String stringACopiar) {
	return strcpy(stringReceptor, stringACopiar);
}

String* stringSeparar(String unString, String separador) {
	return string_split(unString, separador);
}

bool stringNulo(String unString) {
	return unString == NULL;
}

bool stringValido(String unString) {
	return unString != NULL;
}

bool stringIguales(String s1, String s2) {
	if(stringNulo(s1) || stringNulo(s2))
		return false;
	if (strcmp(s1, s2) == NULO)
		return true;
	else
		return false;
}

bool stringDistintos(String unString, String otroString) {
	return !stringIguales(unString, otroString);
}

//--------------------------------------- Funciones de Bitmap -------------------------------------

Bitmap bitmapCrear(int cantidadBloques) {
	int tamanioBytes = bitmapCalculo(cantidadBloques);
	char* bits = malloc(tamanioBytes);
	Bitmap bitmap = bitarray_create_with_mode(bits, tamanioBytes, LSB_FIRST);
	int indice;
	for(indice = 0; indice < tamanioBytes*8; indice++){
		bitarray_clean_bit(bitmap, indice);
	}
	return bitmap;
}

void bitmapDestruir(Bitmap bitmap) {
	memoriaLiberar(bitmap->bitarray);
	bitarray_destroy(bitmap);
}

void bitmapLiberarBit(Bitmap bitmap, int posicion) {
	bitarray_clean_bit(bitmap, posicion);
}

void bitmapOcuparBit(Bitmap bitmap, int posicion) {
	bitarray_set_bit(bitmap, posicion);
}

bool bitmapBitOcupado(Bitmap bitmap, int posicion) {
	return bitarray_test_bit(bitmap, posicion);
}

size_t bitmapCantidadBits(Bitmap bitmap) {
	return bitarray_get_max_bit(bitmap);
}

int bitmapCalculo(int cantidadBloques) {
	return (int)ceil((double)cantidadBloques/(double)8);
}

//--------------------------------------- Funciones de Impresion -------------------------------------

void imprimirMensaje(ArchivoLog archivoLog, String mensaje) {
	/*if(archivoLog->is_active_console){
		puts(mensaje);
	}*/
	log_info(archivoLog, mensaje);
}

void imprimirMensaje1(ArchivoLog archivoLog, String mensaje, void* algo1) {
	/*if(archivoLog->is_active_console){
		printf(mensaje, algo1 );
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1);
}

void imprimirMensaje2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	/*if(archivoLog->is_active_console){
		printf(mensaje, algo1, algo2);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2);
}

void imprimirMensaje3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	/*if(archivoLog->is_active_console){
		printf(mensaje, algo1, algo2, algo3);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirMensaje4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	/*if(archivoLog->is_active_console){
		printf(mensaje, algo1, algo2, algo3, algo4);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirAviso(ArchivoLog archivoLog, String mensaje) {
/*	if(archivoLog->is_active_console){
		printf(AMARILLO);
		puts(mensaje);
		printf(BLANCO);
	}*/
	log_warning(archivoLog, mensaje);
}

void imprimirAviso1(ArchivoLog archivoLog, String mensaje, void* algo1) {
/*	if(archivoLog->is_active_console){
		printf(AMARILLO);
		printf(mensaje, algo1 );
		printf(BLANCO);
		puts("");
	}*/
	log_warning(archivoLog, mensaje, algo1);
}

void imprimirAviso2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	/*if(archivoLog->is_active_console){
		printf(AMARILLO);
		printf(mensaje, algo1, algo2);
		printf(BLANCO);
		puts("");
	}*/
	log_warning(archivoLog, mensaje, algo1, algo2);
}

void imprimirAviso3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	/*if(archivoLog->is_active_console){
		printf(AMARILLO);
		printf(mensaje, algo1, algo2, algo3);
		printf(BLANCO);
		puts("");
	}*/
	log_warning(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirAviso4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	/*if(archivoLog->is_active_console){
		printf(AMARILLO);
		printf(mensaje, algo1, algo2, algo3, algo4);
		printf(BLANCO);
		puts("");
	}*/
	log_warning(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirError(ArchivoLog archivoLog, String mensaje) {
	/*if(archivoLog->is_active_console){
		printf(ROJO);
		puts(mensaje);
		printf(BLANCO);
	}*/
	log_error(archivoLog, mensaje);
}
void imprimirError1(ArchivoLog archivoLog, String mensaje, void* algo1) {
	/*if(archivoLog->is_active_console){
		printf(ROJO);
		printf(mensaje, algo1);
		printf(BLANCO);
		puts("");
	}*/
	log_error(archivoLog, mensaje, algo1);
}

void imprimirError2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	/*if(archivoLog->is_active_console){
		printf(ROJO);
		printf(mensaje, algo1, algo2);
		printf(BLANCO);
		puts("");
	}*/
	log_error(archivoLog, mensaje, algo1, algo2);
}

void imprimirError3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	/*if(archivoLog->is_active_console){
		printf(ROJO);
		printf(mensaje, algo1, algo2, algo3);
		printf(BLANCO);
		puts("");
	}*/
	log_error(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirError4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	/*if(archivoLog->is_active_console){
		printf(ROJO);
		printf(mensaje, algo1, algo2, algo3, algo4);
		printf(BLANCO);
		puts("");
	}*/
	log_error(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirVerde(ArchivoLog archivoLog, String mensaje) {
/*	if(archivoLog->is_active_console){
		printf(VERDE);
		puts(mensaje);
		printf(BLANCO);
	}*/
	log_info(archivoLog, mensaje);
}


void imprimirVerde1(ArchivoLog archivoLog, String mensaje, void* algo1) {
/*	if(archivoLog->is_active_console){
		printf(VERDE);
		printf(mensaje, algo1);
		printf(BLANCO);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1);
}

void imprimirVerde2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	/*if(archivoLog->is_active_console){
		printf(VERDE);
		printf(mensaje, algo1, algo2);
		printf(BLANCO);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2);
}

void imprimirVerde3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	/*if(archivoLog->is_active_console){
		printf(VERDE);
		printf(mensaje, algo1, algo2, algo3);
		printf(BLANCO);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirVerde4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
/*	if(archivoLog->is_active_console){
		printf(VERDE);
		printf(mensaje, algo1, algo2, algo3, algo4);
		printf(BLANCO);
		puts("");
	}*/
	log_info(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirMensajeProceso(String mensaje) {
	puts("-------------------------------------------------------------------------");
	puts(mensaje);
	puts("-------------------------------------------------------------------------");
}

//--------------------------------------- Funciones varias -------------------------------------

void senialAsignarFuncion(int unaSenial, void(*funcion)(int)) {
	signal(unaSenial, funcion);
}

Puntero memoriaAlocar(size_t dato) {
	return malloc(dato);
}

void memoriaLiberar(Puntero puntero) {
	if(puntero != NULL)
		free(puntero);
}

void pantallaLimpiar() {
//	system("clear");
}

int caracterObtener() {
	return getchar();
}


bool caracterDistintos(char unCaracter, char otroCaracter) {
	return unCaracter != otroCaracter;
}

bool caracterIguales(char unCaracter, char otroCaracter) {
	return unCaracter == otroCaracter;
}

File fileAbrir(String rutaArchivo, String modoApertura) {
	File archivo = fopen(rutaArchivo, modoApertura);
	return archivo;
}

void fileCerrar(File unArchivo) {
	fclose(unArchivo);
}

void fileLimpiar(String ruta) {
	File archivo = fopen(ruta, "r");
	if(archivo != NULL) {
		remove(ruta);
		fclose(archivo);
	}
}

/*+++++++++++++++++++++++++++++++++++++++ PROTOCOLO +++++++++++++++++++++++++++++++++++++++++++++++++++*/
buffer_com_t serializar_request(req_com_t msg)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion=REQUEST;

	//Calculo el tamaño del payload
	//El payload contiene el largo del string y el string del request
	tam_payload = sizeof(int) + msg.tam;

	//Aloco en memoria el stream
	//El stream contiene el tipo de conexión, el tamaño del payload y el payload
	buf.tam = sizeof(conexion_t)+sizeof(int)+tam_payload;
	buf.stream = malloc(buf.tam);

	//Ahora cargo el stream con los datos que necesito

	//Primero el tipo
	memcpy(buf.stream+desp,&tipo_conexion,sizeof(conexion_t));
	desp += sizeof(conexion_t);

	//Ahora el tamaño
	memcpy(buf.stream+desp,&tam_payload,sizeof(int));
	desp += sizeof(int);

	//Ahora el largo del request
	memcpy(buf.stream+desp,&msg.tam,sizeof(int));
	desp += sizeof(int);

	//Después el request
	memcpy(buf.stream+desp,msg.str,msg.tam);
	desp += tam_payload;
	return buf;
}

buffer_com_t serializar_respuesta(resp_com_t resp)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion = RESPUESTA;

	//Calculo el tamaño del payload
	//El payload contiene el tipo de respuesta, el largo del string y el string
	tam_payload = sizeof(resp_tipo_com_t) + sizeof(int) + resp.msg.tam;

	//Aloco en memoria el stream
	//El stream contiene el tipo de conexión, el tamaño del payload y el payload
	buf.tam = sizeof(conexion_t)+sizeof(int)+tam_payload;
	buf.stream = malloc(buf.tam);

	//Ahora cargo el stream con los datos que necesito

	//Primero el tipo
	memcpy(buf.stream+desp,&tipo_conexion,sizeof(conexion_t));
	desp += sizeof(conexion_t);

	//Ahora el tamaño
	memcpy(buf.stream+desp,&tam_payload,sizeof(int));
	desp += sizeof(int);

	//Ahora el tipo de respuesta
	memcpy(buf.stream+desp,&resp.tipo,sizeof(resp_tipo_com_t));
	desp += sizeof(resp_tipo_com_t);

	//Ahora el largo del request
	memcpy(buf.stream+desp,&resp.msg.tam,sizeof(int));
	desp += sizeof(int);

	if(resp.msg.tam > 0 && resp.msg.str != NULL){
		//Después el request
		memcpy(buf.stream+desp,resp.msg.str,resp.msg.tam);
		desp += tam_payload;
	}
	return buf;
}

buffer_com_t serializar_gossiping(gos_com_t msg)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion=GOSSIPING;

	//Calculo el tamaño del payload
	//El payload contiene cantidad de tablas (n) y nro memoria, ip y puerto n veces
	tam_payload = sizeof(int)+(sizeof(int)+LARGO_IP+LARGO_PUERTO)*msg.cant;

	//Aloco en memoria el stream
	//El stream contiene el tipo de conexión, el tamaño del payload y el payload
	buf.tam = sizeof(conexion_t)+sizeof(int)+tam_payload;
	buf.stream = malloc(buf.tam);

	//Ahora cargo el stream con los datos que necesito

	//Primero el tipo
	memcpy(buf.stream+desp,&tipo_conexion,sizeof(conexion_t));
	desp += sizeof(conexion_t);

	//Después el tamaño del payload
	memcpy(buf.stream+desp,&tam_payload,sizeof(int));
	desp += sizeof(int);

	//Ahora la cantidad de tablas de gossiping
	memcpy(buf.stream+desp,&(msg.cant),sizeof(int));
	desp += sizeof(int);

	//Ahora la info de cada una de las tablas
	for(int i=0; i<msg.cant; i++)
	{
		//Número de memoria
		memcpy(buf.stream+desp,&(msg.seeds[i].numMemoria),sizeof(int));
		desp += sizeof(int);

		//Ip
		memcpy(buf.stream+desp,(msg.seeds[i].ip),LARGO_IP);
		desp += LARGO_IP;

		//Ip
		memcpy(buf.stream+desp,(msg.seeds[i].puerto),LARGO_PUERTO);
		desp += LARGO_PUERTO;
	}

	return buf;
}

int enviar_request(int socket, req_com_t enviar)
{
	int retval = 0;
	buffer_com_t serializado;
	serializado = serializar_request(enviar);
	if(send(socket, serializado.stream, serializado.tam, 0)==-1)
		retval = -1;
	borrar_buffer(serializado);
	return retval;
}

int enviar_respuesta(int socket, resp_com_t enviar)
{
	int retval = 0;
	buffer_com_t serializado;
	serializado = serializar_respuesta(enviar);
	if(send(socket, serializado.stream, serializado.tam, 0)==-1)
		retval = -1;
	borrar_buffer(serializado);
	return retval;
}

int enviar_gossiping(int socket, gos_com_t enviar)
{
	int retval = 0;
	buffer_com_t serializado;
	serializado = serializar_gossiping(enviar);
	if(send(socket, serializado.stream, serializado.tam, 0)==-1)
		retval = -1;
	borrar_buffer(serializado);
	return retval;
}

msg_com_t recibir_mensaje(int conexion)
{
	msg_com_t recibido;
	recibido.payload.tam = 0;
	recibido.payload.stream = NULL;

	//Primero recibo el tipo
//	printf("\nVoy a esperar recibir el tipo");
	if(recv(conexion, &(recibido.tipo), sizeof(conexion_t), 0) < sizeof(conexion_t)){
		recibido.tipo = DESCONECTADO;
//		printf("\nError al recibir el tipo");
		return recibido;
	}
//	printf("\nRecibi el tipo");

	//Ahora recibo el tamaño
	if(recv(conexion, &(recibido.payload.tam), sizeof(int), MSG_WAITALL) <= 0){
			recibido.tipo = DESCONECTADO;
//			printf("\nError al recibir el tamaño");
			return recibido;
	}

//	printf("\nRecibi el tamaño");

	//Ahora aloco en memoria el stream
	recibido.payload.stream = malloc(recibido.payload.tam);

	//Ahora recibo el payload de tamaño ya conocido
	if(recv(conexion, recibido.payload.stream, recibido.payload.tam, MSG_WAITALL) <= 0){
			recibido.tipo = DESCONECTADO;
			borrar_buffer(recibido.payload);
//			printf("\nError al recibir el payload");
			return recibido;
	}
//	printf("\nRecibi el payload");

	return recibido;
}

req_com_t procesar_request(msg_com_t msg)
{
	req_com_t req;
	int desp = 0;

	//Tengo que desempaquetar el stream

	//Primero tengo el largo del request
	memcpy(&req.tam, msg.payload.stream+desp, sizeof(int));
	desp += sizeof(int);

	//Ahora ya sé el largo del string
	req.str = malloc(req.tam);

	memcpy(req.str, msg.payload.stream+desp, req.tam);

	return req;
}

handshake_com_t procesar_handshake(msg_com_t msg)
{
	handshake_com_t hs;
	int desp = 0;
	//Tengo que desempaquetar el stream

	//Primero tengo el id
	memcpy(&hs.id,msg.payload.stream,sizeof(id_com_t));
	desp += sizeof(id_com_t);
	//Ahora tengo el largo del string
	memcpy(&hs.msg.tam, msg.payload.stream+desp, sizeof(int));
	desp += sizeof(int);
	//Si el tamaño del string es mayor a 0, aloco en memoria el espacio para almacenarlo y lo almaceno
	if(hs.msg.tam>0){
		hs.msg.str = malloc(hs.msg.tam);
		memcpy(hs.msg.str, msg.payload.stream+desp, hs.msg.tam);
	}
	return hs;
}

resp_com_t procesar_respuesta(msg_com_t msg)
{
	resp_com_t resp;

	int desp = 0;

	//Tengo que desempaquetar el stream

	//Primero tengo el tipo de respuesta
	memcpy(&resp.tipo, msg.payload.stream+desp, sizeof(resp_tipo_com_t));
	desp += sizeof(resp_tipo_com_t);

	//Despues el largo del request
	memcpy(&resp.msg.tam, msg.payload.stream+desp, sizeof(int));
	desp += sizeof(int);

	//Ahora ya sé el largo del string
	if(resp.msg.tam > 0){
		resp.msg.str = malloc(resp.msg.tam);
		memcpy(resp.msg.str, msg.payload.stream+desp, resp.msg.tam);
	}
	else
		resp.msg.str = NULL;
	return resp;

}

gos_com_t procesar_gossiping(msg_com_t msg)
{
	gos_com_t gossip;
	int desp = 0;

	//Tengo que desempaquetar el stream

	//Primero tengo la cantidad de tablas
	memcpy(&gossip.cant, msg.payload.stream+desp, sizeof(int));
	desp += sizeof(int);

	//Ahora ya sé cuantas tablas tengo
	gossip.seeds = malloc(sizeof(seed_com_t)*gossip.cant);

	for(int i=0; i<gossip.cant; i++)
	{
		memcpy(&gossip.seeds[i].numMemoria, msg.payload.stream+desp, sizeof(int));
		desp += sizeof(int);
		memcpy(gossip.seeds[i].ip, msg.payload.stream+desp, LARGO_IP);
		desp += LARGO_IP;
		memcpy(gossip.seeds[i].puerto, msg.payload.stream+desp, LARGO_PUERTO);
		desp += LARGO_PUERTO;

	}

	return gossip;
}

void borrar_request_com(req_com_t req)
{
	if(req.tam >0)
		free(req.str);
}

void borrar_respuesta(resp_com_t resp)
{
	if(resp.msg.tam > 0)
		free(resp.msg.str);
}

void borrar_buffer(buffer_com_t buf)
{
	if(buf.tam>0 && buf.stream != NULL)
		free(buf.stream);
}

void borrar_mensaje(msg_com_t msg)
{
	borrar_buffer(msg.payload);
}


int iniciar_servidor(char*ip,char*puerto)
{
	int socket_servidor=-1;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;
        int yes = 1;
        // lose the pesky "Address already in use" error message
        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("Error");
            exit(1);
        }

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    //log_trace(logger, "Listo para escuchar a mi cliente");
//   printf("\n**Servidor listo para escuchar al cliente**\n");
    //log_info(logger,"Servidor listo para escuchar al cliente");

    return socket_servidor;
}

int conectar_a_servidor(char *ip,char *puerto, id_com_t id)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

//	printf("\nIP: %s\nPUERTO: %s", ip, puerto);
	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		//printf("\n**No se pudo establecer conexion con el servidor**\n");
		freeaddrinfo(server_info);
		return -1;
    }
/*
	AddrInfo* infoaux;
	while(server_info!=NULL){
		infoaux = server_info->ai_next;
		free(server_info->ai_canonname);
			free(server_info->ai_addr);
			free(server_info);
			server_info= infoaux;
	}
*/
	freeaddrinfo(server_info);

//	printf("\nMe conecté!");

	//ENVIAR HANDSHAKE CON TIPO CLIENTE
	handshake_com_t hs;
	hs.id = id;

	//No me interesa enviar ningún mensaje de presentación
	hs.msg.tam = 0;

	enviar_handshake(socket_cliente, hs);
	borrar_handshake(hs);

//	printf("\nMe presenté!");
	return socket_cliente;
}

cliente_com_t esperar_cliente(int servidor)
{
	cliente_com_t cliente;
	struct sockaddr_in dir_cliente;
	unsigned int tam_direccion = sizeof(struct sockaddr_in);
	int socket_cliente = accept(servidor, (void*) &dir_cliente, &tam_direccion);
	msg_com_t msg;
	handshake_com_t hs;
	cliente.socket = socket_cliente;
	if(socket_cliente == -1)
		return cliente;
	msg = recibir_mensaje(socket_cliente);
	hs = procesar_handshake(msg);
	borrar_mensaje(msg);

	//Sólo me interesa el id
	cliente.id = hs.id;
	borrar_handshake(hs);

	return cliente;
}

buffer_com_t serializar_handshake(handshake_com_t msg)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion=HANDSHAKECOMANDO;

	//Calculo el tamaño del payload
	//El payload contiene la identificación, el largo del string y el string
	tam_payload = sizeof(id_com_t) + sizeof(int) + msg.msg.tam;

	//Aloco en memoria el stream
	//El stream contiene el tipo de conexión, el tamaño y el payload
	buf.tam = sizeof(conexion_t)+sizeof(int)+tam_payload;
	buf.stream = malloc(buf.tam);

	//Ahora cargo el stream con los datos que necesito

	//Primero el tipo
	memcpy(buf.stream+desp,&tipo_conexion,sizeof(conexion_t));
	desp += sizeof(conexion_t);

	//Ahora el tamaño
	memcpy(buf.stream+desp,&tam_payload,sizeof(int));
	desp += sizeof(int);

	//Después la identificacion
	memcpy(buf.stream+desp,&msg.id,sizeof(id_com_t));
	desp += sizeof(id_com_t);

	//Ahora el largo del string
	memcpy(buf.stream+desp,&msg.msg.tam,sizeof(int));
	desp += sizeof(int);

	//Finalmente el string (si su tamaño es mayor a 0)
	if(msg.msg.tam > 0)
		memcpy(buf.stream+desp,msg.msg.str,msg.msg.tam);

	return buf;
}

int enviar_handshake(int socket, handshake_com_t enviar)
{
	int retval = 0;
	buffer_com_t serializado;
	serializado = serializar_handshake(enviar);
	if(send(socket, serializado.stream, serializado.tam, 0)==-1)
		retval = -1;
	borrar_buffer(serializado);
	return retval;
}

void borrar_string(str_com_t str)
{
	if(str.tam > 0)
		free(str.str);
}

void borrar_handshake(handshake_com_t hs)
{
	borrar_string(hs.msg);
}

void borrar_gossiping(gos_com_t gos)
{
	if(gos.seeds != NULL && gos.cant > 0)
		free(gos.seeds);
}


int dar_bienvenida_cliente(int socket, id_com_t id, char *msg)
{
	handshake_com_t hs;
	int retval = 1;
	hs.id = id;
	hs.msg.tam = strlen(msg)+1;
	hs.msg.str = malloc(hs.msg.tam);
	strcpy(hs.msg.str,msg);
	if(enviar_handshake(socket,hs) == -1){
		retval = -1;
	}
	borrar_handshake(hs);
	return retval;
}

int rechazar_cliente(int socket, char *msg)
{
	handshake_com_t hs;
	int retval = 1;
	hs.id = RECHAZADO;
	if(msg != NULL){
		hs.msg.tam = strlen(msg)+1;
		hs.msg.str = malloc(hs.msg.tam);
		strcpy(hs.msg.str,msg);
	}
	else{
		hs.msg.tam = 0;
		hs.msg.str = NULL;
	}
	if(enviar_handshake(socket,hs) == -1)
		retval = -1;
	borrar_handshake(hs);
	return retval;
}

int responder_request(int socket,char *msg, resp_tipo_com_t tipo_resp)
{
	resp_com_t resp;
	resp.tipo = tipo_resp;
	if(msg != NULL){
		resp.msg.tam = strlen(msg)+1;
		resp.msg.str = malloc(resp.msg.tam);
		strcpy(resp.msg.str,msg);
	}
	else{
		resp.msg.tam = 0;
		resp.msg.str = NULL;
	}
	if(enviar_respuesta(socket,resp)==-1){
		borrar_respuesta(resp);
		return -1;
	}
	borrar_respuesta(resp);
	return 1;
}

resp_com_t armar_respuesta(resp_tipo_com_t tipo,char *msg)
{
	resp_com_t respuesta;
	respuesta.tipo = tipo;
	if(msg == NULL){
		respuesta.msg.tam = 0;
		respuesta.msg.str = NULL;
	}
	else{
		respuesta.msg.tam = strlen(msg)+1;
		respuesta.msg.str = malloc(respuesta.msg.tam);
		strcpy(respuesta.msg.str,msg);
	}
	return respuesta;
}
