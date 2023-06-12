# WebServer
A basic FTP Server

## Integrantes (C-312):
Claudia Alvarez Martínez

Roger Moreno Gutiérrez

## Instrucciones de ejecución:
Abrir la terminal en el directorio raiz del proyecto y ejecutar el siguiente comando:

```bash
./m <PORT> <PATH>
```

donde `<PORT>` representa el puerto se conexión y `<PATH>` la ruta local que se desea mostrar.



De manera opcional se puede ejecutar el comando:

```bash
./m <PORT> <PATH> -s
```

donde el flag `-s` indica que se desea saber exactamente el tamaño de cada directorio
(Opcional porque lo que hace es recursividad por los directorios calculando el tamaño de sus archivos, por tanto en directorios como C:\, la recursividad se hace tardía)

