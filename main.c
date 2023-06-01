#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

// Predefined functions
void print_matrix(char** matrix, int len);
void handle_client(int client_socket, char* ROOT, int PORT);
char* build_html(int PORT, char** names, int filesCount);
void getNames(const char* root, char** names, int* numNames);
char* parseHttpRequest(const char* httpRequest);

#define BUFFER_SIZE 1024
#define RED_COLOR "\033[0;31m"
#define DEFAULT_COLOR "\033[0m"


int main(int argc, char *argv[])
{
    int PORT = atoi(argv[1]);
    char* ROOT = argv[2];

    if(PORT == 0) { fprintf(stderr, RED_COLOR "Error: invalid Port\n" DEFAULT_COLOR); exit(1); }
    DIR* dir = opendir(argv[2]);
    if (dir == NULL) { fprintf(stderr, RED_COLOR "Error: invalid Directory Root\n" DEFAULT_COLOR); exit(1); }
    closedir(dir);

    // printf("argc = %i\n", argc);
    printf("Port = %i\n", PORT);
    printf("Root = %s\n", ROOT);


    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);


    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        fprintf(stderr, RED_COLOR "Error: cannot create Socket\n" DEFAULT_COLOR);
        exit(1);
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket al puerto
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, RED_COLOR "Error: bind error\n" DEFAULT_COLOR);
        exit(1);
    }

    // Escuchar por conexiones entrantes
    if (listen(server_socket, 5) < 0)
    {
        fprintf(stderr, RED_COLOR "Error: listen error\n" DEFAULT_COLOR);
        exit(1);
    }

    printf("WebServer is running...\n");


    // Listen loop
    while (1)
    {
        // Aceptar una nueva conexión de cliente
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            fprintf(stderr, RED_COLOR "Error: accept error\n" DEFAULT_COLOR);
            exit(1);
        }

        printf("New connection accepted\n");


        // Manejar al cliente en un proceso hijo
        if (fork() == 0)
        {
            close(server_socket);
            handle_client(client_socket, ROOT, PORT);
            exit(0);
        }

        close(client_socket);
    }
    
    close(server_socket);



    return 0;
}


void handle_client(int client_socket, char* ROOT, int PORT)
{
    // Buffer para almacenar los datos recibidos
    char buffer[BUFFER_SIZE];

    // Leer los datos enviados por el cliente
    ssize_t bytes_read ;

    char* html_content = "<!DOCTYPE html>""<html>""<head></head><body></body></html>";

    while (bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0) > 0)
    {
        // Verificar si hubo un error en la lectura
        if (bytes_read < 0)
        {   fprintf(stderr, RED_COLOR "Error: recv error\n" DEFAULT_COLOR);
            close(client_socket);
            return; }
        // Verificar si la lectura ha terminado (bytes_read == 0 indica fin de conexión)
        if (bytes_read == 0)
        {   close(client_socket);
            return; }


        // Leer los datos enviados por el cliente
        // ssize_t bytes_read;

        // Buscar el final de los encabezados
        // char* body_start = strstr(buffer, "\r\n\r\n");
        // if (body_start != NULL) {
        //     body_start += 4;  // Saltar los caracteres "\r\n\r\n"
        //     // Aquí puedes procesar el cuerpo de la solicitud según tus necesidades
        //     // En este ejemplo, simplemente imprimimos el cuerpo en el servidor
        //     printf("Received request body from client:\n");
        //     printf("%s\n", body_start);
        // }

        


        // Aquí puedes procesar los datos recibidos según el protocolo que estés utilizando
        // y enviar respuestas al cliente en consecuencia.
        // Puedes implementar el manejo de solicitudes HTTP u otro protocolo aquí.

        // if (strncmp(buffer, "POST", 4) == 0)
        // {
        //     // Leer los datos enviados por el cliente
        //     ssize_t bytes_read;

        //     // Buscar el final de los encabezados
        //     char* body_start = strstr(buffer, "\r\n\r\n");
        //     if (body_start != NULL) {
        //         body_start += 4;  // Saltar los caracteres "\r\n\r\n"
        //         // Aquí puedes procesar el cuerpo de la solicitud según tus necesidades
        //         // En este ejemplo, simplemente imprimimos el cuerpo en el servidor
        //         printf("Received POST request body from client:\n");
        //         // printf("%s\n", body_start);

                printf("Buffer = \n%s", buffer);
                printf("~~~~~~~~~~~~~ END_Buffer ~~~~~~~~~~~~~~");

        //         char* value = parseHttpRequest(buffer);

        //         printf("~~~~~~ Parsed = %s", value);


        //     }

        //         // 🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨
        //         // printf("Received POST data from client:\n");
        //         // printf("%s\n", buffer);
        // }
        // else{
        //     // En este ejemplo, simplemente imprimimos los datos recibidos en el servidor
        //     printf("Received data, not POST, from client:\n");
        // }



        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Abrir el directorio y obtener el número de archivos y carpetas
        DIR* dir = opendir(ROOT);
        if (dir == NULL) {
            printf("No se pudo abrir el directorio %s\n", ROOT);
            exit(1);
        }
        int filesCount = 0;
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                filesCount++;
            }
        }
        // Crear un array para almacenar los nombres
        char** names = (char**)malloc(filesCount * sizeof(char*));
        if (names == NULL) {
            printf("Error al asignar memoria\n");
            exit(1);
        }

        // Obtener los nombres de los archivos y carpetas
        getNames(ROOT, names, &filesCount);

        // Imprimir los nombres
        // printf("Archivos y carpetas en el directorio %s:\n", ROOT);
        // for (int i = 0; i < numArchivos; i++) {
        //     printf("%s\n", names[i]);
        // }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~







        // Construir el contenido HTML de la página
        html_content = build_html(PORT, names, filesCount);


        // Construir la respuesta HTTP (código de estado 200 OK)
        char response[4098];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %ld\r\n\r\n"
                        "%s", strlen(html_content), html_content);



        // Enviar la respuesta al cliente
        ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);

        // Verificar si hubo un error en el envío
        if (bytes_sent < 0)
        {
            fprintf(stderr, RED_COLOR "Error: send error\n" DEFAULT_COLOR);
        }

    }

    // Cerrar la conexión con el cliente
    close(client_socket);
}


char* parseHttpRequest(const char* httpRequest) {
    const char* delimiter = "\n";
    char* token;
    char* value = NULL;
    
    
    return value;
}



void getNames(const char* root, char** names, int* numNames) {
    DIR* dir;
    struct dirent* ent;

    // Abrir el directorio
    dir = opendir(root);
    if (dir == NULL) {
        printf("No se pudo abrir el directorio %s\n", root);
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        // Ignorar los nombres "." y ".."
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        // Copiar el nombre del archivo o carpeta al array
        names[i] = strdup(ent->d_name);
        i++;
    }

    *numNames = i;

    // Cerrar el directorio
    closedir(dir);
}


char* build_html(int PORT, char** names, int filesCount)
{
    char* response = malloc(4096);
    char buff[4]; 
    sprintf(buff, "%d", PORT);

    strcat(response, "<!DOCTYPE html>"
            "<html>"
            "<head>"
                "<style>"
                   "table {"
                       "width: 95\%;"
                        "border-collapse: collapse;"
                        "border:1px solid black;"
                        "margin-left:auto;"
                        "margin-right:auto;"
                    "}"
                    "th, td {"
                        "padding: 8px;"
                        "text-align: left;"
                        "border-bottom: 1px solid #ddd;"
                    "}"
                    "tr:hover {"
                        "background-color: #ffffff;"
                    "}"
                    "th {"
                        "background-color: #3f7bdb;"
                        "color: rgb(255, 255, 255);"
                    "}"
                "</style></head>"
            "<body>"
            "<h1>Explorer</h1>"
            "<table>"
                "<tr>"
                    "<th>Nombre</th>"
                "</tr>");

    // ahora poner todos los <tr> dinamicamente
    for (int i = 0; i < filesCount; i++) {
        // printf("%s\n", names[i]);
        strcat(response, "<tr onclick=\"sendRequest('");
        strcat(response, names[i]);
        strcat(response, "')\"><td>");
        strcat(response, names[i]);
        strcat(response, "<td></tr>");
    }

    strcat(response, 
            "</table>"
            "<script>"
                "function sendRequest(name) {"

                    "var xhr = new XMLHttpRequest();"
                    "xhr.open('POST', 'http://localhost:");
    strcat(response, buff);
    strcat(response, "', true);"
                        "xhr.onreadystatechange = function() {"
                        "if (xhr.readyState === 4 && xhr.status === 200)"
                        "{"
                        "    console.log('Solicitud enviada correctamente');"
                        "    console.log(xhr.responseText);"
                        "}"
                        "else console.log('Error en la solicitud');"
                    "};"
                    "var data = 'name:' + encodeURIComponent(name) + '\\n';"
                    // 🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨
                    // "var data = name.length.toString() + '\\n' + name + '\\n';"
                    "xhr.send(data);"
                "}"
            "</script>"
            "</body>"
            "</html>");

    return response;
}



void print_matrix(char** matrix, int len)
{
    printf("~~~ Matrix: ~~~\n");
    for (int i = 0; i < len; i++)
    {
        if(matrix[i] == NULL)
        {
            printf("NULL\n");
        }
        else
        {
            printf("%s\n", matrix[i]);
        }
    }
    printf("~~~~ End ~~~~\n");
}

