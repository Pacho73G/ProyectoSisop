/**************************************************************************
Pontificia Universidad Javeriana
Autor: Juan Alba, Jorge Fortich, Francisco Guzman, Santiago Botero.
Fecha: 11/11/24
Materia: Sistemas Operativos
Tema: Programa Sistema

Este programa implementa un sistema de publicación-suscripción utilizando pipes FIFO para la comunicación entre los diferentes componentes del sistema:
publicadores y suscriptores. El sistema permite que un publicador envíe noticias sobre ciertos temas a suscriptores que se han registrado para recibir noticias sobre esos temas.
Los suscriptores se suscriben a temas específicos, y cuando una noticia relacionada con uno de esos temas es publicada, se distribuye automáticamente a todos los suscriptores interesados.
        
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>  

// Definiciones y constantes
#define BUFFER 256               // Tamaño máximo del buffer para almacenar mensajes o datos
#define MAX_SUBSCRIPTORES 10     // Número máximo de suscriptores que pueden registrarse en el sistema
#define MAX_TEMAS 5              // Número máximo de temas a los que un suscriptor puede estar suscrito

// Estructura para la configuración del sistema
typedef struct {
    char* pipePSC; // Nombre del pipe donde el publicador envía las noticias
    char* pipeSSC; // Nombre del pipe donde los suscriptores envían sus suscripciones
    int timeF;     // Tiempo máximo de espera antes de cerrar el sistema si no hay noticias
} sistema_config;

// Estructura para representar a cada suscriptor
typedef struct {
    char temas[MAX_TEMAS];       // Lista de temas a los que el suscriptor está suscrito (hasta 5 temas)
    int contador_temas;          // Número actual de temas a los que está suscrito
    char nombre_pipe[BUFFER];    // Nombre del pipe a través del cual el suscriptor recibe noticias
    int fd;                      // Descriptor de archivo para el pipe del suscriptor (conexión al suscriptor)
} subscriptor;

subscriptor subscriptores[MAX_SUBSCRIPTORES];  // Arreglo para almacenar la información de todos los suscriptores
int contador_suscriptores = 0;                 // Contador que mantiene el número actual de suscriptores registrados

/*
  Nombre: procesar_subscriptores
  Argumentos: arg que es la configuracion del sistema que contiene
    - pipeSSC: Un descriptor de archivo para la comunicación con el suscriptor.
    - pipePSC: Un descriptor de archivo para la comunicación con el publicador.
    - timeF: El tiempo que espera el sistema a otro publicador.

  Descripción: 
    Esta función se encarga de procesar las solicitudes de suscripción que llegan a través del pipe de suscriptores. 
    Lee las suscripciones y las registra en la estructura `suscriptores`. Verifica si el suscriptor ya está registrado 
    y si su suscripción a un tema es válida.
  
*/

void* procesar_subscriptores(void* arg) {
    // Recibimos la configuración del sistema como argumento
    sistema_config* config = (sistema_config*)arg;

    // Abrimos el pipe donde los suscriptores envían sus suscripciones
    int fdSSC = open(config->pipeSSC, O_RDONLY);
    if (fdSSC == -1) {
        // Si no se pudo abrir el pipe, mostramos un error y salimos
        printf("Error abriendo pipe de suscripciones\n");
        return NULL;
    }

    char subscripcion[BUFFER]; // Buffer para almacenar la información de cada suscripción
    while (1) {
        // Leemos las suscripciones de los suscriptores desde el pipe
     read(fdSSC, subscripcion, sizeof(subscripcion));
        
        // Si no hay datos en el pipe, seguimos esperando
        subscripcion[strcspn(subscripcion, "\n")] = '\0'; // Eliminamos el salto de línea al final de la cadena
        if (strcmp(subscripcion, "salida") == 0) {
            // Si recibimos el comando "salida", terminamos el proceso de suscripción
            break;
        }
        
        printf("Suscripción recibida: %s\n", subscripcion);

        char tema;
        char nombre_pipe[BUFFER];
        // Parseamos la suscripción, que tiene el formato: "Tema Nombre_pipe"
        sscanf(subscripcion, "%c %s", &tema, nombre_pipe);

        // Validamos si el tema es válido (A, E, C, P, S)
        if (tema != 'A' && tema != 'E' && tema != 'C' && tema != 'P' && tema != 'S') {
            printf("El tema: %c no existe\n", tema);
            continue; // Si el tema no es válido, seguimos con la siguiente suscripción
        }

        // Verificamos si el suscriptor ya está registrado en el sistema
        bool encontrado = false;
        for (int i = 0; i < contador_suscriptores; i++) {
            if (strcmp(subscriptores[i].nombre_pipe, nombre_pipe) == 0) {
                encontrado = true;
                // Si el suscriptor ya está registrado, verificamos si ya está suscrito a este tema
                if (subscriptores[i].contador_temas < MAX_TEMAS) {
                    bool existe = false;
                    for (int j = 0; j < subscriptores[i].contador_temas; j++) {
                        if (subscriptores[i].temas[j] == tema) {
                            existe = true; // El suscriptor ya está suscrito a este tema
                            break;
                        }
                    }
                    // Si no está suscrito al tema, lo agregamos
                    if (!existe) {
                        subscriptores[i].temas[subscriptores[i].contador_temas++] = tema;
                        printf("Nuevo tema agregado: %c para el suscriptor %s\n", tema, nombre_pipe);
                    } else {
                        printf("El suscriptor %s ya está suscrito al tema %c\n", nombre_pipe, tema);
                    }
                }
                break;
            }
        }

        // Si el suscriptor no está registrado, lo registramos como nuevo
        if (!encontrado && contador_suscriptores < MAX_SUBSCRIPTORES) {
            strcpy(subscriptores[contador_suscriptores].nombre_pipe, nombre_pipe);
            subscriptores[contador_suscriptores].temas[0] = tema;
            subscriptores[contador_suscriptores].contador_temas = 1;
            // Abrimos el pipe de escritura para el nuevo suscriptor
            subscriptores[contador_suscriptores].fd = open(nombre_pipe, O_WRONLY);
            if (subscriptores[contador_suscriptores].fd == -1) {
                printf("Error abriendo pipe del suscriptor\n");
                continue;
            }
            contador_suscriptores++;
            printf("Nuevo suscriptor registrado: %s con tema %c\n", nombre_pipe, tema);
        }
    }

    // Cerramos el pipe de suscripciones al finalizar
    close(fdSSC);
    return NULL;
}
/*
  Nombre: procesar_noticias
    Argumentos: arg que es la configuracion del sistema que contiene
    - pipeSSC: Un descriptor de archivo para la comunicación con el suscriptor.
    - pipePSC: Un descriptor de archivo para la comunicación con el publicador.
    - timeF: El tiempo que espera el sistema a otro publicador.
  Descripción: 
    Esta función lee las noticias de un publicador a través del pipe `pipePSC`. Dependiendo del tema de la noticia, 
    se distribuye a los suscriptores correspondientes. La función también se asegura de que solo se envíen noticias a los 
    suscriptores que están interesados en ese tema.
*/

void* prosesar_noticias(void* arg) {
    sistema_config *config = (sistema_config *)arg;

    // Abrimos el pipe donde los publicadores envían las noticias
    int ppsc_fd = open(config->pipePSC, O_RDONLY);
    if (ppsc_fd == -1) {
        // Si no se puede abrir el pipe de noticias, mostramos un error y salimos
        printf("Error abriendo pipe de publicadores\n");
        return NULL;
    }

    char buffer[BUFFER]; // Buffer para almacenar las noticias recibidas
    while (1) {
        // Leemos una noticia del pipe
        int bytesRead = read(ppsc_fd, buffer, sizeof(buffer));
        
        // Si no hay datos, esperamos por un tiempo antes de cerrar el sistema
        if (bytesRead == 0) {
            // Si no se recibe una noticia, esperamos el tiempo especificado en la configuración
            for (int h = 0; h < config->timeF; h++) {
                sleep(1);
                bytesRead = read(ppsc_fd, buffer, sizeof(buffer));
                if (bytesRead > 0) break;
            }
            if (bytesRead == 0) {
                printf("La emisión ha terminado\n");
                break;
            }
        }
        printf("Recibida la noticia '%s'.\n", buffer);
        // Eliminamos el salto de línea al final de la cadena
        buffer[strcspn(buffer, "\n")] = '\0';

        // El primer carácter de la noticia es el tema
        char tema = buffer[0];

        // Enviamos la noticia a los suscriptores que están suscritos a este tema
        for (int i = 0; i < contador_suscriptores; i++) {
            for (int j = 0; j < subscriptores[i].contador_temas; j++) {
                if (subscriptores[i].temas[j] == tema) {
                    write(subscriptores[i].fd, buffer, strlen(buffer) + 1);  // Enviamos la noticia al suscriptor
                    printf("Enviando noticia '%s' al suscriptor del tema '%c'\n", buffer, tema);
                }
            }
        }
    }

    // Al finalizar, notificamos a todos los suscriptores que el sistema está cerrando
    for (int i = 0; i < contador_suscriptores; i++) {
        write(subscriptores[i].fd, "salida", 7);  // Enviamos el mensaje "salida"
    }
    // Cerramos el pipe de noticias
    close(ppsc_fd);
    return NULL;
}
/*
  Nombre: cerrar_sistema
  Argumentos: 
    -argc: la cantidad de comandos en la linea de comandos
    -argv: un array con los comandos de la linea
  Descripción: 
    Esta funcion se encarga de procesar los argumentos llenando la estructura de la configuracion con los datos que se insertaron en el comando de ejecucion.
*/

void procesar_argumentos(int argc, char *argv[], sistema_config *config) {
    if (argc < 7) {
        // Si no se pasan los argumentos correctos, mostramos cómo debe ejecutarse el programa
        fprintf(stderr, "Uso: %s -p pipePSC -s pipeSSC -t timeF\n", argv[0]);
        exit(1);  // Salimos del programa con código de error
    }

    // Iteramos sobre los argumentos y los procesamos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            config->pipePSC = argv[++i];  // Asignamos el nombre del pipe para publicadores
        } else if (strcmp(argv[i], "-s") == 0) {
            config->pipeSSC = argv[++i];  // Asignamos el nombre del pipe para suscriptores
        } else if (strcmp(argv[i], "-t") == 0) {
            config->timeF = atoi(argv[++i]);  // Asignamos el tiempo máximo de espera
        }
    }
}

int main(int argc, char *argv[]) {
  // Estructura para almacenar la configuración del sistema
    sistema_config config = {NULL, NULL, 0};
  // Procesamos los argumentos de la línea de comandos
    procesar_argumentos(argc, argv, &config);
    // Creamos el archivo FIFO (pipe) para el publicador y suscriptores
    if(mkfifo(config.pipePSC, 0666) == -1) {
        perror("Error creando pipe de publicadores");
        exit(1);
    }
    if (mkfifo(config.pipeSSC, 0666) == -1) {
        perror("Error creando pipe de suscriptores");
        exit(1);
    }
           
    printf("Configuración del sistema:\n");
    printf("Pipe de publicadores: %s\n", config.pipePSC);
    printf("Pipe de suscriptores: %s\n", config.pipeSSC);
    printf("Tiempo de espera: %d\n", config.timeF);
    
    // Creamos los hilos para procesar las suscripciones y las noticias
    pthread_t hilo_subs, hilo_noticias;
    pthread_create(&hilo_subs, NULL, procesar_subscriptores, &config);
    pthread_create(&hilo_noticias, NULL, prosesar_noticias, &config);

    // Esperamos a que los hilos terminen su ejecución
    pthread_join(hilo_subs, NULL);
    pthread_join(hilo_noticias, NULL);
    
    unlink(config.pipePSC);
    unlink(config.pipeSSC);
    return 0;
}
