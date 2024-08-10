#ifndef loanctrl
#define loanctrl

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "loanctrl.h"
#include "jsonparse.h"
#define LOAN "Data/loansDB.json"
#define SIZE_BUF_MIN 16
#define DAYS_TO_RET 14

int count_loans(const char* username) {
    cJSON *json = read_json(LOAN);
    if (!json) {
        return -1;
    }

    cJSON *loan = NULL;
    cJSON *loans_array = json;
    int counter=0;

    cJSON_ArrayForEach(loan, loans_array) {
        cJSON *user_username = cJSON_GetObjectItem(loan, "username");
        if ((strcmp(user_username->valuestring, username) == 0)) {
            counter++;
        }
    }

    cJSON_Delete(json);
    return counter;
}

int add_loan(const char* username, const char* isbn) {
    time_t current_time;
    struct tm tm_info;
    char loan_date[SIZE_BUF_MIN];
    char return_date[SIZE_BUF_MIN];

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
    strftime(loan_date, SIZE_BUF_MIN, "%d-%m-%Y", &tm_info);
    cJSON_AddStringToObject(loan, "loan_date", loan_date);

    tm_info.tm_mday += DAYS_TO_RET;
    mktime(&tm_info);
    strftime(return_date, SIZE_BUF_MIN, "%d-%m-%Y", &tm_info);
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