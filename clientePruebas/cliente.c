#include "comunicacion.h"

void * enviarLoQueEscribo(int * socket_p);

#define TEST_CLIENTE
#ifdef TEST_CLIENTE

//Recibe IP y PUERTO del servidor como argumentos. Se intenta conectar. Cuando lo logra envia y recibe mensajes
int main(int argc, char **argv)
{
    id_com_t soy = KERNEL;
	char*ip,*puerto;
    int conexion;
    pthread_t thread_enviar;

    if(argc<3){
    	printf("\nSe deben indicar como argumentos del main ip y puerto");
    }

    ip = argv[1];
    puerto = argv[2];

    printf("\n**Preparando para conectarse a puerto: %s. Ip: %s**\n",puerto,ip);

    //Me conecto al servidor
    do{
        printf("\n**Intentando conectarse al servidor...**\n");
        conexion = conectar_a_servidor(ip,puerto,soy);
        if(conexion == -1){
            printf("\n*No se pudo establecer conexion con el servidor. Volviendo a intentar en 5 segundos...**\n");
            sleep(5);
        }
    }while(conexion == -1);

    printf("\nConexion establecida!");

    //Espero el handshake del servidor
    msg_com_t msg = recibir_mensaje(conexion);

    //Si me llegó algo que no es handshake, no sigo
    if(msg.tipo != HANDSHAKE){
    	printf("\nEl servidor no responde como se espera...\n\n");
    	borrar_mensaje(msg);
    }
    else{
    	handshake_com_t hs = procesar_handshake(msg);
    	if(hs.id == RECHAZADO){
    		//Si el servidor rechazó mi conexión, no sigo
    		printf("\nEl servidor rechazó la conexion: %s\n\n",hs.msg.str);
    		borrar_handshake(hs);
    	}
    	else{
    		//Si el servidor me recibió, comienzo a enviarle 'cosas'
    		printf("\nEl servidor nos recibió: %s\n\n",hs.msg.str);
    		borrar_handshake(hs);
    		pthread_create(&thread_enviar,NULL,(void*)enviarLoQueEscribo,(void *)&conexion);
    		pthread_join(thread_enviar,NULL);
    	}
    }
    printf("\n\n");

    //Cierro la conexión
    cerrar_conexion(conexion);

    return 0;
}

void * enviarLoQueEscribo(int * socket_p)
{
    int conexion = *socket_p;
    char *linea;
    req_com_t req;
    resp_com_t resp;
    msg_com_t msg;

    while(1){
    	linea = readline("\n>");
    	if(!strcmp(linea,"FIN"))
    		break;
    	req.tam = strlen(linea)+1;
    	req.str = malloc(req.tam);
    	strcpy(req.str,linea);
    	free(linea);
    	if(enviar_request(conexion,req) == -1){
    		printf("\n***Servidor desconectado***");
    		borrar_string(req);
    		break;
    	}
    	borrar_string(req);
        printf("\n***Esperando respuesta");
        printf("***");
        printf("\n");
    	msg = recibir_mensaje(conexion);
    	if(msg.tipo == RESPUESTA){
    		resp = procesar_respuesta(msg);
    		if(resp.msg.tam > 0)
    			printf("--->%s",resp.msg.str);
    		else
    			printf("--->S/R");
    		borrar_respuesta(resp);
    	}
        else if(msg.tipo == DESCONECTADO){
            printf("\n***Servidor desconectado***");
        	borrar_mensaje(msg);
    		break;
        }
    	borrar_mensaje(msg);
    }
    printf("\n\n**Finalizando...**\n");
}

void * enviarSarasa(int * socket_p)
{
    int conexion = *socket_p;
    char *msg;

    //Creo estructuras de ejemplo con datos random


    //Una para el gossiping
    gos_com_t gossip;
    gossip.cant = 3;
	gossip.seeds = malloc(sizeof(seed_com_t)*gossip.cant);

	//Le cargo datos de 3 memorias
	gossip.seeds[0].numMemoria = 1;
	strcpy(gossip.seeds[0].ip,"127.0.0.1");
	strcpy(gossip.seeds[0].puerto,"4040");

	gossip.seeds[1].numMemoria = 2;
	strcpy(gossip.seeds[1].ip,"127.0.0.3");
	strcpy(gossip.seeds[1].puerto,"4045");

	gossip.seeds[2].numMemoria = 3;
	strcpy(gossip.seeds[2].ip,"127.0.0.6");
	strcpy(gossip.seeds[2].puerto,"4444");


	//Una para un request
	req_com_t req;
	req.tam = 25;
	req.str = malloc(req.tam);
	strcpy(req.str, "SELECT TABLA1 2212");


	//Una para un mensaje de error
	error_com_t error;
	error.tam = 40;
	error.str = malloc(error.tam);
	strcpy(error.str, "La tabla 'TABLA32' no existe!");


	//Una para un handshake
	handshake_com_t hs;
	id_com_t id;
	hs.id = MEMORIA;
	hs.msg.tam = 30;
	hs.msg.str = malloc(hs.msg.tam);
	strcpy(hs.msg.str, "Bienvenido!");


    //Hago un bucle infinito que envía los distintos tipos de mensaje
    int tipo_msg = 0;
    int fin = 0;
    while(!fin){
    	switch(tipo_msg){
    		case 0:
    			printf("\n-Envío tabla de gossiping");
    			if(enviar_gossiping(conexion,gossip)==-1)
    				fin = 1;
    			break;
    		case 1:
    			printf("\n-Envío request");
    			if(enviar_request(conexion,req)==-1)
    				fin = 1;
    			break;
    		case 2:
    			printf("\n-Envío mensaje de error");
    			if(enviar_error(conexion,error)==-1)
    				fin = 1;
    			break;
    		case 3:
    			//No tiene sentido enviar handshake ahora, sólo es para probar los tipos de mensaje
    			printf("\n-Envío handshake");
    			if(enviar_handshake(conexion,hs)==-1)
    				fin = 1;
    			break;
    	}
    	printf("\n");
    	tipo_msg ++;
    	if(tipo_msg == 4) tipo_msg = 0;
    	sleep(3);
    }
    printf("\n\n**El servidor se desconecto. Finalizando...**\n");
	
    //Libero la memoria que pedí
    borrar_gossiping(gossip);
    borrar_string(req);
    borrar_string(error);
    borrar_handshake(hs);

}
#endif
