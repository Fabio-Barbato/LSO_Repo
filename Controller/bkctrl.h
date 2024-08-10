#ifndef bkctrl
#define bkctrl

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "bkctrl.h"
#include "jsonparse.h"
#define BK "Data/booksDB.json"

cJSON* search_book(const char* isbn) { //use cJSON_Print(cJSON*) to print it
    cJSON *json = read_json(BK);
    if (!json) {
        return NULL;
    }

    cJSON *book = NULL;
    cJSON *books_array = json;

    cJSON_ArrayForEach(book, books_array) {
        cJSON *book_isbn = cJSON_GetObjectItem(book, "isbn");
        if (cJSON_IsString(book_isbn) && (strcmp(book_isbn->valuestring, isbn) == 0)) {
            cJSON *book_copy = cJSON_Duplicate(book, 1);
            cJSON_Delete(json);
            return book_copy;
        }
    }

    cJSON_Delete(json);
    return NULL;
}

int loan(const char* isbn, const int add) { //0 add, else loan
    cJSON *json = read_json(BK);
    if (!json) {
        return -1;
    }

    cJSON *book = NULL;
    cJSON *books_array = json;

    cJSON_ArrayForEach(book, books_array) {
        cJSON *book_isbn = cJSON_GetObjectItem(book, "isbn");
        if (cJSON_IsString(book_isbn) && (strcmp(book_isbn->valuestring, isbn) == 0)) {
            cJSON *copies = cJSON_GetObjectItem(book, "copies");
            cJSON *given_copies = cJSON_GetObjectItem(book, "given_copies");
            int new_copies = cJSON_GetNumberValue(copies);
            int new_givcopies = cJSON_GetNumberValue(given_copies);

            if(add==0){ 
                new_givcopies = (new_givcopies==0) ? 0:new_givcopies-1; //add or return
                new_copies++;  
            }
            else{ //loan
                if(new_copies==0){
                    printf("No copies available of the book %s\n",isbn);
                    return -1;
                }
                new_copies--;
                new_givcopies++;
            }
            cJSON_SetIntValue(copies,new_copies);
            cJSON_SetIntValue(given_copies,new_givcopies);
            int result = write_json(BK, json);
            cJSON_Delete(json);
            return result;
        }
    }

    cJSON_Delete(json);
    return -1;
}

int checkout(char *isbn[], int length){
    int result = 0, i = 0;

    while (i<length && result==0)
    {
        result = loan(isbn[i],1);
        i++;
    }

    return result;
}

#endif