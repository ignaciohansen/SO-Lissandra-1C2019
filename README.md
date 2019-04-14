# Práctico: Mi ultimo segundo TP. 
Carrera:      Ing. en Sistemas de Información
Materia:     Sistemas Operativos 
Ayudante: Maxi Felice

# Kernel

# LFS

# "Pool de Memoria"
Iniciado: Domingo 07/04/2019

Se espera realización de memorias funcionando en paralelo basadas en pthread.

 (*) La parte de sockets está comentada.
 (*) Está en desarrollo la interpretación de comandos.
 (*) ¿va a ser necesario lanzar (la interfaz de la memoria por) consola y sockets a la vez? 
   -   ¿es decir, se usa hilos acá? 

Requerimientos:
1) Interpretación de Queries-LQL para el modulo Pool de Memorias.

2) Comunicación con kernel. Obtener: valor de tam. máx. de value para paginación.
//COMUNICACION HECHA, FALTA SOLO SERIALIZAR LOS MENSAJES

3) Administración de tabla de páginas. (Próximas entregas)
4) Memoria principal: funcionando sobre memoria contigua... (heavy o se desaprueba)

 (*) Requerimiento de cátedra: 
   -    Inicializar TODO una sola vez. (Usar una sola vez el MALLOC)

DUDAS: 13/04/2019
POOL DE MEMORIAS:
	
EL pool de memorias en el archivo config de Memoria tienen sus propias IPs y tambien puertos. 
A todo esto, significa que tendran que correr como procesos apartes (Modulos)?


MULTITHREADS

Uso de pthread_join pero mejor usar pthread_detacth


# Biblioteca