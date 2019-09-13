#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "utility_funcs.h"


typedef struct{
    char *ext;
    char *media_type;
}Extensions;

/*Extension Types that our Server
  should be able to handle
*/
Extensions extn[] = {
    {"gif", "image/gif" },
    {"jpg", "image/jpg" },
    {"jpeg", "image/jpeg" },
    {"png", "image/png" },
    {"ico", "image/ico" },
    {"zip", "image/zip" },
    {"tar", "image/tar" },
    {"gz", "image/gz" },
    {"txt", "text/plain" },
    {"htm", "text/html" },
    {"html", "text/html" },
    {"php", "text/html" },
    {"pdf", "application/pdf" },
    {"zip", "application/octet-stream" },
    {"rar", "application/octet-stream" },
    {NULL, NULL },
};

/* read_file(FILE *f)
   Takes @f FILE as argument
   and copies it's contents to a
   char *buffer
*/
char* read_file (FILE *f)
{
    char *buffer = 0;
    long length;
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);

        if (buffer)
        {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
    buffer[length-1] = '\0';
    return buffer;
}







/* handle_connection (int cl_socket)
   @cl_socket: Client Socket
   Descripttion: This function handles the
   requests coming from the client.
*/
int handle_connection (int cl_socket)
{
    char request[1024];
    char resource[1024];
    char *ptr;
    int fdl;
    int length = 0;

    if (recv_new(cl_socket, request) == 0) {
        printf("Recieve failed.\n");
    }
    printf("%s\n", request);
    ptr = strstr(request, "HTTP/");
    if (ptr == NULL) {
        printf("Not a valid HTTP request.\n");
    } else {
        *ptr = 0;
        ptr = NULL;

        if (strncmp(request, "GET ", 4) == 0)
            ptr = request + 4;
        if (ptr == NULL) // in case above failed
            printf("Unknown Request.\n");
        else {
            if(ptr[strlen(ptr) - 1] == '/') {
                strcat(ptr, "index.html");
            }
            strcpy(resource, webroot());
            int len = strlen(ptr);
            ptr[len-1] = '\0'; //null terminate and trim the leading white spance
            strcat(resource, ptr);
            char *s = strchr(ptr, '.');
            s = s+1;
            int i;
            for (i = 0; extn[i].ext != NULL; i++) {
                if (strcmp(s, extn[i].ext) == 0) {
                    fdl = open(resource, O_RDONLY, 0);
                    printf("Opening \"%s\" resource.\n", resource);
                    if(fdl == -1) {
                        printf ("404 File not found Error\n");
                        server_send(cl_socket, "Waqars Server\r\n\r\n");
                        server_send(cl_socket, "<html><head>404 Not Found</head><body>404 Not found.</body></html>");
                        close(cl_socket);
                        return 0;
                    }
                    else {
                        printf("200 Ok. Content Type: %s\n\n", extn[i].media_type);
                        server_send(cl_socket, "HTTP/1.1 200 Ok\r\n");
                        server_send(cl_socket, "Waqars Server\r\n\r\n");
                        if (ptr == request + 4) { //if it's a GET request
                            if ((length = get_file_size(fdl)) == -1)
                                printf("Error getting file size\n");
                            size_t total_bytes_sent = 0;
                            ssize_t bytes_sent;
                            while (total_bytes_sent < length) {
                                //Zero Copy Optimization
                                if ((bytes_sent = sendfile(
                                    cl_socket, fdl, 0,
                                    length - total_bytes_sent)) <= 0) {
                                        if (errno == EINTR || errno == EAGAIN) {
                                            continue;
                                        }
                                        perror("Sendfile");
                                        return -1;
                                    }
                                total_bytes_sent += bytes_sent;
                            }
                        }
                    }
                    break;
                }
                int size = sizeof(extn) /sizeof(extn[0]);
                if (i==size -2) {
                    printf("415 Unsupported Media Type\n");
                    server_send(cl_socket, "HTTP/1.1 415 Unsupported Media Type\r\n");
                    //server_send(cl_socket, "Waqars Server\r\n\r\n");
                    server_send(cl_socket, "<html><head>415 Unsupported Media Type</head><body>415 Unsupported Mediatype</body></html>");
                }
            }
            close(cl_socket);
        }
    }
    shutdown(cl_socket, SHUT_RDWR);
    return 1;
}

int main(int argc, char *argv[])
{
    int server_socket;
    int client_socket;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength;

    //create the socket
    server_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    //define the address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8001);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //bind the socket
    if(!bind(server_socket, (struct sockaddr*) &serverAddress, 
            sizeof(serverAddress))) {
        printf("Failed to bind socket");
    }
    listen(server_socket, 5);

    while(1) {
        //create the client socket
        client_socket = accept(server_socket,
                              (struct sockaddr*)&clientAddress,
                              &clientAddressLength); 
        //server_send(client_socket, "HTTP/1.1 200 Ok\r\n\n<html><head>File</head><body>Hello</body></html>");
        handle_connection(client_socket);
    }
    close(server_socket);
    return 0;
}