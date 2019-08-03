/*
 * BibliotecaSockets.h
 *
 *  Created on: 12/8/2017
 *      Author: mi_ultimo_segundo_tp
 */

//--------------------------------------- Includes -------------------------------------

#ifndef BIBLIOTECA_INC
#define BIBLIOTECA_INC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "commons/config.h"
#include "commons/log.h"
#include "commons/string.h"
#include <commons/collections/queue.h>
#include "commons/collections/list.h"
#include "commons/temporal.h"
#include "commons/bitarray.h"
#include <stdint.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

//--------------------------------------- Constantes -------------------------------------

#define IP_LOCAL "127.0.0.1"
#define LISTEN 10
#define ERROR -1
#define DESCONEXION 0
#define NULO 0
#define OK 1
#define HANDSHAKE 1
#define ACTIVADO 1
#define DESACTIVADO 0
#define VACIO ""
#define ESCRITURA "w"
#define LECTURA "r"
#define COMANDO_SIZE 35
#define ROJO "\x1b[31m"
#define AMARILLO "\x1b[33m"
#define BLANCO "\x1b[0m"
#define VERDE "\x1b[32m"
#define SEPARADOR "|"

//--------------------------------------- Definiciones -------------------------------------

typedef int Socket;
typedef fd_set ListaSockets;
typedef void* Puntero;
typedef char* String;
typedef socklen_t Socklen;
typedef struct addrinfo* AddrInfo;
typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr* SockAddr;
typedef t_config* ArchivoConfig;
typedef t_log* ArchivoLog;
typedef t_log_level NivelLog;
typedef sem_t Semaforo;
typedef pthread_mutex_t Mutex;
typedef pthread_rwlock_t RWlock;
typedef pthread_t Hilo;
typedef t_list* Lista;
typedef FILE* File;
typedef int32_t Entero;
typedef t_bitarray* Bitmap;
struct sockaddr_in miDireccionSocket;
char* mensaje;


//--------------------------------------- Estructuras -------------------------------------

typedef struct __attribute__((packed)){
	Entero operacion;
	Entero tamanio;
} Header;

typedef struct __attribute__((packed)){
	Header header;
	Puntero datos;
} Mensaje;

typedef struct __attribute__((__packed__)){
	char ip[20];
	char port[20];
	char nombre[10];
} Dir;

typedef struct {
	AddrInfo informacion;
	SockAddrIn address;
	Socklen tamanioAddress;
	String port;
	String ip;
} Conexion;

typedef struct{
	int unsigned comando;
	int unsigned tamanio;
	int unsigned cantArgumentos;
}t_header;


enum comandos{
	Select = 0,
	insert,
	create,
	describe,
	drop,
	journal,
	add,
	run,
	metrics,
	salir

};

typedef uint64_t timestamp_t;

//FUNCIONES PARA ABORTAR UN PROCESO
void abortarProcesoPorUnError(t_log log, char* mensaje);
void abortarProcesoPorUnErrorImportante(t_log* log, char* mensaje);

//--------------------------------------- Funcion timestamp -------------------------------------

//timestamp_t timestamp(void);
timestamp_t timestamp(void);
//--------------------------------------- Funciones para Socket -------------------------------------

//void socketConfigurar(Conexion* conexion, String ip, int puerto, t_log* logger);
//int socketCrear(Conexion* conexion, String ip, int puerto, t_log* logger);

Socket nuevoSocket(t_log* logger);
int conectarSocket(int fd_socket, const char* ipDestino, int puerto,t_log* logger);
//void socketConectar(Conexion* conexion, Socket unSocket);
struct sockaddr_in asociarSocket(int fd_socket, int puerto,t_log* logger);
//void socketBindear(Conexion* conexion, Socket unSocket);
void socketEscuchar(Socket unSocket, int ClientesEnEspera,t_log* logger);
int aceptarConexionSocket(int fd_socket,t_log* logger);
int socketAceptar(Socket unSocket, int idEsperada,t_log* logger);
void socketRedireccionar(Socket unSocket);
void socketSelect(Socket cantidadSockets, ListaSockets* listaSockets,int);
int socketRecibir(Socket socketEmisor, Puntero buffer, int tamanioBuffer,t_log* logger);
int socketEnviar(Socket socketReceptor, Puntero mensaje, int tamanioMensaje,t_log* logger);
void socketCerrar(Socket unSocket);
bool socketSonIguales(Socket unSocket, Socket otroSocket);
bool socketSonDistintos(Socket unSocket, Socket otroSocket);
bool socketEsMayor(Socket unSocket, Socket otroSocket);
void socketError(int estado, String error);
Socket socketCrearListener(String ip, String puerto, t_log* logger);
Socket socketCrearCliente(String ip, String puerto, int idProceso, t_log* logger);

//--------------------------------------- Funciones para ListaSocket -------------------------------------

void listaSocketsAgregar(Socket unSocket, ListaSockets* listaSockets);
void listaSocketsEliminar(Socket unSocket, ListaSockets* listaSockets);
bool listaSocketsContiene(Socket unSocket, ListaSockets* listaSockets);
void listaSocketsLimpiar(ListaSockets* listaSockets);

//--------------------------------------- Funciones para Mensaje -------------------------------------

void* mensajeCrear(int operacion, Puntero dato, int tamanioDato);
int mensajeEnviar(int socketReceptor, Entero operacion, void* dato, Entero tamanioDato, t_log* logger);
bool mensajeOperacionIgualA(Mensaje* mensaje, int operacion);
Mensaje* mensajeRecibir(Socket socketEmisor, t_log* logger);
void mensajeDestruir(Mensaje* mensaje);
void mensajeAvisarDesconexion(Mensaje* mensaje);
bool mensajeConexionFinalizada(int bytes);
void mensajeRevisarConexion(Mensaje* mensaje, Socket socketReceptor, int bytes,t_log* logger);
void mensajeObtenerDatos(Mensaje* mensaje, Socket socketReceptor,t_log* logger);
bool mensajeDesconexion(Mensaje* mensaje) ;


//--------------------------------------- Funciones para Header -------------------------------------

Header headerCrear(int operacion, int tamanio);

//--------------------------------------- Funciones para ArchivoConfig -------------------------------------

ArchivoConfig archivoConfigCrear(String path, String* campos);
bool archivoConfigTieneCampo(ArchivoConfig archivoConfig, String campo);
bool archivoConfigFaltaCampo(ArchivoConfig archivoConfig, String campo);
String archivoConfigStringDe(ArchivoConfig archivoConfig, String campo);
int archivoConfigEnteroDe(ArchivoConfig archivoConfig, String campo);
long archivoConfigLongDe(ArchivoConfig archivoConfig, String campo);
double archivoConfigDoubleDe(ArchivoConfig archivoConfig, String campo);
String* archivoConfigArrayDe(ArchivoConfig archivoConfig, String campo);
int archivoConfigCantidadCampos(ArchivoConfig archivoConfig);
void archivoConfigDestruir(ArchivoConfig archivoConfig);
void archivoConfigSetearCampo(ArchivoConfig archivoConfig, String campo, String valor);
bool archivoConfigInvalido(ArchivoConfig archivoConfig, String* campos);
bool archivoConfigIncompleto(ArchivoConfig archivoConfig, String* campos);
bool archivoConfigInexistente(ArchivoConfig archivoConfig);

//--------------------------------------- Funciones para ArchivoLog -------------------------------------

ArchivoLog archivoLogCrear(String nombreArchivo, String nombrePrograma);
void archivoLogDestruir(ArchivoLog archivoLog);
void archivoLogInformarMensaje(ArchivoLog archivoLog, String mensaje);
void archivoLogInformarPeligro(ArchivoLog archivoLog, String mensajePeligro);
void archivoLogInformarError(ArchivoLog archivoLog, String mensajeError);
void archivoLogInformarTrace(ArchivoLog archivoLog, String mensajeTrace);
void archivoLogInformarDebug(ArchivoLog archivoLog, String mensajeDebug);
String archivoLogNivelLogAString(NivelLog nivelLog);
NivelLog archivoLogStingANivelLog(String stringNivelLog);
void archivoLogValidar(String rutaArchivo);

void imprimirMensaje(ArchivoLog archivoLog, String mensaje);
void imprimirMensaje1(ArchivoLog archivoLog, String mensaje, void* algo1);
void imprimirMensaje2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2);
void imprimirMensaje3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3);
void imprimirMensaje4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4);
void imprimirAviso(ArchivoLog archivoLog, String mensaje);
void imprimirAviso1(ArchivoLog archivoLog, String mensaje, void* algo1);
void imprimirAviso2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2);
void imprimirAviso3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3);
void imprimirAviso4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4);
void imprimirError(ArchivoLog archivoLog, String mensaje);
void imprimirError1(ArchivoLog archivoLog, String mensaje, void* algo1);
void imprimirError2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2);
void imprimirError3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3);
void imprimirError4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4);
void imprimirVerde(ArchivoLog archivoLog, String mensaje);
void imprimirVerde1(ArchivoLog archivoLog, String mensaje, void* algo1);
void imprimirVerde2(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2);
void imprimirVerde3(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3);
void imprimirVerde4(ArchivoLog archivoLog, String mensaje, void* algo1, void* algo2, void* algo3, void* algo4);
void imprimirMensajeProceso(String mensaje);

//--------------------------------------- Funciones para Semaforo -------------------------------------

void semaforoIniciar(Semaforo* semaforo, unsigned int valor);
void semaforoWait(Semaforo* semaforo);
void semaforoSignal(Semaforo* semaforo);
void semaforoValor(Semaforo* semaforo, int* buffer);
void semaforoDestruir(Semaforo* semaforo);

//--------------------------------------- Funciones para Mutex -------------------------------------

void mutexIniciar(Mutex* mutex);
void mutexBloquear(Mutex* mutex);
void mutexDesbloquear(Mutex* mutex);

//--------------------------------------- Funciones para RWLock -------------------------------------

void rwLockIniciar(RWlock* rwLock);
void rwLockLeer(RWlock* rwLock);
void rwLockEscribir(RWlock* rwLock);
void rwLockDesbloquear(RWlock* rwLock);

//--------------------------------------- Funciones para Hilo -------------------------------------

void hiloCrear(Hilo* hilo, void*(*funcionHilo)(void*), void* parametroHilo);
void hiloSalir();
void hiloCancelar(Hilo hilo);
void hiloEsperar(Hilo hilo);
void hiloDetach(Hilo hilo);
Hilo hiloId();

//--------------------------------------- Funciones para Lista -------------------------------------

Lista listaCrear();
void listaDestruir(Lista lista);
void listaDestruirConElementos(Lista lista, void(*funcion)(void*));
int listaAgregarElemento(Lista lista, void* elemento);
int list_addM(Lista,void*,size_t);
int listaAgregarElementoM(Lista lista, void* elemento,size_t);
void listaAgregarEnPosicion(Lista lista, void* elemento, int posicion);
void listaAgregarOtraLista(Lista lista, Lista otraLista);
void* listaObtenerElemento(Lista lista, int posicion);
Lista listaTomar(Lista lista, int cantidadElementos);
Lista listaSacar(Lista lista, int cantidadElementos);
Lista listaFiltrar(Lista lista, bool(*funcion)(void*));
Lista listaMapear(Lista lista, void*(*funcion)(void*));
void* listaReemplazarElemento(Lista lista, void* elemento, int posicion);
void listaReemplazarDestruyendoElemento(Lista lista, void* elemento, int posicion, void(*funcion)(void*));
void listaEliminarElemento(Lista lista, int posicion);
void listaEliminarDestruyendoElemento(Lista lista, int posicion, void(*funcion)(void*));
void listaEliminarPorCondicion(Lista lista, bool(*funcion)(void*));
void listaEliminarDestruyendoPorCondicion(Lista lista, bool(*funcion)(void*), void(*funcionDestruir)(void*));
void listaLimpiar(Lista lista);
void listaLimpiarDestruyendoElementos(Lista lista, void(*funcion)(void*));
void listaIterar(Lista lista, void(*funcion)(void*));
void* listaBuscar(Lista lista, bool(*funcion)(void*));
int listaCantidadElementos(Lista lista);
bool listaEstaVacia(Lista lista);
void listaOrdenar(Lista lista, bool(*funcion)(void*, void*));
int listaCuantosCumplen(Lista lista, bool(*funcion)(void*));
bool listaCumpleAlguno(Lista lista, bool(*funcion)(void*));
bool listaCumplenTodos(Lista lista, bool(*funcion)(void*));
void* listaPrimerElemento(Lista lista);
bool listaTieneElementos(Lista lista);

//--------------------------------------- Funciones para String -------------------------------------

String stringCrear();
bool stringContiene(String unString, String otroString);
String stringConvertirEntero(int entero);
String stringRepetirCaracter(char caracter, int repeticiones);
void  stringConcatenarString(String unString, String otroString);
String stringDuplicar(String string);
void stringPonerEnMayuscula(String string);
void stringPonerEnMinuscula(String string);
void stringPonerEnCapital(String string);
void stringRemoverVacios(String* string);
void stringRemoverVaciosIzquierda(String* string);
void stringRemoverVaciosDerecha(String* string);
int stringLongitud(String string);
bool stringEstaVacio(String string);
bool stringEmpiezaCon(String string, String stringComienzo);
bool stringTerminaCon(String string, String stringTerminacion);
String stringDarVuelta(String string);
String stringTomarCantidad(String string, int desde, int cantidad);
String stringTomarDesdePosicion(String string, int posicion);
String stringTomarDesdeInicio(String string, int cantidad);
void stringLimpiar(String string, int tamanioString);

//--------------------------------------- Funciones de HandShake-------------------------------------

int handShakeRecepcionExitosa(Socket unSocket, int idEsperada,t_log* logger);
int handShakeEnvioExitoso(Socket unSocket, int idProceso,t_log* logger);
void handShakeError(Socket unSocket);
int handShakeRecepcionFallida(Socket unSocket, int idEsperada,t_log* logger);
int handShakeEnvioFallido(Socket unSocket, int idProceso,t_log* logger);
bool handShakeRealizado(Mensaje* mensaje);
bool handShakeAceptado(Mensaje* mensaje);
bool handShakeIdsIguales(int idEnviada, int idEsperada);

//--------------------------------------- Funciones de Bitmap -------------------------------------

int bitmapCalculo(int cantidadBloques);
Bitmap bitmapCrear(int cantidadBloques);
void bitmapLiberarBit(Bitmap bitmap, int posicion);
void bitmapDestruir(Bitmap bitmap);
void bitmapOcuparBit(Bitmap bitmap, int posicion);
bool bitmapBitOcupado(Bitmap bitmap, int posicion);
size_t bitmapCantidadBits(Bitmap bitmap);
int bitmapCalculo(int cantidadBloques);

//--------------------------------------- Funciones varias -------------------------------------

File fileAbrir(String rutaArchivo, String modoApertura);
void fileCerrar(File unArchivo);
Puntero memoriaAlocar(size_t dato);
void memoriaLiberar(Puntero puntero);
void configuracionSenialHijo(int senial);
void imprimirMensajeProceso(String mensaje);
void fileLimpiar(String ruta);


/************************************PROTOCOLO ****************************************************/

#define LARGO_IP 20
#define LARGO_PUERTO 10

/* Tipos de datos */

//Tipos de mensaje soportados
typedef enum
{
	GOSSIPING,
	REQUEST,
	ERRORCOMANDO,
	HANDSHAKECOMANDO,
	RESPUESTA,
	DESCONECTADO //Lo agregué para poder detectar y manejar la caída del servidor
} conexion_t;

//Identificación del tipo de proceso
typedef enum
{
	LFS,
	MEMORIA,
	KERNEL,
	RECHAZADO
} id_com_t;

//Tipo de respuestas a pedidos
typedef enum{
	RESP_OK,														//0
	//DESDE ACA LOS QUE NO GENERAN UN FALLO EN KERNEL
	RESP_ERROR_METADATA,											//1
	RESP_ERROR_NO_HAY_TABLAS,										//2
	RESP_ERROR_COMUNICACION,										//3
	RESP_ERROR_GENERAL,												//4
	RESP_ERROR_KEY_NO_EXISTE,										//5
	//DESDE ACA LOS QUE SI GENERAN UN FALLO EN KERNEL
	RESP_ERROR_PEDIDO_DESCONOCIDO,									//6
	RESP_ERROR_SIN_MEMORIAS_CRITERIO,								//7
	RESP_ERROR_SIN_MEMORIAS_ASOCIADAS,								//8
	RESP_ERROR_DESCONOZCO_CRITERIO_TABLA,							//9
	RESP_ERROR_TABLA_NO_EXISTE,										//10
	RESP_ERROR_CANT_PARAMETROS,										//11
	RESP_ERROR_MAYOR_MAX_VALUE,										//12
	RESP_ERROR_TABLA_YA_EXISTE										//13
} resp_tipo_com_t;

//String de tamaño 'tam'
typedef struct{
	int tam;
	char *str;
} str_com_t;

//Respuestas a pedidos
typedef struct{
	resp_tipo_com_t tipo;
	str_com_t msg;
} resp_com_t;

//Por el momento dijimos que un request es un string
typedef str_com_t req_com_t;

//Por el momento dijimos que un error es un string
typedef str_com_t error_com_t;

//Hanshake. Tiene un id y puede tener un string. Si seteamos en 0 el tam del msg, este no se envía.
typedef struct
{
	id_com_t id;
	str_com_t msg;
} handshake_com_t;

//Registro de la tabla de gossiping
typedef struct{
	int numMemoria;
	char ip[LARGO_IP], puerto[LARGO_PUERTO];
} seed_com_t;

//Tabla de gossiping. Contiene 'cant' registros
typedef struct{
	int cant;
	seed_com_t *seeds;
} gos_com_t;

//Buffer. Similar a un string pero sin caracter de fin de cadena
typedef struct{
	int tam;
	void *stream;
} buffer_com_t;

//Mensaje crudo, sin procesar. Sólo se identifica el tipo de mensaje
typedef struct{
	conexion_t tipo;
	buffer_com_t payload;
} msg_com_t;

//Estructura para un cliente. Contiene socket de conexión e identificación
typedef struct{
	int socket;
	id_com_t id;
} cliente_com_t;


int enviar_request(int socket, req_com_t enviar);
int enviar_respuesta(int socket, resp_com_t enviar);
int enviar_handshake(int socket, handshake_com_t enviar);
int enviar_gossiping(int socket, gos_com_t enviar);
msg_com_t recibir_mensaje(int conexion);
req_com_t procesar_request(msg_com_t msg);
resp_com_t procesar_respuesta(msg_com_t msg);
handshake_com_t procesar_handshake(msg_com_t msg);
gos_com_t procesar_gossiping(msg_com_t msg);
void borrar_buffer(buffer_com_t buf);
void borrar_mensaje(msg_com_t msg);
void borrar_request_com(req_com_t req);
void borrar_respuesta(resp_com_t resp);
void borrar_handshake(handshake_com_t hs);
void borrar_gossiping(gos_com_t gos);

/**
* @NAME: iniciar_servidor
* @DESC: Inicializa el servidor en el ip y puerto indicado
*/
int iniciar_servidor(char*ip,char*puerto);

/**
* @NAME: esperar_cliente
* @DESC: Espera un cliente en el socket servidor y espera su handshake.
* 	  	 Nos devuelve una estructura con el socket y la identificación.
*/
cliente_com_t esperar_cliente(int servidor);

/**
* @NAME: conectar_a_servidor
* @DESC: Establece la conexión con el servidor y le envia la identificación
*/
int conectar_a_servidor(char *ip,char *puerto, id_com_t id);

int dar_bienvenida_cliente(int socket, id_com_t yo, char *msg);
int rechazar_cliente(int socket, char *msg);
int responder_request(int socket,char *msg, resp_tipo_com_t tipo_resp);
resp_com_t armar_respuesta(resp_tipo_com_t tipo,char *msg);

#endif
