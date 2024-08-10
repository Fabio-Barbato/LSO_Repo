#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "cJSON.h"

#define SIZE_BUF 1024
#define LOAN_FILE "loans.json"
#define NOTIF_FILE "notifications.json"
#define CHECK_INTERVAL 86400 // Intervallo di controllo (1 giorno)

// Funzione per convertire una stringa di data in `time_t`
time_t parse_date(const char* date_str) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    strptime(date_str, "%d-%m-%Y", &tm);
    return mktime(&tm);
}

// Funzione per leggere il JSON da un file
cJSON* read_json(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    cJSON* json = cJSON_Parse(data);
    free(data);
    return json;
}

// Funzione per scrivere un JSON su un file
int write_json(const char* filename, cJSON* json) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Cannot open file");
        return -1;
    }
    char* json_string = cJSON_Print(json);
    fwrite(json_string, 1, strlen(json_string), file);
    fclose(file);
    cJSON_free(json_string);
    return 0;
}

// Funzione per aggiungere una notifica non consegnata al file
void save_notification(const char* username, const char* isbn, const char* message) {
    cJSON* notifications = read_json(NOTIF_FILE);
    if (!notifications) {
        notifications = cJSON_CreateArray();
    }

    cJSON* notification = cJSON_CreateObject();
    cJSON_AddStringToObject(notification, "username", username);
    cJSON_AddStringToObject(notification, "isbn", isbn);
    cJSON_AddStringToObject(notification, "message", message);

    cJSON_AddItemToArray(notifications, notification);

    write_json(NOTIF_FILE, notifications);
    cJSON_Delete(notifications);
}

// Funzione per inviare una notifica al client
void notify_user(const char* username, const char* isbn) {
    // Simulazione di invio notifica
    printf("Notifica a %s: Restituisci il libro ISBN %s. La scadenza è stata superata.\n", username, isbn);

    // Se l'utente non è connesso, salvare la notifica
    // Per esempio: il client non risponde o non è collegato
    int client_connected = 0; // Simulazione
    if (!client_connected) {
        char message[SIZE_BUF];
        snprintf(message, SIZE_BUF, "Restituisci il libro ISBN %s. La scadenza è stata superata.", isbn);
        save_notification(username, isbn, message);
    }
}

// Funzione per controllare i prestiti scaduti
void check_overdue_loans() {
    cJSON* loans = read_json(LOAN_FILE);
    if (!loans) {
        return;
    }

    time_t now = time(NULL);

    cJSON* loan = NULL;
    cJSON_ArrayForEach(loan, loans) {
        cJSON* return_date = cJSON_GetObjectItem(loan, "return_date");
        cJSON* username = cJSON_GetObjectItem(loan, "username");
        cJSON* isbn = cJSON_GetObjectItem(loan, "isbn");

        time_t due_time = parse_date(return_date->valuestring);
        if (now > due_time) {
            notify_user(username->valuestring, isbn->valuestring);
        }
    }

    cJSON_Delete(loans);
}

// Funzione eseguita dal thread per il controllo periodico delle scadenze
void* overdue_check_thread(void* arg) {
    while (1) {
        printf("Esecuzione del controllo delle scadenze...\n");
        check_overdue_loans();
        sleep(CHECK_INTERVAL); // Attendere l'intervallo di controllo
    }
    return NULL;
}

// Funzione per inviare notifiche pendenti all'utente quando si riconnette
void send_pending_notifications(const char* username) {
    cJSON* notifications = read_json(NOTIF_FILE);
    if (!notifications) {
        return;
    }

    cJSON* notif = NULL;
    cJSON* new_notifs = cJSON_CreateArray();

    cJSON_ArrayForEach(notif, notifications) {
        cJSON* notif_user = cJSON_GetObjectItem(notif, "username");
        cJSON* message = cJSON_GetObjectItem(notif, "message");

        if (strcmp(notif_user->valuestring, username) == 0) {
            // Simulazione invio notifica
            printf("Invio notifica a %s: %s\n", username, message->valuestring);
        } else {
            cJSON_AddItemToArray(new_notifs, cJSON_Duplicate(notif, 1));  // Mantieni le altre notifiche
        }
    }

    write_json(NOTIF_FILE, new_notifs);
    cJSON_Delete(notifications);
    cJSON_Delete(new_notifs);
}

int main() {
    pthread_t thread_id;

    // Creazione del thread per il controllo delle scadenze
    if (pthread_create(&thread_id, NULL, overdue_check_thread, NULL) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // Esempio di utente che si riconnette
    const char* username = "Raziel";
    send_pending_notifications(username);

    // Il server può continuare ad accettare connessioni e altre operazioni
    printf("Server in esecuzione...\n");

    // Attendere la terminazione del thread (anche se in questo caso non si fermerà mai)
    pthread_join(thread_id, NULL);

    return 0;
}