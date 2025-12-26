#include "migration.h"

int server_port = -1;
char *server_ip = NULL; 

void set_dest_VMM(int port, char *ip) {
    server_port = port;
    server_ip = ip;
}

size_t my_write(int fd, void *buf, size_t length)
{
    size_t left;
    ssize_t sent;
    char *buff_pointer = buf;

    left = length;
    
    while (left > 0) {
        sent = write(fd, buff_pointer, left);
        if (sent <= 0) {
            printf("Error while sending\n");
            exit(1);
        } 

        left -= sent;
        buff_pointer += sent;
    }
    return length;
}

int client_connection(int port, char *ip) {
    struct sockaddr_in server_addr;

    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);    
    if (conn_fd == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    while (connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("connect failed, retry in 1 second...\n");
        sleep(1);
    }
    
    printf("Connected to VMM in %s:%d\n", ip, port);

    return conn_fd;
}

void send_vm_image(VM_image *image) {

    if (server_port == -1 || server_ip == NULL) {
        return;
    }

    int conn_fd = client_connection(server_port, server_ip);

    ssize_t sent = my_write(conn_fd, &image->registers, sizeof(image->registers));
    sent += my_write(conn_fd, &image->sregisters, sizeof(image->sregisters));

    sent += my_write(conn_fd, &image->msrs, sizeof(image->msrs));

    sent += my_write(conn_fd, &image->fpu, sizeof(image->fpu));

    sent += my_write(conn_fd, &image->memory_size, sizeof(image->memory_size));
    sent += my_write(conn_fd, &image->guest_memory_physical_base, sizeof(image->guest_memory_physical_base));
    sent += my_write(conn_fd, &image->guest_memory, sizeof(image->guest_memory));

    sent += my_write(conn_fd, &image->n_open_files, sizeof(image->n_open_files));
    sent += my_write(conn_fd, &image->open_files, sizeof(image->open_files));

    printf("Sent %zd bytes\n", sent);

    close(conn_fd);
}

size_t my_read(int fd, void *buf, size_t length)
{
    size_t left;
    ssize_t recv;
    char *buff_pointer = buf;

    left = length;

    while (left > 0) {
        recv = read(fd, buff_pointer, left);
        if (recv <= 0) {
            printf("Error while reading\n");
            exit(1);
        } 
        left -= recv;
        buff_pointer += recv;
    }

    return length;
}

int server_connection(int port) {
    int conn_fd, client_fd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    
    // Connection socket creation
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd == -1) {
        printf("Socket creation failed\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    if (bind(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        printf("Socket bind failed\n");
        exit(1);
    }
    
    // Listen for 1 connection 
    listen(conn_fd, 1);
    printf("Waiting for VM image on port %d...\n", port);
    
    // Wait for a connection
    client_len = sizeof(client_addr);
    if ((client_fd = accept(conn_fd, (struct sockaddr*)&client_addr, &client_len)) < 0) {
        printf("Connection failed\n");
        exit(1);
    }

    close(conn_fd);

    return client_fd;
}

VM_image receive_vm_image(int client_fd) {
    VM_image image = {0};

    ssize_t received = my_read(client_fd, &image.registers, sizeof(image.registers));
    received += my_read(client_fd, &image.sregisters, sizeof(image.sregisters));

    received += my_read(client_fd, &image.msrs, sizeof(image.msrs));

    received += my_read(client_fd, &image.fpu, sizeof(image.fpu));

    received += my_read(client_fd, &image.memory_size, sizeof(image.memory_size));
    received += my_read(client_fd, &image.guest_memory_physical_base, sizeof(image.guest_memory_physical_base));
    received += my_read(client_fd, &image.guest_memory, sizeof(image.guest_memory));

    received += my_read(client_fd, &image.n_open_files, sizeof(image.n_open_files));
    received += my_read(client_fd, &image.open_files, sizeof(image.open_files));

    printf("Received %zd bytes\n", received);
    
    close(client_fd);

    return image;
}

