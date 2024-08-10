#ifndef serverop
#define serverop

#include "Controller/usrctrl.h"
#include "Controller/bkctrl.h"
#include "Controller/loanctrl.h"
#include "Controller/notctrl.h"
#include "serverop.h"
#define SIZE_BUF 1024
#define MAX_LOAN 5 //Loan's max number

pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t loan_mutex = PTHREAD_MUTEX_INITIALIZER;

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

char* login_request(char request[], int client_socket) {
    char command[SIZE_BUF] = {0};
    char password[SIZE_BUF] = {0};
    char username[SIZE_BUF] = {0};
    sscanf(request, "%s %s %s", command, username, password);

    int result = login(username, password);

    if (result == 0) {
        // Allocate memory for the returned username
        char *username_copy = malloc(strlen(username) + 1);
        if (username_copy == NULL) {
            perror("Failed to allocate memory");
            return NULL;
        }
        strcpy(username_copy, username);

        send(client_socket, "Successfully logged", strlen("Successfully logged"), 0);
        return username_copy;
    } else {
        send(client_socket, "Failed login", strlen("Failed login"), 0);
        return NULL;
    }
}

void loan_request(char request[], int client_socket) {
    char command[SIZE_BUF] = {0};
    char username[SIZE_BUF] = {0};
    char isbn_list[SIZE_BUF] = {0};
    char *isbn_array[SIZE_BUF];
    int isbn_count = 0;

    sscanf(request, "%s %s %[^\n]", command, username, isbn_list);  // %[^\n] takes all remaining string as ISBN list

    // Fetch ISBN from list
    char *token = strtok(isbn_list, " ");
    while (token != NULL) {
        isbn_array[isbn_count++] = token;  // Add isbn to array and increases counter
        token = strtok(NULL, " ");
    }

    pthread_mutex_lock(&reg_mutex);

    int result = count_loans(username);  // check user's loans

    if (result < MAX_LOAN && result+isbn_count<MAX_LOAN) {  // check if loan is possible
        result = checkout(isbn_array, isbn_count);

        if (result == 0) {
            for (int i = 0; i < isbn_count; i++) {
                result = add_loan(username, isbn_array[i]);
            }
        } else {
            result = -2;
        }
    } else {
        result = -1;
    }

    pthread_mutex_unlock(&reg_mutex);

    if (result == 0) {
        send(client_socket, "Loan done", strlen("Loan done"), 0);
    } else if (result == -1) {
        send(client_socket, "You have reached max number of loans", strlen("You have reached max number of loans"), 0);
    } else if(result == -2) {
        send(client_socket, "No copy available", strlen("No copy available"), 0);
    }
}


void command_parse(char request[], int client_socket) {
    char command[SIZE_BUF] = {0};
    sscanf(request, "%s", command);

    if (strcmp(command, "ADD_USER") == 0) {
        add_request(request, client_socket);
    } else if (strcmp(command, "LOGIN") == 0) {
        char *username = login_request(request, client_socket);
        if (username) {
            printf("Notifying user %s\n",username);
            notify_user(username, client_socket);
            free(username); // Deallocate the memory allocated for the username
        }
    } else if (strcmp(command, "LOAN") == 0) {
        loan_request(request, client_socket);
    } else {
        send(client_socket, "Unknown command", strlen("Unknown command"), 0);
    }
}

#endif