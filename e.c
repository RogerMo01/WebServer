#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

// Predefined functions
void print_matrix(char** matrix, int len);
void handle_client(int client_socket, char* ROOT, int PORT, char* baseROOT);
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



    while (1)
    {
        // Aceptar una nueva conexión de cliente
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            fprintf(stderr, RED_COLOR "Error: accept error\n" DEFAULT_COLOR);
            exit(1);
        }

        printf("New connection accepted\n\n");


        // Manejar al cliente en un proceso hijo
        if (fork() == 0)
        {
            close(server_socket);
            char ROOTcopy[128] = "";
            strcat(ROOTcopy, ROOT);
            handle_client(client_socket, ROOTcopy, PORT, ROOT);
            exit(0);
        }

        close(client_socket);
    }
    
    close(server_socket);
    return 0;
}






void handle_client(int client_socket, char* ROOT, int PORT, char* baseROOT)
{
    // Buffer para almacenar los datos recibidos
    char buffer[BUFFER_SIZE];

    // Leer los datos enviados por el cliente
    ssize_t bytes_read;
    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        char* html_content = malloc(4096);
        
        // Imprimir los datos de la solicitud en la consola
        printf("Solicitud recibida:\n%s\n", buffer);


        // Obtener el método HTTP (GET, POST, etc.)
        char method[10];
        sscanf(buffer, "%s", method);
        printf("method = %s\n", method);

        // Obtener la ruta de la solicitud
        char path[BUFFER_SIZE];
        sscanf(buffer, "%*s %s", path);
        printf("path = %s\n", path);

        printf("la root es: %s\n", ROOT);



        printf("ROOT[strlen(ROOT) - 1] = %c\n",ROOT[strlen(ROOT) - 1]);
        if(ROOT[strlen(ROOT) - 1] == '/') ROOT[strlen(ROOT) - 1] = '\0';
        printf("ROOT[strlen(ROOT) - 1] = %c\n",ROOT[strlen(ROOT) - 1]);



        if(strcmp(method, "GET") == 0 && strcmp(path, "/favicon.ico") != 0)
        {
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GET case ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            
            // Obtener los parámetros GET (si los hay)
            char* query_string = strchr(path, '?');
            if (query_string != NULL)
            {
                // Avanzar el puntero para omitir el signo de interrogación
                query_string++;

                // Procesar los pares clave-valor de los parámetros GET
                char* token = strtok(query_string, "&");
                while (token != NULL)
                {
                    // Dividir el parámetro en clave y valor
                    char key[BUFFER_SIZE];
                    char value[BUFFER_SIZE];
                    sscanf(token, "%[^=]=%s", key, value);

                    // Realizar la lógica deseada con los parámetros GET

                    // Avanzar al siguiente parámetro
                    token = strtok(NULL, "&");
                }
            }
            printf("query_string = %s\n\n", query_string);



            // Determinate if new path is a Folder or file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            char tempRoot[128] = "";
            strcat(tempRoot, baseROOT);
            strcat(tempRoot, path);

            printf("Mientras ROOT = %s, tempRoot = %s\n", ROOT, tempRoot);

            // Folder = 1
            // File = 2
            int type = 0;
            DIR* dir = opendir(tempRoot);
            if(dir) {
                printf(RED_COLOR"%s es una carpeta.\n"DEFAULT_COLOR, tempRoot);
                type = 1;
                closedir(dir);
            } 
            else{
                printf(RED_COLOR"%s es un archivo.\n"DEFAULT_COLOR, tempRoot);
                type = 2;
            }
            tempRoot[0] = '\0';
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


            // if(is folder)
            if(type == 1)
            {
                // concatenar path a ROOT
                strcat(ROOT, path);
                printf("la NEW root es: %s\n", ROOT);


                // Abrir el directorio y obtener el número de archivos y carpetas
                DIR* dir = opendir(ROOT);
                if (dir == NULL) { printf("No se pudo abrir el directorio %s\n", ROOT); exit(1); }
                int filesCount = 0;
                struct dirent* ent;
                while ((ent = readdir(dir)) != NULL) { if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) filesCount++; }
                // Crear un array para almacenar los nombres
                char** names = (char**)malloc(filesCount * sizeof(char*));
                if (names == NULL) { printf("Error al asignar memoria\n"); exit(1); }
                // Obtener los nombres de los archivos y carpetas
                getNames(ROOT, names, &filesCount);


                html_content = build_html(PORT, names, filesCount);
            }
            else if (type == 2)//if(is File)
            {
                // poner un '\0' en la posicion total - len del path
                printf("Cuando es un file:\n");
                printf("ROOT = %s, baseROOT = %s, y path = %s\n", ROOT, baseROOT, path);

                // 🚨🚨🚨🚨  Aquí lo que hacer con un archivo  🚨🚨🚨🚨
                // Construir la ruta completa del archivo
                char file_path[128] = "";
                strcpy(file_path, baseROOT);
                strcat(file_path, path);

                // Abrir el archivo en modo de lectura binaria
                FILE* file = fopen(file_path, "rb");
                if (file == NULL)
                {
                    // El archivo no existe o no se puede abrir
                    // Envía una respuesta de error al cliente
                    char error_response[128];
                    sprintf(error_response, "HTTP/1.1 404 Not Found\r\n"
                                            "Content-Length: %d\r\n\r\n"
                                            "File not found or cannot be opened", 32);
                    send(client_socket, error_response, strlen(error_response), 0);
                }
                else 
                {
                    // Obtener el tamaño del archivo
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    // Leer el contenido del archivo en un búfer
                    char* file_buffer = malloc(file_size);
                    fread(file_buffer, file_size, 1, file);
                    

                    // Construir la respuesta HTTP con el contenido del archivo
                    char response[4096];
                    sprintf(response, "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: application/octet-stream; charset=UTF-8\r\n"
                                    "Content-Disposition: attachment; filename=\"%s\"\r\n"
                                    "Content-Length: %ld\r\n\r\n", path, file_size);
                    send(client_socket, response, strlen(response), 0);

                    // Enviar el contenido del archivo al cliente
                    send(client_socket, file_buffer, file_size, 0);

                    // Liberar memoria y cerrar el archivo
                    free(file_buffer);
                    fclose(file);

                    continue;
                }

            }
        
        } 
        else if(strcmp(method, "POST") == 0)
        {
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ POST case ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        }
        
        
        char response[4098];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html; charset=UTF-8\r\n"
                        "Content-Length: %ld\r\n\r\n"
                        "%s", strlen(html_content), html_content);


        send(client_socket, response, strlen(response), 0);
    }


    // Cerrar la conexión con el cliente
    close(client_socket);
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
                        "border-collapse: collapse;"
                        "width: 95%;"
                        "font-family: Arial, sans-serif;"
                        "border-radius: 8px;"
                        "overflow: hidden;"
                    "}"

                    "th, td {"
                        "padding: 8px;"
                        "text-align: left;"
                        "border-bottom: 1px solid #ddd;"
                    "}"

                    "th {"
                        "background-color: #ababab;"
                    "}"

                    "tr:nth-child(even) {"
                        "background-color: #f9f9f9;"
                   "}"

                    "tr:hover {"
                        "background-color: #eaeaea;"
                        "cursor: pointer;"
                    "}"

                    "td:first-child {"
                        "border-left: 1px solid #ddd;"
                    "}"

                    "td:last-child {"
                        "border-right: 1px solid #ddd;"
                    "}"
                "</style>"

            "</head>"
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
                        "var current = window.location.href;"
                        "console.log(current);"
                        "if(current[current.length-1] == '/')"
                        "{ window.location.href = current + name; }"
                        "else window.location.href = current + '/' + name;"
                    "}"
                "</script>"
            "</body>"
            "</html>");

    return response;
}

