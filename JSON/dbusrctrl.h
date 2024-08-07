#ifndef dbusrctrl
#define dbusrctrl

#include "jsonparse.h"
#define FILENAME "usersDB.json"

int add_user(const char* name, const char* username, const char* password) {
    cJSON *json = read_json(FILENAME);
    if (!json) {
        json = cJSON_CreateArray();
    }

    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "username", username);
    cJSON_AddStringToObject(user, "password", password);

    cJSON_AddItemToArray(json, user);

    int result = write_json(FILENAME, json);
    cJSON_Delete(json);
    return result;
}

#endif