/*
 * kernel.c
 *
 *  Created on: 4 abr. 2019
 *      Author: utnso
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
    log_memoria = archivoLogCrear(LOG_PATH, "Proceso Memoria");
    log_info(log_memoria, " \n ========== Iniciación de Pool de Memoria ========== \n \n ");

    log_info(log_memoria, "(1) LOG CREADO. ");

    cargarConfiguracion();
    log_info(log_memoria, " *** CONFIGURACIÓN DE MEMORIA CARGADA. *** ");

    // SECCIÓN DE CONSOLA.
    // Operaciones: SELECT, INSERT, CREATE, DROP, DESCRIBE.

    String    comando;
    comando = lectura_consola();
    log_info(log_memoria,"Se lee de consola la línea: "); log_info(log_memoria,comando);

    stringRemoverVaciosIzquierda(&comando); // depurado
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
        buffer = malloc( 10 * sizeof(char) );
        recibiendoMensaje = socketRecibir(conexionEntrante,buffer,10);
        printf("Recibimos por socket %s",buffer);
        log_info(log_memoria,"El mensaje que se recibio fue %s", buffer);
    }
    // FIN DE BLOQUE DE RED.
    */

    log_info(log_memoria, "FIN DE PROCESO MEMORIA");

    log_destroy   (log_memoria);
    free          (log_memoria);
    log_memoria  = NULL        ;

    return 0;

} // MAIN.

char* lectura_consola() {
    char* linea = (char*)readline(">");
    return linea;
}

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
