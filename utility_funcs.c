#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#define EOL "\r\n"
#define EOL_SIZE 2

char* webroot() {
 // open the file "conf" for reading
    FILE *in = fopen("conf", "rt");
 // read the first line from the file
    char buff[1000];
    fgets(buff, 1000, in);
 // close the stream
    fclose(in);
    char* nl_ptr = strrchr(buff, '\n');
    if (nl_ptr != NULL)
        *nl_ptr = '\0';
    return strdup(buff);
}

void server_send(int sockett, char *msg) {
 int len = strlen(msg);
 if (send(sockett, msg, len, 0) == -1) {
  printf("Error while sending\n");
 }
}

/*
 This function recieves the buffer
 until an "End of line(EOL)" byte is recieved
 */
int recv_new(int fd, char *buffer) {
    char *p = buffer; // Use of a pointer to the buffer rather than dealing with the buffer directly
    int eol_matched = 0; // Use to check whether the recieved byte is matched with the buffer byte or not
    while (recv(fd, p, 1, 0) != 0) // Start receiving 1 byte at a time
    {
        if (*p == EOL[eol_matched]) // if the byte matches with the first eol byte that is '\r'
        {
            ++eol_matched;
            if (eol_matched == EOL_SIZE) // if both the bytes matches with the EOL
            {
                *(p + 1 - EOL_SIZE) = '\0'; // End the string
                return (strlen(buffer)); // Return the bytes recieved
            }
        } else {
            eol_matched = 0;
        }
        p++; // Increment the pointer to receive next byte
    }
    return (0);
}

int get_file_size(int fd) {
 struct stat stat_struct;
 if (fstat(fd, &stat_struct) == -1)
  return (1);
 return (int) stat_struct.st_size;
}