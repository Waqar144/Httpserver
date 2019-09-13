#ifndef _utility_funcs_h
#define _utility_funcs_h

/* Returns the webroot */
char* webroot();

void server_send(int sockett, char *msg);

int recv_new(int fd, char *buffer);

int get_file_size(int fd);

#endif