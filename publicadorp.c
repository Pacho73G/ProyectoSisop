/**************************************************************************
Pontificia Universidad Javeriana
Autor: Juan Alba, Jorge Fortich, Francisco Guzman, Santiago Botero.
Fecha: 11/11/24
Materia: Sistemas Operativos
Tema: Programa publicador

Este programa es un publicador que lee un archivo de noticias, valida su formato y las envía a un sistema 
de comunicaciones (SC) a través de un pipe. Si el formato de la noticia no es correcto, muestra un mensaje de error
indicando la línea incorrecta. Además, se introduce un retraso entre el envío de cada noticia.
        
**************************************************************************/
//declaracion de las librerias en uso
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define BUFFER 256 // Tamaño del buffer para leer líneas del archivo

/*
  Nombre:verificar_formato.
  Argumentos:linea cadena de caracteres (const char*), que representa una línea de texto que se leerá del archivo de noticias.
  Descripcion: la funcion recibe una linea del archivo de noticias, y sobre esta verifica el formato.
  Return: 0 si no cumple el formato y 1 si lo cumple.

*/
int verificar_formato(char* linea ) {
    // Eliminar espacios al principio de la línea
    while (isspace(*linea)) linea++; 

    // Calcular la longitud de la línea para eliminar espacios al final
    size_t len = strlen(linea);
    while (len > 0 && isspace(linea[len - 1])) {
        len--;
    }

    // Crear un buffer temporal para almacenar la línea sin espacios al inicio y al final
    char buffer[len + 1];
    strncpy(buffer, linea, len);
    buffer[len] = '\0'; // Terminar la línea con un carácter nulo

    // Verificar que la línea tenga al menos 3 caracteres (tipo, ':', texto)
    if (strlen(buffer) < 3) return 0;

    // Comprobar que el primer carácter sea un tipo válido (P, A, S, C, E)
    if (buffer[0] != 'P' && buffer[0] != 'A' && buffer[0] != 'S' && buffer[0] != 'C' && buffer[0] != 'E') {
        return 0; // Retorna 0 si el tipo es inválido
    }

    // Verificar que el segundo carácter sea ':'
    if (buffer[1] != ':') {
        return 0; // Retorna 0 si no hay un ':' después del tipo
    }

    // Verificar que la línea termine con un punto '.'
    if (buffer[len - 1] != '.') {
        return 0; // Retorna 0 si la línea no termina en '.'
    }

    return 1; // Retorna 1 si el formato es válido
}

int main(int argc, char* argv[]) {
    // Verificar que el número de argumentos sea el correcto
    if (argc != 7) {
        printf("Uso: %s -p pipePSC -f file -t timeN\n", argv[0]);
        exit(1);
    }

    // Abrir el pipe en modo escritura
    int fdPSC = open(argv[2], O_WRONLY);
    if (fdPSC == -1) {
        printf("Error al abrir el pipe\n");
        exit(1);
    }

    // Abrir el archivo de noticias en modo lectura
    FILE* archivo = fopen(argv[4], "r");
    if (!archivo) {
        printf("Error al abrir el archivo\n");
        close(fdPSC); // Cerrar el pipe antes de salir
        exit(1);
    }

    // Convertir el tiempo entre envíos (timeN) de cadena a entero
    int timeN = atoi(argv[6]);
    char buffer[BUFFER]; // Buffer para almacenar las líneas leídas del archivo

    // Leer cada línea del archivo
    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        // Verificar el formato de la línea leída
        if (verificar_formato(buffer) == 1) {
            // Si el formato es válido, enviar la línea al pipe
            write(fdPSC, buffer, strlen(buffer) + 1); // +1 para incluir el carácter nulo
            sleep(timeN); // Esperar el tiempo especificado antes de enviar la siguiente noticia
        } else {
            // Si la línea no es válida, imprimir un mensaje de error
            printf("Línea no válida: '%s'\n", buffer);
        }
    }

    // Cerrar el archivo de noticias y el pipe
    fclose(archivo);
    close(fdPSC);

    return 0; // Salida exitosa
}

