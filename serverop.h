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
        send(client_socket, "Username already exists", strlen("Username already exists"), 0);
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

    pthread_mutex_lock(&loan_mutex);

    int result = count_loans(username);  // Check user's loans
    int count = result;
    if (isbn_count <= MAX_LOAN) {
        if (result <= MAX_LOAN && result + isbn_count <= MAX_LOAN) {  // Check if the loan is possible
            result = checkout(isbn_array, isbn_count);

            if (result == 0) {
                for (int i = 0; i < isbn_count; i++) {
                    result = add_loan(username, isbn_array[i]);
                }
            } else {
                result = -2;
            }
        } else {
            if (result >= MAX_LOAN) {
                result = -1;
            } else {
                result = -3;
            }
        }
    } else {
        result = -4;
    }

    pthread_mutex_unlock(&loan_mutex);

    char message[SIZE_BUF];
    if (result == 0) {
        snprintf(message, SIZE_BUF, "Loan confirmed");
    } else if (result == -1) {
        snprintf(message, SIZE_BUF, "You have already made %d loans", MAX_LOAN);
    } else if (result == -2) {
        snprintf(message, SIZE_BUF, "No more copies available for the book: %s", unavailable_book_title);
    } else if (result == -3) {
        snprintf(message, SIZE_BUF, "You have already made %d loans, you can only apply for %d more", count, MAX_LOAN-count);
    } else if (result == -4) {
        snprintf(message, SIZE_BUF, "Request less than %d books", MAX_LOAN);
    }

    send(client_socket, message, strlen(message), 0);
    unavailable_book_title[0] = '\0';
}

void send_book(char request[], int client_socket){
    char command[SIZE_BUF] = {0};
    char isbn[SIZE_BUF] = {0};

    sscanf(request, "%s %s", command, isbn);

    char* book = cJSON_Print(search_book(isbn));
    printf("Sending %s\n",book);
    send(client_socket, book, strlen(book), 0);
}

void send_books(int client_socket) {
    char *json_data = jsonToString(BK);
    if (json_data == NULL) {
        const char *error_message = "Failed to read JSON file";
        send(client_socket, error_message, strlen(error_message), 0);
        return;
    }
    printf("Sending %s\n",json_data);
    send(client_socket, json_data, strlen(json_data), 0);

    free(json_data);
}

void send_loans(const char* request, int client_socket) {
    char command[SIZE_BUF] = {0};
    char username[SIZE_BUF] = {0};
    sscanf(request, "%s %s", command, username);

    cJSON* response = NULL;

    if (strcmp(command, "GET_LOANS") == 0) {
        response = get_user_loans(username);
    }

    if (response) {
        char* response_str = cJSON_Print(response);
        send(client_socket, response_str, strlen(response_str), 0);
        free(response_str);
        cJSON_Delete(response);
    } else {
        char* response_str = "[]"; // ritorna un JSON vuoto in caso di errore o dati non trovati
        send(client_socket, response_str, strlen(response_str), 0);
    }
}

char *username;
void command_parse(char request[], int client_socket) {
    char command[SIZE_BUF] = {0};
    sscanf(request, "%s", command);

    if (strcmp(command, "ADD_USER") == 0) {
        add_request(request, client_socket);
    } else if (strcmp(command, "LOGIN") == 0) {
        username = login_request(request, client_socket);
    } else if (strcmp(command, "LOAN") == 0) {
        loan_request(request, client_socket);
    } else if (strcmp(command, "GET_BOOKS") == 0) {
            send_books(client_socket);
    } else if (strcmp(command, "GET_BOOK") == 0) {
            send_book(request, client_socket);
    }  else if (strcmp(command, "GET_LOANS") == 0) {
        send_loans(request, client_socket);
    } else if (strcmp(command, "CHECK_NOTIFICATIONS") == 0) {
      if (username) {
        notify_user(username, client_socket);
        free(username); // Deallocate the memory allocated for the username
        }
    } else {
        send(client_socket, "Unknown command", strlen("Unknown command"), 0);
    }
}

#endif