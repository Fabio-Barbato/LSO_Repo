#ifndef bkctrl
#define bkctrl

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "bkctrl.h"
#include "jsonparse.h"
#define BK "booksDB.json"

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

/*int add_book(const char* title, const char* isbn, const char* author, const char* genre, const int* copies) {
    cJSON *json = read_json(BK);
    if (!json) {
        perror("No JSON file\n");
        return -1;
    }

    if(search_book(isbn)){
        perror("book already exists!\n");
        return -1;
    }

    cJSON *book = cJSON_CreateObject();
    cJSON_AddStringToObject(book, "title", title);
    cJSON_AddStringToObject(book, "isbn", isbn);
    cJSON_AddStringToObject(book, "author", author);
    cJSON_AddStringToObject(book, "genre", genre);
    cJSON_AddStringToObject(book, "copies", copies);
    
    cJSON_AddItemToArray(json, book);

    int result = write_json(BK, json);
    cJSON_Delete(json);
    return result;
}

int delete_book(const char* isbn) {
    cJSON *json = read_json(BK);
    if (!json) {
        return -1;
    }

    cJSON *book = NULL;
    int index = 0;
    cJSON *books_array = json;

    cJSON_ArrayForEach(book, books_array) {
        cJSON *book_isbn = cJSON_GetObjectItem(book, "isbn");
        if (cJSON_IsString(book_isbn) && (strcmp(book_isbn->valuestring, isbn) == 0)) {
            cJSON_DeleteItemFromArray(books_array, index);
            int result = write_json(BK, json);
            cJSON_Delete(json);
            return result;
        }
        index++;
    }

    cJSON_Delete(json);
    return -1;
}*/

int update_copies(const char* isbn, const int new_copies) {
    cJSON *json = read_json(BK);
    if (!json) {
        return -1;
    }

    cJSON *book = NULL;
    cJSON *books_array = json;

    cJSON_ArrayForEach(book, books_array) {
        cJSON *book_isbn = cJSON_GetObjectItem(book, "isbn");
        if (cJSON_IsString(book_isbn) && (strcmp(book_isbn->valuestring, isbn) == 0)) {
            if (new_copies) {
                cJSON *copies = cJSON_GetObjectItem(book, "copies");
                cJSON_SetIntValue(copies,new_copies);
            }

            int result = write_json(BK, json);
            cJSON_Delete(json);
            return result;
        }
    }

    cJSON_Delete(json);
    return -1;
}


int add_copies(const char* isbn, const int add) { //0 add, 1 remove a copy
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
            int new_copies = cJSON_GetNumberValue(copies);

            if(add==0)
                new_copies++;
            else
                new_copies--;

            cJSON_SetIntValue(copies,new_copies);
            int result = write_json(BK, json);
            cJSON_Delete(json);
            return result;
        }
    }

    cJSON_Delete(json);
    return -1;
}

#endif