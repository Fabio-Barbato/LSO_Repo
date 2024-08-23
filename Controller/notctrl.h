#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "cJSON.h"

#define SIZE_BUF 1024
#define LOAN_FILE "Data/loans.json"
#define NOTIF_FILE "Data/notifications.json"
#define CHECK_INTERVAL 86400 // 1 day in seconds

time_t parse_date(const char* date_str) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    strptime(date_str, "%d-%m-%Y", &tm);
    return mktime(&tm);
}

int save_notification(const char* username, const char* isbn) {
    cJSON* notifications = read_json(NOTIF_FILE);
    if (!notifications) {
        return -1;
    }

    cJSON* notification = cJSON_CreateObject();
    char message[SIZE_BUF];
    snprintf(message, SIZE_BUF, "Dear %s, the loan period has expired. You must return the book with ISBN %s.\n", username, isbn);

    cJSON_AddStringToObject(notification, "username", username);
    cJSON_AddStringToObject(notification, "isbn", isbn);
    cJSON_AddStringToObject(notification, "message", message);
    cJSON_AddBoolToObject(notification, "notified", 0);

    cJSON_AddItemToArray(notifications, notification);

    write_json(NOTIF_FILE, notifications);
    cJSON_Delete(notifications);

    return 0;
}

int notify_user(const char* username, int client_socket) {
    cJSON *json = read_json(NOTIF_FILE);
    if (!json) {
        printf("Can't read file json file for notifications\n");
        return -1;
    }

    cJSON *notification = NULL;
    cJSON *notifications_array = json;

    cJSON_ArrayForEach(notification, notifications_array) {
        cJSON *user_username = cJSON_GetObjectItem(notification, "username");
        cJSON *notified = cJSON_GetObjectItem(notification, "notified");

        if ((strcmp(user_username->valuestring, username) == 0) && !cJSON_IsTrue(notified)) {
            printf("Sending notification to %s...\n", username);
            cJSON *message = cJSON_GetObjectItem(notification, "message");
            send(client_socket, message->valuestring, strlen(message->valuestring), 0);

            cJSON_ReplaceItemInObject(notification, "notified", cJSON_CreateBool(1));
        }
    }

    write_json(NOTIF_FILE, json);
    cJSON_Delete(json);
    return 0;
}

void reset_notifications() {
    cJSON *json = read_json(NOTIF_FILE);
    if (!json) {
        printf("Can't read file json file for notifications\n");
        return;
    }

    cJSON *notification = NULL;
    cJSON *notifications_array = json;

    printf("Resetting notifications...\n");
    cJSON_ArrayForEach(notification, notifications_array) {
        cJSON_ReplaceItemInObject(notification, "notified", cJSON_CreateBool(0));
    }

    write_json(NOTIF_FILE, json);
    cJSON_Delete(json);
}

int search_notification(const char* username, const char* isbn) {
    cJSON *json = read_json(NOTIF_FILE);
    if (!json) {
        printf("Can't read file json file for notifications\n");
        return -1;
    }

    cJSON *notification = NULL;
    cJSON *notifications_array = json;

    cJSON_ArrayForEach(notification, notifications_array) {
        cJSON *user_username = cJSON_GetObjectItem(notification, "username");
        cJSON *book_isbn = cJSON_GetObjectItem(notification, "isbn");
        if (user_username && book_isbn && 
            (strcmp(user_username->valuestring, username) == 0) && 
            (strcmp(book_isbn->valuestring, isbn) == 0)) {
            printf("Notification already exists for user %s and ISBN %s\n", username, isbn);
            cJSON_Delete(json);
            return 1;
        }
    }

    printf("No notifications for user %s and ISBN %s. Saving...\n", username, isbn);
    cJSON_Delete(json);
    return 0;
}

int check_overdue_loans() {
    cJSON* json = read_json(LOAN_FILE);
    if (!json) {
        printf("Error reading loans\n");
        return -1;
    }

    time_t now = time(NULL);

    cJSON* loan = NULL;
    cJSON* loan_array = json;
    cJSON_ArrayForEach(loan, loan_array) {
        cJSON* return_date = cJSON_GetObjectItem(loan, "return_date");
        cJSON* username = cJSON_GetObjectItem(loan, "username");
        cJSON* isbn = cJSON_GetObjectItem(loan, "isbn");
        
        // if (!return_date || !username || !isbn) {
        //     printf("Error: Missing data in loan record\n");
        //     continue;
        // }

        time_t due_time = parse_date(return_date->valuestring);
        if (now > due_time) {
            int result = search_notification(username->valuestring, isbn->valuestring);
            if(result == 0) {
                printf("Saving notification for %s late to return the book with ISBN %s...\n", username->valuestring, isbn->valuestring);
                save_notification(username->valuestring, isbn->valuestring);
            }
        }
    }

    cJSON_Delete(loan_array);

    return 0;
}

void* overdue_check(void* arg) {
    int result;
    reset_notifications();
    while (1) {
        printf("Checking overdue loans...\n");
        result = check_overdue_loans();
        if(result<0)
             printf("Error notifications...\n");
        sleep(CHECK_INTERVAL);
    }

    return NULL;
}