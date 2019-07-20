/*
 * Gossiping.h
 *
 *  Created on: 21 jun. 2019
 *      Author: martin
 */

#include "Biblioteca.h"

#ifndef GOSSIPING_H_
#define GOSSIPING_H_

typedef uint64_t time_gos_t;

typedef struct{
	id_com_t id_proceso;
	void (*funcion) (void);
}thread_gos_args_t;

t_list* lista_seeds(void);
void inicializar_estructuras_gossiping(t_log *logger, time_gos_t retardo);
void actualizar_retardo_gossiping(time_gos_t retardo);
void liberar_memoria_gossiping(void);
void agregar_seed(int nro_mem, char* ip, char *puerto);
int conozco_memoria(seed_com_t memoria);
void incorporar_seeds_gossiping(gos_com_t nuevas);
gos_com_t armar_vector_seeds(id_com_t id_proceso);
void borrar_seed(seed_com_t *memoria);
void *hilo_gossiping(thread_gos_args_t *args);
int iniciar_hilo_gossiping(id_com_t *id_proceso, pthread_t *thread, void (*funcion_actualizacion) (void));
t_list *hayMemoriasCaidas(void);
bool huboCambios(void);


#endif /* GOSSIPING_H_ */
