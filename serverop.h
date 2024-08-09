#ifndef serverop
#define serverop

#include "Controller/usrctrl.h"
#include "Controller/bkctrl.h"
#include "Controller/loanctrl.h"
#define SIZE_BUF 1024

void add_request(char request[],int client_socket){
    
    char command[SIZE_BUF] = {0};
    char name[SIZE_BUF] = {0}, surname[SIZE_BUF] = {0}, username[SIZE_BUF] = {0}, password[SIZE_BUF] = {0};
    sscanf(request, "%s %s %s %s %s", command, name, surname, username, password);

    int result = add_user(name,surname,username,password);

    if (result == 0) {
        send(client_socket, "User added successfully", strlen("User added successfully"), 0);
    } else {
        send(client_socket, "Failed to add user", strlen("Failed to add user"), 0);
    }

}

#endif