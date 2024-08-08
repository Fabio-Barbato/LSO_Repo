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

void extract_and_print_data(cJSON* json) {
    cJSON *name = cJSON_GetObjectItem(json, "name");
    cJSON *age = cJSON_GetObjectItem(json, "age");
    cJSON *is_student = cJSON_GetObjectItem(json, "is_student");
    cJSON *scores = cJSON_GetObjectItem(json, "scores");

    if (cJSON_IsString(name) && (name->valuestring != NULL)) {
        printf("Name: %s\n", name->valuestring);
    }

    if (cJSON_IsNumber(age)) {
        printf("Age: %d\n", age->valueint);
    }

    if (cJSON_IsBool(is_student)) {
        printf("Is student: %s\n", cJSON_IsTrue(is_student) ? "true" : "false");
    }

    if (cJSON_IsArray(scores)) {
        int score_count = cJSON_GetArraySize(scores);
        printf("Scores: ");
        for (int i = 0; i < score_count; i++) {
            cJSON *score = cJSON_GetArrayItem(scores, i);
            if (cJSON_IsNumber(score)) {
                printf("%d ", score->valueint);
            }
        }
        printf("\n");
    }
}

cJSON* create_sample_json() {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "name", "John Doe");
    cJSON_AddNumberToObject(json, "age", 30);
    cJSON_AddBoolToObject(json, "is_student", 0);

    cJSON *scores = cJSON_CreateIntArray((const int[]){95, 85, 76}, 3);
    cJSON_AddItemToObject(json, "scores", scores);

    return json;
}

#endif 