#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

char database_file_path[MAX_PATH];
cJSON *database = NULL;

void update_database_file()
{
    char *json = cJSON_Print(database);
    FILE *file = fopen(database_file_path, "w");
    if (file)
    {
        fprintf(file, "%s", json);
        fclose(file);
    }
    free(json);
}

void load_database()
{
    FILE *file = fopen(database_file_path, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *data = (char *)malloc(length + 1);
        fread(data, 1, length, file);
        data[length] = '\0';
        database = cJSON_Parse(data);
        free(data);
        fclose(file);
    }
    else
    {
        database = cJSON_CreateObject();
        cJSON_AddItemToObject(database, "postman", cJSON_CreateArray());
        cJSON *postman_array = cJSON_GetObjectItem(database, "postman");
        cJSON *default_user = cJSON_CreateObject();
        cJSON_AddStringToObject(default_user, "username", "default");
        cJSON_AddStringToObject(default_user, "password", "default");
        cJSON *default_user_parcels = cJSON_CreateArray();
        cJSON_AddItemToObject(default_user, "parcels", default_user_parcels);
        cJSON_AddItemToArray(postman_array, default_user);
        update_database_file();
    }
}

void reply(SOCKET sock, const char *status, cJSON *message)
{
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", status);
    cJSON_AddItemToObject(response, "message", message);

    char *response_str = cJSON_PrintUnformatted(response);
    send(sock, response_str, strlen(response_str), 0);

    free(response_str);
    cJSON_Delete(response);
}

int login(const char *username, const char *password, int *info_index)
{
    cJSON *postman = cJSON_GetObjectItem(database, "postman");
    if (!cJSON_IsArray(postman))
        return 0;

    int index = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, postman)
    {
        cJSON *user = cJSON_GetObjectItem(item, "username");
        cJSON *pass = cJSON_GetObjectItem(item, "password");

        if (user && pass && strcmp(user->valuestring, username) == 0)
        {
            if (strcmp(pass->valuestring, password) == 0)
            {
                *info_index = index;
                return 1;
            }
        }
        index++;
    }
    return 0;
}

void handle_add(SOCKET sock, int info_index, const char *id, cJSON *parameter)
{
    cJSON *postman = cJSON_GetObjectItem(database, "postman");
    cJSON *user = cJSON_GetArrayItem(postman, info_index);
    cJSON *parcels = cJSON_GetObjectItem(user, "parcels");

    cJSON *parcel;
    cJSON_ArrayForEach(parcel, parcels)
    {
        cJSON *pid = cJSON_GetObjectItem(parcel, "id");
        if (pid && strcmp(pid->valuestring, id) == 0)
        {
            reply(sock, "error", cJSON_CreateString("parcel already exists"));
            return;
        }
    }

    cJSON *new_parcel = cJSON_CreateObject();
    cJSON_AddStringToObject(new_parcel, "id", id);
    cJSON_AddItemToObject(new_parcel, "parameter", cJSON_Duplicate(parameter, 1));
    cJSON_AddItemToArray(parcels, new_parcel);

    update_database_file();
    reply(sock, "success", cJSON_CreateString("parcel added"));
}

void handle_delete(SOCKET sock, int info_index, const char *id)
{
    cJSON *postman = cJSON_GetObjectItem(database, "postman");
    cJSON *user = cJSON_GetArrayItem(postman, info_index);
    cJSON *parcels = cJSON_GetObjectItem(user, "parcels");

    int index = 0;
    cJSON *parcel;
    cJSON_ArrayForEach(parcel, parcels)
    {
        cJSON *pid = cJSON_GetObjectItem(parcel, "id");
        if (pid && strcmp(pid->valuestring, id) == 0)
        {
            cJSON_DeleteItemFromArray(parcels, index);
            update_database_file();
            reply(sock, "success", cJSON_CreateString("parcel deleted"));
            return;
        }
        index++;
    }
    reply(sock, "error", cJSON_CreateString("parcel not found"));
}

void handle_list(SOCKET sock, int info_index)
{
    cJSON *postman = cJSON_GetObjectItem(database, "postman");
    cJSON *user = cJSON_GetArrayItem(postman, info_index);
    cJSON *parcels = cJSON_GetObjectItem(user, "parcels");
    // #test
    {
        char *parcels_str = cJSON_PrintUnformatted(parcels);
        printf("parcels: %s\n", parcels_str);
        free(parcels_str);
    }
    reply(sock, "success", parcels);
}

void handle_signup(SOCKET sock, const char *username, const char *password)
{
    cJSON *postman = cJSON_GetObjectItem(database, "postman");

    cJSON *item;
    cJSON_ArrayForEach(item, postman)
    {
        cJSON *user = cJSON_GetObjectItem(item, "username");
        if (user && strcmp(user->valuestring, username) == 0)
        {
            reply(sock, "error", cJSON_CreateString("username already exists"));
            return;
        }
    }

    cJSON *new_user = cJSON_CreateObject();
    cJSON_AddStringToObject(new_user, "username", username);
    cJSON_AddStringToObject(new_user, "password", password);
    cJSON_AddItemToObject(new_user, "parcels", cJSON_CreateArray());
    cJSON_AddItemToArray(postman, new_user);

    update_database_file();
    reply(sock, "success", cJSON_CreateString("signup success"));
}

void handle_get(SOCKET sock, int info_index)
{
    // #test
    {
        printf("enter handle_get\n");
    }

    cJSON *postman = cJSON_GetObjectItem(database, "postman");
    cJSON *user = cJSON_GetArrayItem(postman, info_index);
    cJSON *parcels = cJSON_GetObjectItem(user, "parcels");

    // #test
    {
        char *parcels_str = cJSON_PrintUnformatted(parcels);
        printf("enter: parcels: %s\n", parcels_str);
        free(parcels_str);
    }

    char task_id[10];
    do
    {
        int task_id_int = (int)((double)rand() / RAND_MAX * 1000000000);
        snprintf(task_id, sizeof(task_id), "%09d", task_id_int);

        int exists = 0;

        cJSON *parcel;
        cJSON_ArrayForEach(parcel, parcels)
        {
            cJSON *parcel_id = cJSON_GetObjectItem(parcel, "id");
            if (parcel_id && strcmp(parcel_id->valuestring, task_id) == 0)
            {
                exists = 1;
                break;
            }
        }

        // #test
        {
            printf("not found parcel id collision\n");
        }

        if (!exists)
        {
            cJSON *new_parcel = cJSON_CreateObject();
            cJSON_AddStringToObject(new_parcel, "id", task_id);
            cJSON_AddItemToObject(new_parcel, "parameter", cJSON_CreateObject());

            // #test
            {
                char *new_parcel_str = cJSON_PrintUnformatted(new_parcel);
                printf("new_parcel: %s\n", new_parcel_str);
                free(new_parcel_str);
            }

            cJSON_AddItemToArray(parcels, new_parcel);
            update_database_file();
            reply(sock, "success", cJSON_CreateString("parcel added"));
            break;
        }
    } while (1);

    // #test
    {
        char *parcels_str = cJSON_PrintUnformatted(parcels);
        printf("exit: parcels: %s\n", parcels_str);
        free(parcels_str);
    }
}

void handle_postman_request(SOCKET sock, int info_index, cJSON *request)
{
    cJSON *type = cJSON_GetObjectItem(request, "type");
    cJSON *arguments = cJSON_GetObjectItem(request, "arguments");

    if (strcmp(type->valuestring, "add") == 0)
    {
        cJSON *id = cJSON_GetObjectItem(arguments, "id");
        cJSON *param = cJSON_GetObjectItem(arguments, "parcel_parameter");
        handle_add(sock, info_index, id->valuestring, param);
    }
    else if (strcmp(type->valuestring, "delete") == 0)
    {
        cJSON *id = cJSON_GetObjectItem(arguments, "id");
        handle_delete(sock, info_index, id->valuestring);
    }
    else if (strcmp(type->valuestring, "list") == 0)
    {
        handle_list(sock, info_index);
    }
    else if (strcmp(type->valuestring, "get") == 0)
    {
        handle_get(sock, info_index);
    }
    else
    {
        reply(sock, "error", cJSON_CreateString("invalid request type"));
    }
}

void handle_postman(SOCKET sock, cJSON *data)
{
    cJSON *user = cJSON_GetObjectItem(data, "user");
    cJSON *username = cJSON_GetObjectItem(user, "username");
    cJSON *password = cJSON_GetObjectItem(user, "password");

    int info_index;
    if (login(username->valuestring, password->valuestring, &info_index))
    {
        cJSON *request = cJSON_GetObjectItem(data, "request");
        handle_postman_request(sock, info_index, request);
    }
    else
    {
        reply(sock, "error", cJSON_CreateString("login failed"));
    }
}

void process_request(SOCKET sock, cJSON *data)
{
    cJSON *user = cJSON_GetObjectItem(data, "user");
    cJSON *type = cJSON_GetObjectItem(user, "type");

    if (strcmp(type->valuestring, "postman") == 0)
    {
        handle_postman(sock, data);
    }
    else
    {
        reply(sock, "error", cJSON_CreateString("invalid user type"));
    }
}

// 新增函数：解析 Request Format Alternative
void parse_alternative_request(char *buffer, int buffer_size, char **username, char **password, char **request_type, char **arguments, int *arguments_length)
{
    int offset = 0;

    // 解析 header_length 和 request_length
    int header_length = *(int *)(buffer + offset);
    offset += sizeof(int);
    int request_length = *(int *)(buffer + offset);
    offset += sizeof(int);

    // 解析 username 和 password
    int username_length = *(int *)(buffer + offset);
    offset += sizeof(int);
    *username = (char *)malloc(username_length + 1);
    memcpy(*username, buffer + offset, username_length);
    (*username)[username_length] = '\0';
    offset += username_length;

    int password_length = *(int *)(buffer + offset);
    offset += sizeof(int);
    *password = (char *)malloc(password_length + 1);
    memcpy(*password, buffer + offset, password_length);
    (*password)[password_length] = '\0';
    offset += password_length;

    // 解析 request_type 和 arguments
    int user_type_enum = *(int *)(buffer + offset);
    offset += sizeof(int);

    int arguments_length_value = *(int *)(buffer + offset);
    offset += sizeof(int);
    *arguments_length = arguments_length_value;

    *arguments = (char *)malloc(arguments_length_value);
    memcpy(*arguments, buffer + offset, arguments_length_value);
}

// 新增函数：处理 postman 请求（基于 Request Format Alternative）
void handle_alternative_postman_request(SOCKET sock, int info_index, char *request_type, char *arguments, int arguments_length)
{
    if (strcmp(request_type, "add") == 0)
    {
        int id_length = *(int *)arguments;
        char *id = (char *)malloc(id_length + 1);
        memcpy(id, arguments + sizeof(int), id_length);
        id[id_length] = '\0';

        int param_length = *(int *)(arguments + sizeof(int) + id_length);
        char *param = (char *)malloc(param_length);
        memcpy(param, arguments + sizeof(int) + id_length + sizeof(int), param_length);

        handle_add(sock, info_index, id, cJSON_Parse(param));
        free(id);
        free(param);
    }
    else if (strcmp(request_type, "delete") == 0)
    {
        int id_length = *(int *)arguments;
        char *id = (char *)malloc(id_length + 1);
        memcpy(id, arguments + sizeof(int), id_length);
        id[id_length] = '\0';

        handle_delete(sock, info_index, id);
        free(id);
    }
    else if (strcmp(request_type, "list") == 0)
    {
        handle_list(sock, info_index);
    }
    else if (strcmp(request_type, "get") == 0)
    {
        handle_get(sock, info_index);
    }
    else
    {
        reply(sock, "error", cJSON_CreateString("invalid request type"));
    }
}

// 新增函数：处理 Request Format Alternative 请求
void process_alternative_request(SOCKET sock, char *buffer, int buffer_size)
{
    char *username = NULL;
    char *password = NULL;
    char *request_type = NULL;
    char *arguments = NULL;
    int arguments_length = 0;

    parse_alternative_request(buffer, buffer_size, &username, &password, &request_type, &arguments, &arguments_length);

    int info_index;
    if (login(username, password, &info_index))
    {
        handle_alternative_postman_request(sock, info_index, request_type, arguments, arguments_length);
    }
    else
    {
        reply(sock, "error", cJSON_CreateString("login failed"));
    }

    free(username);
    free(password);
    free(request_type);
    free(arguments);
}

// 修改主循环：增加对 Request Format Alternative 的支持
int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    srand((unsigned int)time(NULL));

    GetModuleFileNameA(NULL, database_file_path, MAX_PATH);
    char *last_slash = strrchr(database_file_path, '\\');
    if (last_slash)
    {
        *(last_slash + 1) = '\0';
        strcat(database_file_path, "data.json");
    }
    else
    {
        strcpy(database_file_path, "data.json");
    }
    load_database();

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    bind(server_socket, (SOCKADDR *)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    while (1)
    {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET)
            continue;

        char buffer[BUFFER_SIZE];
        int recv_size;
        while ((recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
        {
            buffer[recv_size] = '\0';

            // 判断是否为 Request Format Alternative
            if (*(int *)buffer == sizeof(int))
            {
                process_alternative_request(client_socket, buffer, recv_size);
            }
            else
            {
                cJSON *request = cJSON_Parse(buffer);
                if (!request)
                {
                    reply(client_socket, "error", cJSON_CreateString("invalid JSON"));
                    continue;
                }
                process_request(client_socket, request);
                cJSON_Delete(request);
            }
        }
        closesocket(client_socket);
    }

    cJSON_Delete(database);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
