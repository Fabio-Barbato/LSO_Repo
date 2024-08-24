#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "serverop.h"
#define PORT 8080
#define SIZE_BUF 1024


void* handle_client(void* arg);

int main(int argc, char const* argv[])
{
    int server_fd, new_socket; //socket descriptor, first connection extracted
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t tid;
    pthread_t tid_notify;

    printf("Creating socket file descriptor...\n");
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //printf("%s",cJSON_Print(search_book("9788850250998")));

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { //reuse address enabled
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
/*
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) { //reuse port enabled
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }*/

    printf("Defining address...\n");
// DEFINE ADDRESS
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    printf("Binding...\n");
//  BIND
    if (bind(server_fd, (struct sockaddr*)&address,sizeof(address))< 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening...\n");
// LISTEN
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

//NOTIFY
    if (pthread_create(&tid_notify, NULL, overdue_check, (void*)(intptr_t)new_socket) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

while(1){
//ACCEPT
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,&addrlen))< 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid, NULL, handle_client, (void*)(intptr_t)new_socket) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    pthread_detach(tid);

}
    close(server_fd);
    return 0;
}

void* handle_client(void* arg) {
    int new_socket = (intptr_t)arg;
    ssize_t valread;
    char buffer[SIZE_BUF] = {0};

    while ((valread = read(new_socket, buffer, SIZE_BUF - 1)) > 0) {
        buffer[valread] = '\0';
        printf("Command received: %s\n", buffer);
        command_parse(buffer, new_socket);

        // Clean buffer
        memset(buffer, 0, SIZE_BUF);
    }

    if (valread == 0) {
        printf("Client disconnected.\n");
    } else if (valread < 0) {
        perror("read");
    }

    close(new_socket);
    return NULL;
}
