
# 📰 Proyecto de Sistemas Operativos - Sistema de Noticias (Publicador/Suscriptor)

Este proyecto implementa un **sistema de comunicación basado en el patrón Publicador/Suscriptor** utilizando **tuberías con nombre (FIFO)** en lenguaje C. Fue desarrollado como parte del curso **Sistemas Operativos SIU4085** en la Pontificia Universidad Javeriana.

## 🧠 Descripción General

El sistema permite la interacción de tres tipos de procesos:

- **Publicadores**: Procesos que envían noticias clasificadas por tipo (Arte, Política, Ciencia, Espectáculos y Sucesos).
- **Suscriptores**: Procesos que reciben únicamente las noticias de los tópicos a los que están suscritos.
- **Sistema de Comunicación (SC)**: Encargado de recibir las noticias, hacer el "match" con las suscripciones y redirigir las noticias a los suscriptores correspondientes.

## 🔗 Comunicación entre procesos

La comunicación se realiza a través de **pipes con nombre (FIFO)**:

- `pipePSC`: Canal de comunicación de **publicadores → SC**
- `pipeSSC`: Canal de comunicación de **suscriptores → SC**

El SC también se encarga de notificar el fin de las transmisiones una vez finaliza la actividad de los publicadores.

## 🚀 Cómo ejecutar los procesos

### Sistema de Comunicación (SC)

```bash
./sistema -p pipePSC -s pipeSSC -t timeF
```

- `pipePSC`: nombre del pipe para recibir noticias.
- `pipeSSC`: nombre del pipe para recibir suscripciones.
- `timeF`: tiempo de espera adicional antes de cerrar el sistema después de que terminen los publicadores.

### Publicador

```bash
./publicador -p pipePSC -f archivo_noticias.txt -t timeN
```

- `pipePSC`: nombre del pipe para enviar noticias.
- `archivo_noticias.txt`: archivo con las noticias a enviar.
- `timeN`: tiempo entre envíos de noticias (en segundos).

### Suscriptor

```bash
./suscriptor -s pipeSSC
```

- `pipeSSC`: nombre del pipe para enviar suscripciones y recibir noticias.
- Al iniciar, solicita al usuario los tópicos a los que desea suscribirse (mínimo uno).

## 🗂 Formato del archivo de noticias

Cada línea debe tener el siguiente formato:

```
<TIPO>: <contenido de la noticia>.
```

Donde `<TIPO>` puede ser:

- `A`: Arte
- `E`: Espectáculos
- `C`: Ciencia
- `P`: Política
- `S`: Sucesos

Ejemplo:

```
P: Se aproximan las elecciones.
C: El telescopio captó un nuevo exoplaneta.
```

## ⚙️ Compilación

Utiliza el archivo `Makefile` incluido para compilar todos los procesos:

```bash
make
```

Esto generará los ejecutables: `sistema`, `publicador` y `suscriptor`.

## 📌 Requisitos del sistema

- Sistema operativo basado en Unix/Linux
- Compilador GCC
- Uso de funciones POSIX como `mkfifo()`, `open()`, `read()`, `write()`, `fork()`, etc.

---

## 📚 Créditos

Proyecto desarrollado como parte del curso de **Sistemas Operativos**  
Pontificia Universidad Javeriana – Facultad de Ingeniería

---

## 💻 Desarrollado por

**Francisco Guzmán**   
📧 franciscoguzmanv11@gmail.com  
🐙 [GitHub](https://github.com/Pacho73G)

---

 
