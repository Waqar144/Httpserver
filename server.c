#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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

int main(int argc, char *argv)
{
    FILE *html = NULL;
    char* response_data = 0;
    int server_socket = -1;

    typedef struct sockaddr_in InternetSocketAddress;
    InternetSocketAddress isa;

    html = fopen("index.html", "r");
    response_data = read_file(html);

    char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
    strcat(http_header, response_data);

    //create the socket
    server_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    //define the address
    isa.sin_family = AF_INET;
    isa.sin_port = htons(8001);
    isa.sin_addr.s_addr = INADDR_ANY;

    //bind the socket
    if(!bind(server_socket, (struct sockaddr*) &isa, sizeof(isa) )) {
        printf("Failed to bind socket");
    }
    listen(server_socket, 5);
    http_header[2047] = '\0';

    int client_socket;
    while(1) {
        InternetSocketAddress client;
        client_socket = accept(server_socket, NULL, NULL);
        send (client_socket, http_header, sizeof(http_header), 0);
        close(client_socket);
    }

    close(server_socket);
}