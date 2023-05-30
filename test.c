#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
#define RED_COLOR "\033[0;31m"
#define DEFAULT_COLOR "\033[0m"

void handle_client(int client_socket, char* ROOT_DIR);

int main(int argc, char *argv[])
{
    int PORT = atoi(argv[1]);
    char* ROOT_DIR = argv[2];

    if(PORT == 0) { fprintf(stderr, RED_COLOR "Error: invalid Port\n" DEFAULT_COLOR); exit(1); }
    DIR* dir = opendir(argv[2]);
    if (dir == NULL) { fprintf(stderr, RED_COLOR "Error: invalid Directory Root\n" DEFAULT_COLOR); exit(1); }
    closedir(dir);

    // printf("argc = %i\n", argc);
    printf("Port = %i\n", PORT);
    printf("Root = %s\n", ROOT_DIR);




    // Desde aquí hasta abajo me los dión Chat-GPT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error al crear socket");
        exit(1);
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket al puerto
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error en bind");
        exit(1);
    }

    // Escuchar por conexiones entrantes
    if (listen(server_socket, 5) < 0)
    {
        perror("Error en listen");
        exit(1);
    }

    printf("Servidor FTP en ejecución...\n");

    while (1)
    {
        // Aceptar una nueva conexión de cliente
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            perror("Error en accept");
            exit(1);
        }

        printf("Nueva conexión aceptada\n");

        // Manejar al cliente en un proceso hijo
        if (fork() == 0)
        {
            close(server_socket);
            handle_client(client_socket, ROOT_DIR);
            exit(0);
        }

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

void handle_client(int client_socket, char* ROOT_DIR)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Enviar mensaje de bienvenida al cliente
    strcpy(buffer, "220 Bienvenido al servidor FTP\r\n");
    send(client_socket, buffer, strlen(buffer), 0);

    // Recibir comandos del cliente
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        // Eliminar caracteres de nueva línea
        buffer[bytes_received - 2] = '\0';

        printf("Comando recibido: %s\n", buffer);

        // Procesar comandos del cliente
        if (strncmp(buffer, "USER", 4) == 0)
        {
            // Autenticar al usuario (puedes agregar tu propia lógica de autenticación)
            strcpy(buffer, "331 Usuario válido, proporcione la contraseña\r\n");
        }
        else if (strncmp(buffer, "PASS", 4) == 0)
        {
            // Verificar la contraseña (puedes agregar tu propia lógica de autenticación)
            strcpy(buffer, "230 Autenticación exitosa\r\n");
        }
        else if (strncmp(buffer, "GET", 3) == 0)
        {

            // ~~~~~~~🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨~~~~~~~
            // Lo que hay en este if es cosa que me puse a trastear yo
            // lo que me dió GPT fue una línea como la que hay en los otros if's
            // el nombre GET se lo dí yo, no era una opción que me salio por GPT
            // Entonces si pongo en el navegador localhost:<PORT> hace un GET y entra en este if
            // Lo que hay aqu'i que yo hice no sirve para nada

            // Abrir el directorio y leer su contenido
            DIR *dir;
            struct dirent *entry;
            dir = opendir(ROOT_DIR);

            if (dir == NULL)
            {
                strcpy(buffer, "550 Error al abrir el directorio\r\n");
            }
            else
            {
                // Construir una página HTML con la lista de archivos
                char html_buffer[BUFFER_SIZE];
                strcpy(html_buffer, "<html><body><ul>");
                while ((entry = readdir(dir)) != NULL)
                {
                    strcat(html_buffer, "<li>");
                    strcat(html_buffer, entry->d_name);
                    strcat(html_buffer, "</li>");
                }
                strcat(html_buffer, "</ul></body></html>\r\n");

                // Guardar la página HTML en un archivo
                FILE *html_file = fopen("content.html", "w");
                if (html_file != NULL)
                {
                    fprintf(html_file, "%s", html_buffer);
                    fclose(html_file);
                }
                else
                {
                    strcpy(buffer, "550 Error al crear el archivo HTML\r\n");
                }
            }


            // Ruta al archivo HTML
            const char *html_file = "\\C:\\SO\\WebServer\\content.html";

            // Comando para abrir el archivo con el navegador
            // const char *command = "xdg-open";  // Comando para Linux, puede variar según el sistema operativo
            // const char *command = "open";   // Comando para macOS
            const char *command = "start";  // Comando para Windows

            // Construir el comando completo
            char full_command[256];
            snprintf(full_command, sizeof(full_command), "%s %s", command, html_file);

            // Ejecutar el comando
            int result = system(full_command);
            if (result == -1)
            {
                printf("Error al abrir el archivo con el navegador.\n");
            }



            
            
        }
        else if (strncmp(buffer, "RETR", 4) == 0)
        {
            // Enviar un archivo al cliente (debes implementar la lógica para enviar el archivo)
            strcpy(buffer, "Contenido del archivo solicitado\r\n");
        }
        else if (strncmp(buffer, "QUIT", 4) == 0)
        {
            // Finalizar la conexión con el cliente
            strcpy(buffer, "221 Adiós\r\n");
            send(client_socket, buffer, strlen(buffer), 0);
            break;
        }
        else
        {
            // Comando no reconocido
            strcpy(buffer, "500 Comando no válido\r\n");
        }

        // Enviar respuesta al cliente
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
}
