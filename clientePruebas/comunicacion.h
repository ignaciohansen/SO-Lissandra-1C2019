#ifndef COMUNICACION_INCLUDED
#define COMUNICACION_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <commons/config.h>

/* Definiciones */

#define LARGO_IP 15
#define LARGO_PUERTO 10


/* Tipos de datos */

//Tipos de mensaje soportados
typedef enum
{
	GOSSIPING,
	REQUEST,
	ERROR,
	HANDSHAKE,
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

//String de tamaño 'tam'
typedef struct{
	int tam;
	char *str;
} str_com_t;

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

//Tipo de respuestas a pedidos
typedef enum{
	RESP_OK, //Cualquier pedido
	RESP_TABLA_NO_EXISTE, //SELECT-DROP
	RESP_KEY_NO_EXISTE, //SELECT
	RESP_MEM_FULL, //SELECT-INSERT
	RESP_ERROR_GENERAL
} resp_tipo_com_t;

//Respuestas a pedidos
typedef struct{
	resp_tipo_com_t tipo;
	str_com_t msg;
} resp_com_t;

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


/* Prototipos de funciones */

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


/**
* @NAME: cerrar_conexion
* @DESC: Libera la conexión. Sirve tanto para clientes como para el servidor
*/
void cerrar_conexion(int conexion);

/**
* @NAME: enviar_gossiping
* @DESC: Envía una tabla de gossiping por el socket indicado.
* 		 En caso de error, retorna -1.
*/
int enviar_gossiping(int socket, gos_com_t enviar);

/**
* @NAME: enviar_request
* @DESC: Envía un request por el socket indicado.
* 		 En caso de error, retorna -1.
*/
int enviar_request(int socket, req_com_t enviar);

/**
* @NAME: enviar_error
* @DESC: Envía un error por el socket indicado.
* 		 En caso de error, retorna -1.
*/
int enviar_error(int socket, error_com_t enviar);

/**
* @NAME: enviar_handshake
* @DESC: Envía una estructura de handhsake por el socket indicado.
* 		 Si no queremos enviar un mensaje, podemos setear en 0 el tam del msg.
* 		 En caso de error, retorna -1.
*/
int enviar_handshake(int socket, handshake_com_t enviar);

/**
* @NAME: recibir_mensaje
* @DESC: Espera a recibir un mensaje por la conexión.
* 		 Nos lo devuelve conteniendo tipo de mensaje y contenido sin procesar.
* 		 Debemos procesar el mensaje según sea su tipo.
*/
msg_com_t recibir_mensaje(int conexion);

/**
* @NAME: procesar_gossiping
* @DESC: Interpreta el mensaje recibido como una tabla de gossiping y nos devuelve
* 		 una estructura gos_com_t cargada con los valores recibidos.
*/
gos_com_t procesar_gossiping(msg_com_t msg);

/**
* @NAME: procesar_request
* @DESC: Interpreta el mensaje recibido como un request y nos devuelve una estructura
* 		 req_com_t cargada con los valores recibidos.
*/
req_com_t procesar_request(msg_com_t msg);

/**
* @NAME: procesar_error
* @DESC: Interpreta el mensaje recibido como un error y nos devuelve una estructura
* 		 error_com_t cargada con los valores recibidos.
*/
error_com_t procesar_error(msg_com_t msg);

/**
* @NAME: procesar_handshake
* @DESC: Interpreta el mensaje recibido como un handshake y nos devuelve una estructura
* 		 handshake_com_t cargada con los valores recibidos.
*/
handshake_com_t procesar_handshake(msg_com_t msg);

/**
* @NAME: borrar_mensaje
* @DESC: Libera la memoria alocada en la función recibir_mensaje
*/
void borrar_mensaje(msg_com_t msg);

/**
* @NAME: borrar_buffer
* @DESC: Libera la memoria utilizada por un buffer
*/
void borrar_buffer(buffer_com_t buf);

/**
* @NAME: borrar_string
* @DESC: Libera la memoria utilizada por un string
*/
void borrar_string(str_com_t str);

/**
* @NAME: borrar_gossiping
* @DESC: Libera la memoria utilizada por una tabla de gossiping
*/
void borrar_gossiping(gos_com_t gos);

/**
* @NAME: borrar_handshake
* @DESC: Libera la memoria utilizada por un handshake
*/
void borrar_handshake(handshake_com_t hs);

int enviar_respuesta(int socket, resp_com_t enviar);
resp_com_t procesar_respuesta(msg_com_t msg);
void borrar_respuesta(resp_com_t resp);



#endif // COMUNICACION_INCLUDED

