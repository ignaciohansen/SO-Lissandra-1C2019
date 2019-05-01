# Práctico: Mi ultimo segundo TP. 
Carrera:      Ing. en Sistemas de Información
Materia:     Sistemas Operativos 
Ayudante: Maxi Felice

# Kernel

# LFS Sistema de archivos. Persistencia de Datos.

 - Tiene la función de recepción de queries funcionando. Recibe cualquier query.
 - Falta que sea capaz de procesarla.

 (*) Se le pueden enviar queries usando el proyecto: Consola Remota LFS.

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



# Biblioteca