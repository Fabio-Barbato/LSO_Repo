#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
#define SIZE_BUF 2046
#define IP_ADDR "192.168.1.116"

int login(int client_fd, char *logged_in_user) {
    char username[SIZE_BUF], password[SIZE_BUF], buffer[SIZE_BUF] = {0};
    printf("Inserisci username: ");
    scanf("%s", username);
    printf("Inserisci password: ");
    scanf("%s", password);

    char request[SIZE_BUF];
    snprintf(request, SIZE_BUF, "LOGIN %s %s", username, password);
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("%s\n", buffer);

    if (strcmp(buffer, "Successfully logged") == 0) {
        strcpy(logged_in_user, username);
        return 1;
    } else {
        return 0;
    }
}

void register_user(int client_fd) {
    char name[SIZE_BUF], surname[SIZE_BUF], username[SIZE_BUF], password[SIZE_BUF], buffer[SIZE_BUF] = {0};
    printf("Inserisci nome: ");
    scanf("%s", name);
    printf("Inserisci cognome: ");
    scanf("%s", surname);
    printf("Inserisci username: ");
    scanf("%s", username);
    printf("Inserisci password: ");
    scanf("%s", password);

    char request[SIZE_BUF];
    snprintf(request, SIZE_BUF, "ADD_USER %s %s %s %s", name, surname, username, password);
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("%s\n", buffer);
}

void view_books(int client_fd) {
    char buffer[SIZE_BUF] = {0};
    char *request = "GET_BOOKS";
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("Catalogo dei libri:\n%s\n", buffer);
}

void view_cart(char cart[][SIZE_BUF], int cart_count) {
    if (cart_count == 0) {
        printf("Il carrello è vuoto!\n");
        return;
    }
    printf("Carrello:\n");
    for (int i = 0; i < cart_count; i++) {
        printf("%s\n", cart[i]);
    }
}

void add_to_cart(char cart[][SIZE_BUF], int *cart_count) {
    char isbn[SIZE_BUF];
    printf("Inserisci l'ISBN del libro da aggiungere al carrello: ");
    scanf("%s", isbn);
    strcpy(cart[(*cart_count)++], isbn);
    printf("Libro aggiunto al carrello!\n");
}

void checkout(int client_fd, char cart[][SIZE_BUF], int cart_count, char *logged_in_user) {
    if (cart_count == 0) {
        printf("Il carrello è vuoto!\n");
        return;
    }

    char request[SIZE_BUF] = "LOAN ";
    strcat(request, logged_in_user);
    strcat(request, " ");
    for (int i = 0; i < cart_count; i++) {
        strcat(request, cart[i]);
        if (i < cart_count - 1) strcat(request, " ");
    }

    char buffer[SIZE_BUF] = {0};
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("%s\n", buffer);
}

void check_notifications(int client_fd, char *logged_in_user) {
    char buffer[SIZE_BUF] = {0};
    char request[SIZE_BUF];
    snprintf(request, SIZE_BUF, "CHECK_NOTIFICATIONS %s", logged_in_user);
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("Notifiche:\n%s\n", buffer);
}

void check_due_dates(int client_fd, char *logged_in_user) {
    char buffer[SIZE_BUF] = {0};
    char request[SIZE_BUF];
    snprintf(request, SIZE_BUF, "GET_LOANS %s", logged_in_user);
    send(client_fd, request, strlen(request), 0);
    read(client_fd, buffer, SIZE_BUF);
    printf("Scadenze:\n%s\n", buffer);
}

int main(int argc, char const *argv[]) {
    int client_fd, status;
    struct sockaddr_in serv_addr;
    char logged_in_user[SIZE_BUF] = {0};  // Username dell'utente loggato

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Errore nella creazione del socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr) <= 0) {
        printf("Indirizzo non valido o non supportato\n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("Connessione fallita\n");
        return -1;
    }

    int choice, cart_count = 0;
    char cart[SIZE_BUF][SIZE_BUF];
    int logged_in = 0;

    while (1) {
        printf("\n1. Login\n2. Registrati\n3. Visualizza catalogo\n4. Aggiungi al carrello\n5. Visualizza carrello\n6. Checkout\n7. Visualizza notifiche\n8. Visualizza scadenze\n9. Esci\n");
        printf("Scegli un'opzione: ");
        scanf("%d", &choice);

        if (choice == 1) {
            logged_in = login(client_fd, logged_in_user);
        } else if (choice == 2) {
            register_user(client_fd);
        } else if (logged_in) {
            switch (choice) {
                case 3:
                    view_books(client_fd);
                    break;
                case 4:
                    add_to_cart(cart, &cart_count);
                    break;
                case 5:
                    view_cart(cart, cart_count);
                    break;
                case 6:
                    checkout(client_fd, cart, cart_count, logged_in_user);
                    cart_count = 0;  // Svuota il carrello dopo il checkout
                    break;
                case 7:
                    check_notifications(client_fd, logged_in_user);
                    break;
                case 8:
                    check_due_dates(client_fd, logged_in_user);
                    break;
                case 9:
                    close(client_fd);
                    exit(0);
                default:
                    printf("Scelta non valida!\n");
                    break;
            }
        } else {
            printf("Devi effettuare il login per accedere a questa funzionalità.\n");
        }
    }

    return 0;
}