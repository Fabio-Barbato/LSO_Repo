#ifndef serverop
#define serverop

#include "Controller/usrctrl.h"
#include "Controller/bkctrl.h"
#include "Controller/loanctrl.h"
#include "serverop.h"
#define SIZE_BUF 1024

pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void add_request(char request[],int client_socket){
    
    char command[SIZE_BUF] = {0};
    char name[SIZE_BUF] = {0}, surname[SIZE_BUF] = {0}, username[SIZE_BUF] = {0}, password[SIZE_BUF] = {0};
    sscanf(request, "%s %s %s %s %s", command, name, surname, username, password);

    pthread_mutex_lock(&reg_mutex);
    int result = add_user(name,surname,username,password);
    pthread_mutex_unlock(&reg_mutex);

    if (result == 0) {
        send(client_socket, "User added successfully", strlen("User added successfully"), 0);
    } else {
        send(client_socket, "Failed to add user", strlen("Failed to add user"), 0);
    }

}

void login_request(char request[],int client_socket){
    
    char command[SIZE_BUF] = {0};
    char username[SIZE_BUF] = {0}, password[SIZE_BUF] = {0};
    sscanf(request, "%s %s %s", command, username, password);

    int result = login(username,password);

    if (result == 0) {
        send(client_socket, "Successfully logged", strlen("Successfully logged"), 0);
    } else {
        send(client_socket, "Failed login", strlen("Failed login"), 0);
    }

}

void loan_request(char request[],int client_socket){
    
    char command[SIZE_BUF] = {0};
    char username[SIZE_BUF] = {0}, password[SIZE_BUF] = {0};
    sscanf(request, "%s %s %s", command, username, password);

    pthread_mutex_lock(&reg_mutex);
    int result = login(username,password);
    pthread_mutex_unlock(&reg_mutex);

    if (result == 0) {
        send(client_socket, "Successfully logged", strlen("Successfully logged"), 0);
    } else {
        send(client_socket, "Failed login", strlen("Failed login"), 0);
    }

}
void command_parse(char request[],int client_socket){
    char command[SIZE_BUF] = {0};
    sscanf(request, "%s", command);

    if(strcmp(command,"ADD_USER")==0){
        add_request(request, client_socket);
    }else if(strcmp(command,"LOGIN")==0){
        login_request(request,client_socket);
    }else if(strcmp(command,"LOAN")==0){
        loan_request(request,client_socket);
    }
    else{
        send(client_socket, "Unknown command", strlen("Unknown command"), 0);
    }
}

#endif