#include "comunicacion.h"


/* Prototipos para funciones internas a la biblioteca */

buffer_com_t serializar_gossiping(gos_com_t msg);
buffer_com_t serializar_request(req_com_t msg);
buffer_com_t serializar_error(error_com_t msg);
buffer_com_t serializar_handshake(handshake_com_t msg);
buffer_com_t serializar_respuesta(resp_com_t msg);

/* Definición de funciones internas a la biblioteca */

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

buffer_com_t serializar_error(error_com_t msg)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion=ERROR;

	//Calculo el tamaño del payload
	//El payload contiene el largo del string y el string del error
	tam_payload = sizeof(int) + msg.tam;

	//Aloco en memoria el stream
	//El stream contiene el tipo de conexión, el tamaño y el mensaje de error
	buf.tam = sizeof(conexion_t)+sizeof(int)+tam_payload;
	buf.stream = malloc(buf.tam);

	//Ahora cargo el stream con los datos que necesito

	//Primero el tipo
	memcpy(buf.stream+desp,&tipo_conexion,sizeof(conexion_t));
	desp += sizeof(conexion_t);

	//Ahora el tamaño
	memcpy(buf.stream+desp,&tam_payload,sizeof(int));
	desp += sizeof(int);

	//Ahora el largo del string
	memcpy(buf.stream+desp,&msg.tam,sizeof(int));
	desp += sizeof(int);

	//Después el error
	memcpy(buf.stream+desp,msg.str,msg.tam);
	desp += tam_payload;
	return buf;
}

buffer_com_t serializar_handshake(handshake_com_t msg)
{
	buffer_com_t buf;
	int desp=0, tam_payload;
	conexion_t tipo_conexion=HANDSHAKE;

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

	//Después el request
	memcpy(buf.stream+desp,resp.msg.str,resp.msg.tam);
	desp += tam_payload;
	return buf;
}


/* Definición de funciones pública de la biblioteca */

int iniciar_servidor(char*ip,char*puerto)
{
	int socket_servidor;
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
    printf("\n**Servidor listo para escuchar al cliente**\n");
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

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		//printf("\n**No se pudo establecer conexion con el servidor**\n");
		return -1;
    }

	freeaddrinfo(server_info);

	printf("\nMe conecté!");
	
	//ENVIAR HANDSHAKE CON TIPO CLIENTE
	handshake_com_t hs;
	hs.id = id;
	
	//No me interesa enviar ningún mensaje de presentación
	hs.msg.tam = 0;
	
	enviar_handshake(socket_cliente, hs);
	borrar_handshake(hs);
	
	printf("\nMe presenté!");
	return socket_cliente;
}

cliente_com_t esperar_cliente(int servidor)
{
	cliente_com_t cliente;
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);
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

void cerrar_conexion(int conexion)
{
	close(conexion);
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

int enviar_error(int socket, error_com_t enviar)
{
	int retval = 0;
	buffer_com_t serializado;
	serializado = serializar_error(enviar);
	if(send(socket, serializado.stream, serializado.tam, 0)==-1)
		retval = -1;
	borrar_buffer(serializado);
	return retval;
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

msg_com_t recibir_mensaje(int conexion)
{
	msg_com_t recibido;

	//Primero recibo el tipo
	if(recv(conexion, &(recibido.tipo), sizeof(conexion_t), MSG_WAITALL) == 0){
		//printf("\nError al recibir el mensaje");
		recibido.tipo = DESCONECTADO;
		return recibido;
	}

	//Ahora recibo el tamaño
	if(recv(conexion, &(recibido.payload.tam), sizeof(int), MSG_WAITALL) == 0){
			//printf("\nError al recibir el mensaje");
			recibido.tipo = DESCONECTADO;
			return recibido;
	}

	//Ahora aloco en memoria el stream
	recibido.payload.stream = malloc(recibido.payload.tam);

	//Ahora recibo el payload de tamaño ya conocido
	if(recv(conexion, recibido.payload.stream, recibido.payload.tam, MSG_WAITALL) == 0){
			//printf("\nError al recibir el mensaje");
			recibido.tipo = DESCONECTADO;
			return recibido;
	}

	return recibido;
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

error_com_t procesar_error(msg_com_t msg)
{
	error_com_t error;
	int desp = 0;

	//Tengo que desempaquetar el stream

	//Primero tengo el largo del string
	memcpy(&error.tam, msg.payload.stream+desp, sizeof(int));
	desp += sizeof(int);

	//Ahora ya sé el largo del string
	error.str = malloc(error.tam);

	memcpy(error.str, msg.payload.stream+desp, error.tam);

	return error;
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
	return resp;

}

void borrar_mensaje(msg_com_t msg)
{
	borrar_buffer(msg.payload);
}

void borrar_buffer(buffer_com_t buf)
{
	if(buf.tam>0)
		free(buf.stream);
}

void borrar_string(str_com_t str)
{
	if(str.tam > 0)
		free(str.str);
}

void borrar_gossiping(gos_com_t gos)
{
	free(gos.seeds);
}

void borrar_handshake(handshake_com_t hs)
{
	borrar_string(hs.msg);
}

void borrar_respuesta(resp_com_t resp)
{
	if(resp.msg.tam > 0)
		free(resp.msg.str);
}
