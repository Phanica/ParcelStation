#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma pack(push, 1)

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_TASK_ID_LENGTH 32

typedef enum UserType
{
    postman,
    manager,
    user,
} UserType;

typedef enum RequestType
{
    signup,
    login,
    logout,
    get,
    list,
    assign,
    report_loss,
} RequestType;

typedef struct User
{
    UserType type;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

typedef struct SignupLoginArguments
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} SignupLoginArguments;

typedef struct AssignReportArguments
{
    char id[MAX_TASK_ID_LENGTH];
} AssignReportArguments;

typedef union RequestArguments
{
    SignupLoginArguments signup_login;
    AssignReportArguments assign_report;
} RequestArguments;

typedef struct Request
{
    RequestType type;
    RequestArguments arguments;
} Request;

typedef struct Message
{
    User user;
    Request request;
} Message;

typedef struct Response
{
    char status[32];
    char message[256];
} Response;

typedef struct ClientData
{
    char username[32];
    char password[32];
    int has_login;
} ClientData;

#pragma pack(pop)

#define MAX_MESSAGE_SIZE 2048
#define MAX_RESPONSE_SIZE 2048
#define DATA_FILE "data.dat"

SOCKET sock;
WSADATA wsaData;
ClientData client_data;

size_t strsize(const char *str)
{
    return strlen(str) + 1;
}

void serialize_message(const Message *msg, char *buffer)
{
    int username_length = strsize(msg->user.username);
    int password_length = strsize(msg->user.password);
    int header_length =
        sizeof(username_length)
        + sizeof(password_length)
        + sizeof(msg->user.type)
        + username_length
        + password_length;

    char a[0];
    memcpy(buffer, msg->user.type, sizeof(msg->user.type));
    buffer += sizeof(msg->user.type);
}

void deserialize_response(const char *buffer, Response *res)
{
    memcpy(res, buffer, MAX_RESPONSE_SIZE);
}

void handle_signup(const char *username, const char *password)
{
    Message msg = {0};
    strncpy(msg.user.type, "postman", sizeof(msg.user.type));
    strncpy(msg.user.username, "default", sizeof(msg.user.username));
    strncpy(msg.user.password, "default", sizeof(msg.user.password));
    strncpy(msg.request.type, "signup", sizeof(msg.request.type));
    strncpy(msg.request.arguments.signup_login.username, username, 32);
    strncpy(msg.request.arguments.signup_login.password, password, 32);

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

    Response res;
    deserialize_response(res_buffer, &res);
    if (strcmp(res.status, "success") == 0)
    {
        printf("%s\n", res.message);
    }
    else
    {
        fprintf(stderr, "Error: %s\n", res.message);
        exit(1);
    }
}

void handle_login(const char *username, const char *password)
{
    // Try cached credentials first
    if (client_data.has_login)
    {
        Message msg = {0};
        strncpy(msg.user.type, "postman", 32);
        strncpy(msg.user.username, "default", 32);
        strncpy(msg.user.password, "default", 32);
        strncpy(msg.request.type, "login", 32);
        strncpy(msg.request.arguments.signup_login.username, client_data.username, 32);
        strncpy(msg.request.arguments.signup_login.password, client_data.password, 32);

        char buffer[MAX_MESSAGE_SIZE];
        serialize_message(&msg, buffer);
        send(sock, buffer, MAX_MESSAGE_SIZE, 0);

        char res_buffer[MAX_RESPONSE_SIZE];
        recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

        Response res;
        deserialize_response(res_buffer, &res);
        if (strcmp(res.status, "success") == 0)
        {
            printf("%s\n", res.message);
            return;
        }
    }

    // Try new credentials
    strncpy(client_data.username, username, 32);
    strncpy(client_data.password, password, 32);

    Message msg = {0};
    strncpy(msg.user.type, "postman", 32);
    strncpy(msg.user.username, "default", 32);
    strncpy(msg.user.password, "default", 32);
    strncpy(msg.request.type, "login", 32);
    strncpy(msg.request.arguments.signup_login.username, username, 32);
    strncpy(msg.request.arguments.signup_login.password, password, 32);

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

    Response res;
    deserialize_response(res_buffer, &res);
    if (strcmp(res.status, "success") == 0)
    {
        client_data.has_login = 1;
        printf("%s\n", res.message);
    }
    else
    {
        memset(&client_data, 0, sizeof(client_data));
        fprintf(stderr, "Error: %s\n", res.message);
        exit(1);
    }
}

void handle_logout()
{
    memset(&client_data, 0, sizeof(client_data));
}

void handle_get()
{
    Message msg = {0};
    strncpy(msg.user.type, "postman", 32);
    strncpy(msg.user.username, client_data.username, 32);
    strncpy(msg.user.password, client_data.password, 32);
    strncpy(msg.request.type, "get", 32);

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

    Response res;
    deserialize_response(res_buffer, &res);
    printf("%s\n", res.message);
}

void load_client_data()
{
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp)
    {
        fread(&client_data, sizeof(client_data), 1, fp);
        fclose(fp);
    }
}

void save_client_data()
{
    FILE *fp = fopen(DATA_FILE, "wb");
    if (fp)
    {
        fwrite(&client_data, sizeof(client_data), 1, fp);
        fclose(fp);
    }
}

void cleanup()
{
    closesocket(sock);
    WSACleanup();
}

int main(int argc, char *argv[])
{
    atexit(cleanup);
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    memset(&client_data, 0, sizeof(client_data));
    load_client_data();

    // Setup connection
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket error: %d\n", WSAGetLastError());
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Connect error: %d\n", WSAGetLastError());
        return 1;
    }

    // Parse command
    if (argc < 2)
    {
        fprintf(stderr, "Usage: client [command]\n");
        return 1;
    }

    char *cmd = argv[1];
    if (strcmp(cmd, "signup") == 0)
    {
        char *u = NULL, *p = NULL;
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "--username") == 0)
                u = argv[++i];
            if (strcmp(argv[i], "--password") == 0)
                p = argv[++i];
        }
        if (!u || !p)
        {
            fprintf(stderr, "Missing username/password\n");
            return 1;
        }
        handle_signup(u, p);
    }
    else if (strcmp(cmd, "login") == 0)
    {
        char *u = NULL, *p = NULL;
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "--username") == 0)
                u = argv[++i];
            if (strcmp(argv[i], "--password") == 0)
                p = argv[++i];
        }
        if (!u || !p)
        {
            fprintf(stderr, "Missing username/password\n");
            return 1;
        }
        handle_login(u, p);
    }
    else if (strcmp(cmd, "logout") == 0)
    {
        handle_logout();
    }
    else if (strcmp(cmd, "get") == 0)
    {
        if (!client_data.has_login)
        {
            fprintf(stderr, "Login required\n");
            return 1;
        }
        handle_get();
    }
    else if (strcmp(cmd, "assign") == 0 || strcmp(cmd, "report-loss") == 0)
    {
        char *tid = NULL;
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "--task_id") == 0)
                tid = argv[++i];
        }
        if (!tid)
        {
            fprintf(stderr, "Missing task ID\n");
            return 1;
        }
        Message msg = {0};
        strncpy(msg.user.type, "postman", 32);
        strncpy(msg.user.username, client_data.username, 32);
        strncpy(msg.user.password, client_data.password, 32);
        strncpy(msg.request.type, "delete", 32);
        strncpy(msg.request.arguments.assign_report.id, tid, 32);

        char buffer[MAX_MESSAGE_SIZE];
        serialize_message(&msg, buffer);
        send(sock, buffer, MAX_MESSAGE_SIZE, 0);

        char res_buffer[MAX_RESPONSE_SIZE];
        recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

        Response res;
        deserialize_response(res_buffer, &res);
        printf("%s\n", res.status);
    }

    save_client_data();
    return 0;
}
