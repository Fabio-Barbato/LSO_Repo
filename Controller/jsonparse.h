#ifndef jsonparse
#define jsonparse

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }

    fclose(file);
    return content;
}

int write_file(const char* filename, const char* data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Could not open file for writing");
        return -1;
    }

    fputs(data, file);
    fclose(file);
    return 0;
}

cJSON* read_json(const char* filename) {
    char *file_content = read_file(filename);
    if (!file_content) {
        return NULL;
    }

    cJSON *json = cJSON_Parse(file_content);
    free(file_content);

    if (!json) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return NULL;
    }

    return json;
}

int write_json(const char* filename, cJSON* json) {
    char *json_string = cJSON_Print(json);
    if (!json_string) {
        perror("Failed to print JSON");
        return -1;
    }

    int result = write_file(filename, json_string);
    free(json_string);
    return result;
}

#endif 