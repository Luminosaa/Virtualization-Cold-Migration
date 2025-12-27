#ifndef __MIGRATION__
#define __MIGRATION__

#include "../syscall_manager/syscall_handler.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Concern client VMM
void set_dest_VMM(int port, char *ip);

size_t my_write(int fd, void *buf, size_t length);

size_t my_read(int fd, void *buf, size_t length);

int client_connection(int port, char *ip);

void send_vm_image(VM_image *image);

int server_connection(int port);

VM_image receive_vm_image(int client_fd);

#endif
