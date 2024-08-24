#ifndef usrctrl
#define usrctrl

#include <stdlib.h>
#include <stdio.h>
#include "usrctrl.h"
#include "jsonparse.h"
#define USR "Data/users.json"


cJSON* search_user(const char* username) { //use cJSON_Print(cJSON*) to print it
    cJSON *json = read_json(USR);
    if (!json) {
        return NULL;
    }

    cJSON *user = NULL;
    cJSON *users_array = json;

    cJSON_ArrayForEach(user, users_array) {
        cJSON *user_username = cJSON_GetObjectItem(user, "username");
        if (cJSON_IsString(user_username) && (strcmp(user_username->valuestring, username) == 0)) {
            cJSON *user_copy = cJSON_Duplicate(user, 1);
            cJSON_Delete(json);
            return user_copy;
        }
    }

    cJSON_Delete(json);
    return NULL;
}

int add_user(const char* name, const char* surname, const char* username, const char* password) {
    cJSON *json = read_json(USR);
    if (!json) {
        printf("No JSON file\n");
        return -1;
    }

    if(search_user(username)){
        printf("User already exists!\n");
        return -1;
    }

    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "surname", surname);
    cJSON_AddStringToObject(user, "username", username);
    cJSON_AddStringToObject(user, "password", password);

    cJSON_AddItemToArray(json, user);

    int result = write_json(USR, json);
    cJSON_Delete(json);
    return result;
}

int delete_user(const char* username) {
    cJSON *json = read_json(USR);
    if (!json) {
        return -1;
    }

    cJSON *user = NULL;
    int index = 0;
    cJSON *users_array = json;

    cJSON_ArrayForEach(user, users_array) {
        cJSON *user_username = cJSON_GetObjectItem(user, "username");
        if (cJSON_IsString(user_username) && (strcmp(user_username->valuestring, username) == 0)) {
            cJSON_DeleteItemFromArray(users_array, index);
            int result = write_json(USR, json);
            cJSON_Delete(json);
            return result;
        }
        index++;
    }

    cJSON_Delete(json);
    return -1;
}

int update_user(const char* username, const char* new_name, const char* new_surname, const char* new_username, const char* new_password) {
    cJSON *json = read_json(USR);
    if (!json) {
        return -1;
    }

    cJSON *user = NULL;
    cJSON *users_array = json;

    cJSON_ArrayForEach(user, users_array) {
        cJSON *user_username = cJSON_GetObjectItem(user, "username");
        if (cJSON_IsString(user_username) && (strcmp(user_username->valuestring, username) == 0)) {
            if (new_name) {
                cJSON *name = cJSON_GetObjectItem(user, "name");
                cJSON_SetValuestring(name, new_name);
            }
            if (new_surname) {
                cJSON *surname = cJSON_GetObjectItem(user, "surname");
                cJSON_SetValuestring(surname, new_surname);
            }
            if (new_username) {
                cJSON_SetValuestring(user_username, new_username);
            }
            if (new_password) {
                cJSON *password = cJSON_GetObjectItem(user, "password");
                cJSON_SetValuestring(password, new_password);
            }

            int result = write_json(USR, json);
            cJSON_Delete(json);
            return result;
        }
    }

    cJSON_Delete(json);
    return -1;
}

int login(const char* username,const char* password){
    cJSON *json = read_json(USR);
    if (!json) {
        return -1;
    }

    cJSON *user = NULL;
    cJSON *users_array = json;

    cJSON_ArrayForEach(user, users_array) {
        cJSON *user_username = cJSON_GetObjectItem(user, "username");
        cJSON *user_passwd = cJSON_GetObjectItem(user, "password");
        if (cJSON_IsString(user_username)){ 
            if(strcmp(user_username->valuestring, username) == 0 && strcmp(user_passwd->valuestring, password)==0) {
                cJSON_Delete(json);
                return 0;
            }
        }
    }
    cJSON_Delete(json);
    return -1;
}

#endif