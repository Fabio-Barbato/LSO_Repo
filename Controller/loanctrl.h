#ifndef loanctrl
#define loanctrl

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "loanctrl.h"
#include "jsonparse.h"
#define LOAN "Data/loansDB.json"

// cJSON* search_loan(const char* username, const char* isbn) { //use cJSON_Print(cJSON*) to print it, search loan by user and book
//     cJSON *json = read_json(LOAN);
//     if (!json) {
//         return NULL;
//     }

//     cJSON *loan = NULL;
//     cJSON *users_array = json;

//     cJSON_ArrayForEach(user, users_array) {
//         cJSON *user_username = cJSON_GetObjectItem(loan, "username");
//         cJSON *book_isbn = cJSON_GetObjectItem(loan, "isbn");
//         if ((strcmp(user_username->valuestring, username) == 0) && strcmp(book_isbn->valuestring, isbn) == 0) {
//             cJSON *loan_copy = cJSON_Duplicate(loan, 1);
//             cJSON_Delete(json);
//             return loan_copy;
//         }
//     }

//     cJSON_Delete(json);
//     return NULL;
// }  return array of loans

int add_loan(const char* username, const char* isbn) {
    time_t current_time;
    struct tm tm_info;
    char loan_date[16];
    char return_date[16];

    cJSON *json = read_json(LOAN);
    if (!json) {
        perror("No JSON file\n");
        return -1;
    }

    time(&current_time);
    tm_info = *localtime(&current_time);

    cJSON *loan = cJSON_CreateObject();
    cJSON_AddStringToObject(loan, "username", username);
    cJSON_AddStringToObject(loan, "isbn", isbn);
    strftime(loan_date, 16, "%d-%m-%Y", &tm_info);
    cJSON_AddStringToObject(loan, "loan_date", loan_date);
    tm_info.tm_mon += 1;
    mktime(&tm_info);
    strftime(return_date, 16, "%d-%m-%Y", &tm_info);
    cJSON_AddStringToObject(loan, "return_date", return_date);

    cJSON_AddItemToArray(json, loan);

    int result = write_json(LOAN, json);
    cJSON_Delete(json);
    return result;
}

int delete_loan(const char* username, const char* isbn) {
    cJSON *json = read_json(LOAN);
    if (!json) {
        return -1;
    }

    cJSON *loan = NULL;
    int index = 0;
    cJSON *loans_array = json;

    cJSON_ArrayForEach(loan, loans_array) {
        cJSON *loan_user = cJSON_GetObjectItem(loan, "username");
        cJSON *loan_book = cJSON_GetObjectItem(loan, "isbn");
        if ((strcmp(loan_user->valuestring, username) == 0) && (strcmp(loan_book->valuestring, isbn) == 0)) {
            cJSON_DeleteItemFromArray(loans_array, index);
            int result = write_json(LOAN, json);
            cJSON_Delete(json);
            return result;
        }
        index++;
    }

    cJSON_Delete(json);
    return -1;
}


#endif