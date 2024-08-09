#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
#define SIZE_BUF 1024
#define IP_ADDR "192.168.1.116"

int main(int argc, char const* argv[])
{
    int status, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char user_data[SIZE_BUF] = { 0 };
    char buffer[SIZE_BUF] = { 0 };

    snprintf(user_data, SIZE_BUF, "ADD_USER %s %s %s %s", "Fabio", "Barbato", "Raziel", "password");

    
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)))< 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
    send(client_fd, user_data, strlen(user_data), 0);
    printf("User data sent\n");
    read(client_fd, buffer, SIZE_BUF);
    printf("%s\n", buffer);

    close(client_fd);
    return 0;
}
