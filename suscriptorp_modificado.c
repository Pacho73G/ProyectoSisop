/**************************************************************************
Pontificia Universidad Javeriana
Autor: Francisco Guzman, Juan Alba, Jorge Fortich,Santiago Botero.
Fecha: 11/11/24
Materia: Sistemas Operativos
Tema: Programa Suscriptor

Este programa implementa un suscriptor para un sistema de noticias 
basado en comunicación interprocesos usando pipes nombrados (FIFO).
Su objetivo es permitir que los usuarios se suscriban a temas específicos 
y reciban noticias relacionadas con esos temas a través de un canal de comunicación privado.
        
**************************************************************************/
//declaracion de las librerias en uso
#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <fcntl.h>      
#include <string.h>     
#include <sys/stat.h>   

#define BUFFER 256      // Tamaño del buffer para leer noticias

int main(int argc, char *argv[]) {
    // Verifica que el programa se haya ejecutado con exactamente 3 argumentos.
    if (argc != 3) {
        printf("Uso: %s -s pipeSSC\n", argv[0]);
        exit(1);  
    }

    // Obtiene el nombre del pipe SSC desde los argumentos.
    char *pipeSSC = argv[2];
    char nombre_fifo[100];  // Buffer para el nombre del FIFO único del suscriptor.
    int fd;  // Descriptor de archivo para el FIFO único del suscriptor.

    // Crea un nombre de FIFO único basado en el PID del suscriptor.
    snprintf(nombre_fifo, sizeof(nombre_fifo), "/tmp/fifo_suscriptor_%d", getpid());
    
    // Crea un FIFO único para el suscriptor con permisos de lectura/escritura.
    if (mkfifo(nombre_fifo, 0666) == -1) {
        printf("Error al crear el FIFO\n");
        exit(1);  // Finaliza si hay error al crear el FIFO.
    }

    // Abre el pipe SSC para enviar suscripciones al sistema.
    int fdSSC = open(pipeSSC, O_WRONLY);
    if (fdSSC == -1) {
        printf("Error al abrir pipeSSC\n");
        exit(1);  // Finaliza si no se puede abrir el pipe.
    }


            int a = 0;  // Variable para abrir el FIFO del suscriptor una única vez.
    while (1) {
        char tema[6];  // Buffer para almacenar el tema ingresado por el usuario.
        printf("Suscríbete a un tema (A, E, C, P, S) o 'exit' para terminar: ");
        fgets(tema, sizeof(tema), stdin);  // Lee la entrada del usuario.
        tema[strcspn(tema, "\n")] = '\0';  // Elimina el carácter de nueva línea al final.

        // Si el usuario escribe 'exit', termina el bucle de suscripción.
        if (strcmp(tema, "exit") == 0) {
            printf("Saliendo.... \n");
            break;  // Salir del bucle.
        } 

        // Construye la cadena de suscripción en el formato "tema FIFO".
        char subscripcion[200];
        snprintf(subscripcion, sizeof(subscripcion), "%s %s", tema, nombre_fifo);

        // Envía la suscripción al sistema de comunicación a través del pipe SSC.
        write(fdSSC, subscripcion, strlen(subscripcion) + 1);
        printf("Suscripción enviada: %s\n", subscripcion);
        
        // Abre el FIFO privado del suscriptor la primera vez que se suscribe.
        if (a == 0) {
            fd = open(nombre_fifo, O_RDONLY);  // Abre el FIFO en modo solo lectura.
            a = 1;  // Marca que el FIFO ya fue abierto.
        }
    }

    char noticias[BUFFER];  // Buffer para almacenar las noticias recibidas.

    // Bucle para leer noticias del FIFO privado.
    while (1) {
        int bytesRead = read(fd, noticias, sizeof(noticias));  // Lee del FIFO.
        if (bytesRead > 0) {
            noticias[bytesRead] = '\0';  // Asegura que la cadena sea válida.
            
            // Si recibe "exit", el sistema de comunicaciones ha terminado.
            if (strcmp(noticias, "salida") == 0) {
                printf("Notificación de salida recibida.\n");
                write(fdSSC, "salida", 7);  // Notifica al sistema la salida del suscriptor.
                break;  // Termina el bucle.
            }

            // Muestra la noticia recibida.
            printf("Noticia recibida: %s\n", noticias);
        }
    }

    // Cierra los descriptores de archivo y elimina el FIFO del suscriptor.
    close(fdSSC);  
    close(fd);     
    unlink(nombre_fifo);  
    return 0;  // Finaliza el programa exitosamente.
}



