/*
 * Inotify.c
 *
 *  Created on: 25 jun. 2019
 *      Author: utnso
 */

/*
 * Inotify.c
 *
 *  Created on: 25 jun. 2019
 *      Author: utnso
 */

//#include "Inotify.h"
#include "memoria.h"

#include "Inotify.h"

void inotifyAutomatico(char* pathDelArchivoAEscuchar){
	int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
    while(1){

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, pathDelArchivoAEscuchar,
        IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0) {
        perror("read");
    }

    while (i < length) {
        struct inotify_event *event =
            (struct inotify_event *) &buffer[i];
        if (event->len) {
            if (event->mask && IN_CREATE) {
                printf("The file %s was created.\n", event->name);
            } else if (event->mask && IN_DELETE) {
                printf("The file %s was deleted.\n", event->name);
            } else if (event->mask && IN_MODIFY) {
                printf("The file %s was modified.\n", event->name);
            }
        }
        i += EVENT_SIZE + event->len;
    }
    printf("\nSe han realizado cambios en %s\n", pathDelArchivoAEscuchar);
    recargarConfiguracion(PATH_MEMORIA_CONFIG);
    }
    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);

    return;
}


