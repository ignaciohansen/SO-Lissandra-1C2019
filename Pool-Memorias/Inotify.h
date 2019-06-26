/*
 * Inotify.h
 *
 *  Created on: 25 jun. 2019
 *      Author: utnso
 */

#ifndef INOTIFY_H_
#define INOTIFY_H_

#include <sys/inotify.h>

//void inotifyAutomatico(char* pathDelArchivoAEscuchar);

// El tamaño de un evento es igual al tamaño de la estructura de inotify
// mas el tamaño maximo de nombre de archivo que nosotros soportemos
// en este caso el tamaño de nombre maximo que vamos a manejar es de 24
// caracteres. Esto es porque la estructura inotify_event tiene un array
// sin dimension ( Ver C-Talks I - ANSI C ).
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )

// El tamaño del buffer es igual a la cantidad maxima de eventos simultaneos
// que quiero manejar por el tamaño de cada uno de los eventos. En este caso
// Puedo manejar hasta 1024 eventos simultaneos.
#define BUF_LEN     ( 1024 * EVENT_SIZE )

void inotifyAutomatico(char* pathDelArchivoAEscuchar);

#endif /* INOTIFY_H_ */
