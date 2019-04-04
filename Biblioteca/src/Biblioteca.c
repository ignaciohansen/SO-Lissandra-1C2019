/*
 ============================================================================
 Name        : BibliotecaGeneral.c
 Author      : Dario Poma
 Version     : 1.0
 Copyright   : Todos los derechos reservados papu
 Description : Biblioteca general para dummies
 ============================================================================
 */

#include "Biblioteca.h"

//--------------------------------------- Funciones para Socket -------------------------------------

void socketConfigurar(Conexion* conexion, String ip, String puerto) {
	struct addrinfo hints;
	memset(&hints, NULO, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(ip, puerto, &hints, &conexion->informacion); // Carga en serverInfo los datos de la conexion
}

Socket socketCrear(Conexion* conexion, String ip, String puerto) {
	socketConfigurar(conexion, ip, puerto);
	Socket unSocket = socket(conexion->informacion->ai_family, conexion->informacion->ai_socktype, conexion->informacion->ai_protocol);
	socketError(unSocket, "socket");
	return unSocket;
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

void socketEscuchar(Socket unSocket, int clientesEsperando) {
	int estado = listen(unSocket, clientesEsperando);
	socketError(estado, "listen");
}

Socket socketAceptar(Socket unSocket, int idEsperada) {
	Conexion conexion;
	conexion.tamanioAddress = sizeof(SockAddrIn);
	Socket nuevoSocket = accept(unSocket, (SockAddr)&conexion.address, &conexion.tamanioAddress);
	if(nuevoSocket != ERROR)
		if(handShakeRecepcionFallida(nuevoSocket, idEsperada))
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

int socketRecibir(Socket socketEmisor, Puntero buffer, int tamanioBuffer) {
	int estado = recv(socketEmisor, buffer, tamanioBuffer, MSG_WAITALL);
	if(estado == ERROR)
		perror("recv");
	return estado;
}

int socketEnviar(Socket socketReceptor, Puntero mensaje, int tamanioMensaje) {
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

Socket socketCrearListener(String ip, String puerto) {
	Conexion conexion;
	conexion.tamanioAddress = sizeof(conexion.address);
	Socket listener = socketCrear(&conexion, ip, puerto);
	socketRedireccionar(listener);
	socketBindear(&conexion, listener);
	socketEscuchar(listener, LISTEN);
	return listener;
}

Socket socketCrearCliente(String ip, String puerto, int idProceso) {
	Conexion conexion;
	Socket unSocket = socketCrear(&conexion, ip, puerto);
	socketConectar(&conexion, unSocket);
	if(handShakeEnvioFallido(unSocket, idProceso))
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

int mensajeEnviar(int socketReceptor, int32_t operacion, void* dato, int32_t tamanioDato) {
	void* buffer = mensajeCrear(operacion, dato, tamanioDato);
	int resultado = socketEnviar(socketReceptor, buffer, sizeof(Header)+tamanioDato);
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

void mensajeRevisarConexion(Mensaje* mensaje, Socket socketReceptor, int bytes) {
	if(mensajeConexionFinalizada(bytes))
		mensajeAvisarDesconexion(mensaje);
	else
		mensajeObtenerDatos(mensaje, socketReceptor);
}

void mensajeObtenerDatos(Mensaje* mensaje, Socket socketReceptor) {
	int tamanioDato = mensaje->header.tamanio;
	if(tamanioDato==0) {
		mensaje->datos=NULL;
		return;
	}
	mensaje->datos = malloc(tamanioDato);
	int bytes = socketRecibir(socketReceptor, mensaje->datos, tamanioDato);
	if(mensajeConexionFinalizada(bytes))
		mensajeAvisarDesconexion(mensaje);
}

Mensaje* mensajeRecibir(int socketReceptor) {
	Mensaje* mensaje = malloc(sizeof(Mensaje));
	int bytes = socketRecibir(socketReceptor, &mensaje->header, sizeof(Header));
	mensajeRevisarConexion(mensaje, socketReceptor, bytes);
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

int handShakeEnvioExitoso(Socket unSocket, int32_t idProceso) {
	int32_t id = idProceso;
	mensajeEnviar(unSocket, HANDSHAKE, &id, sizeof(int32_t));
	Mensaje* mensaje = mensajeRecibir(unSocket);
	int estado = handShakeRealizado(mensaje) && handShakeAceptado(mensaje);
	mensajeDestruir(mensaje);
	return estado;
}

int handShakeRecepcionExitosa(Socket unSocket, int idEsperada) {
	Mensaje* mensaje = mensajeRecibir(unSocket);
	int idProceso = (*(int32_t*)mensaje->datos);
	mensajeDestruir(mensaje);
	int32_t estado = handShakeIdsIguales(idProceso, idEsperada);
	mensajeEnviar(unSocket, HANDSHAKE, &estado, sizeof(int32_t));
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

int handShakeEnvioFallido(Socket unSocket, int idProceso) {
	return !handShakeEnvioExitoso(unSocket, idProceso);
}

int handShakeRecepcionFallida(Socket unSocket, int idEsperada) {
	return !handShakeRecepcionExitosa(unSocket, idEsperada);
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
	for(indice = 0; indice < tamanioBytes*8; indice++)
		bitarray_clean_bit(bitmap, indice);
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
	puts(mensaje);
	log_info(archivoLog, mensaje);
}

void imprimirMensaje1(ArchivoLog archivoLog, String mensaje, void* algo1) {
	printf(mensaje, algo1 );
	puts("");
	log_info(archivoLog, mensaje, algo1);
}

void imprimirMensaje2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	printf(mensaje, algo1, algo2);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2);
}

void imprimirMensaje3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	printf(mensaje, algo1, algo2, algo3);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirMensaje4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	printf(mensaje, algo1, algo2, algo3, algo4);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirAviso(ArchivoLog archivoLog, String mensaje) {
	printf(AMARILLO);
	puts(mensaje);
	printf(BLANCO);
	log_info(archivoLog, mensaje);
}

void imprimirAviso1(ArchivoLog archivoLog, String mensaje, void* algo1) {
	printf(AMARILLO);
	printf(mensaje, algo1 );
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1);
}

void imprimirAviso2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	printf(AMARILLO);
	printf(mensaje, algo1, algo2);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2);
}

void imprimirAviso3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	printf(AMARILLO);
	printf(mensaje, algo1, algo2, algo3);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirAviso4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	printf(AMARILLO);
	printf(mensaje, algo1, algo2, algo3, algo4);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2, algo3, algo4);
}

void imprimirError(ArchivoLog archivoLog, String mensaje) {
	printf(ROJO);
	puts(mensaje);
	printf(BLANCO);
	log_info(archivoLog, mensaje);
}
void imprimirError1(ArchivoLog archivoLog, String mensaje, void* algo1) {
	printf(ROJO);
	printf(mensaje, algo1);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1);
}

void imprimirError2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2) {
	printf(ROJO);
	printf(mensaje, algo1, algo2);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2);
}

void imprimirError3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3) {
	printf(ROJO);
	printf(mensaje, algo1, algo2, algo3);
	printf(BLANCO);
	puts("");
	log_info(archivoLog, mensaje, algo1, algo2, algo3);
}

void imprimirError4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4) {
	printf(ROJO);
	printf(mensaje, algo1, algo2, algo3, algo4);
	printf(BLANCO);
	puts("");
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
	system("clear");
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
