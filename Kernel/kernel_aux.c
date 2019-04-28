/*
 * kernel_aux.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "kernel_aux.h"

void mostrarQueries(t_log* logger, t_list* queriesList){

  int listSize = list_size(queriesList);
  int cursor   = 0;

  log_info(logger, "\n\n ========== LISTANDO COMANDOS LEIDOS (BEGIN) ========== \n\n ");

  cursor += 1;
  while ( cursor <= listSize ) {
	  char* query   = list_get( queriesList , cursor );
	  //char* aLogear = "Comando enlistado para ejecución: ";
	  //string_append(aLogear,query );
	  log_info(logger, "Comando enlistado para ejecución: %s.", query);
	  free(query);
	  cursor += 1;
  } // while ( cursor <= listSize )

  log_info(logger, "\n\n ========== LISTANDO COMANDOS LEIDOS (END) ========== \n\n ");

}
