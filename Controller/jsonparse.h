#ifndef jsonparse
#define jsonparse

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file\n");
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
        printf("Could not open file for writing\n");
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
        printf("Failed to print JSON\n");
        return -1;
    }

    int result = write_file(filename, json_string);
    free(json_string);
    return result;
}

char* jsonToString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

#endif 