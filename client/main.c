#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma pack(push, 1)

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_TASK_ID_LENGTH 32
#define MAX_ARGS_LENGTH 256
#define MAX_HEADER_LENGTH 256
#define MAX_REQ_LENGTH 256

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

void memcpyforwards(void **buffer_ptr, const void *src, size_t size)
{
    memcpy(*buffer_ptr, src, size);
    *buffer_ptr += size;
}

int calculate_signup_login_args_size(const SignupLoginArguments *args, char *buffer)
{
    int username_length = strsize(args->username);
    int password_length = strsize(args->password);
    int args_length =
        sizeof(username_length) +
        sizeof(password_length) +
        username_length +
        password_length;

    memcpyforwards(&buffer, &username_length, sizeof(username_length));
    memcpyforwards(&buffer, &password_length, sizeof(password_length));
    memcpyforwards(&buffer, args->username, username_length);
    memcpyforwards(&buffer, args->password, password_length);

    return args_length;
}

/* calculate_get_list_args_size = delete */

int calculate_assign_report_args_size(const AssignReportArguments *args, char *buffer)
{
    int id_length = strsize(args->id);
    int args_length =
        sizeof(id_length) +
        id_length;

    memcpyforwards(&buffer, &id_length, sizeof(id_length));
    memcpyforwards(&buffer, args->id, id_length);

    return args_length;
}

int calculate_arguments_size(const RequestArguments *args, char *buffer)
{
}

int calculate_request_size(const Request *req, char *buffer)
{
    switch (req->type)
    {
    case signup:
    case login:
    {
        char *args_buffer[MAX_ARGS_LENGTH];
        int args_length = calculate_signup_login_args_size(&req->arguments.signup_login, args_buffer);

        int request_size =
            sizeof(args_length) +
            sizeof(req->type) +
            args_length;

        memcpyforwards(&buffer, &args_length, sizeof(args_length));
        memcpyforwards(&buffer, req->type, sizeof(req->type));
        memcpyforwards(&buffer, args_buffer, args_length);

        return request_size;
        break;
    }

    case get:
    case list:
    {
        int args_length = 0;

        int request_size =
            sizeof(args_length) +
            sizeof(req->type) +
            args_length;

        memcpyforwards(&buffer, &args_length, sizeof(args_length));
        memcpyforwards(&buffer, &req->type, sizeof(req->type));

        return request_size;
        break;
    }

    case assign:
    case report_loss:
    {
        char *args_buffer[MAX_ARGS_LENGTH];
        int args_length = calculate_assign_report_args_size(&req->arguments.signup_login, args_buffer);

        int request_size =
            sizeof(args_length) +
            sizeof(req->type) +
            args_length;

        memcpyforwards(&buffer, &args_length, sizeof(args_length));
        memcpyforwards(&buffer, req->type, sizeof(req->type));
        memcpyforwards(&buffer, args_buffer, args_length);

        return request_size;
        break;
    }

    default:
        return 0;
    }
}

int calculate_header_size(const User *user, char *buffer)
{
    int username_length = strsize(user->username);
    int password_length = strsize(user->password);

    int header_size =
        sizeof(username_length) +
        sizeof(password_length) +
        sizeof(user->type) +
        username_length +
        password_length;

    memcpyforwards(&buffer, &username_length, sizeof(username_length));
    memcpyforwards(&buffer, &password_length, sizeof(password_length));
    memcpyforwards(&buffer, &user->type, sizeof(user->type));
    memcpyforwards(&buffer, user->username, username_length);
    memcpyforwards(&buffer, user->password, password_length);
}

int serialize_message(const Message *msg, char *buffer)
{
    char header_buffer[MAX_HEADER_LENGTH];
    int header_length = calculate_header_size(&msg->user, header_buffer);

    char req_buffer[MAX_REQ_LENGTH];
    int req_length = calculate_request_size(&msg->request, req_buffer);

    int message_length =
        sizeof(header_length) +
        sizeof(req_length) +
        header_length +
        req_length;

    memcpyforwards(&buffer, &header_length, sizeof(header_length));
    memcpyforwards(&buffer, &req_length, sizeof(req_length));
    memcpyforwards(&buffer, header_buffer, header_length);
    memcpyforwards(&buffer, req_buffer, req_length);

    return message_length;
}

void deserialize_response(const char *buffer, Response *res)
{
    memcpy(res, buffer, MAX_RESPONSE_SIZE);
}

void handle_signup(const char *username, const char *password)
{
    Message msg;
    msg.user.type = postman;
    strncpy(msg.user.username, "default", strsize("default"));
    strncpy(msg.user.password, "default", strsize("default"));
    msg.request.type = signup;
    strncpy(msg.request.arguments.signup_login.username, username, strsize(username));
    strncpy(msg.request.arguments.signup_login.password, password, strsize(password));

    char buffer[MAX_MESSAGE_SIZE];
    int message_length = serialize_message(&msg, buffer);
    send(sock, buffer, message_length, 0);

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

void handle_login(const char *username, const char *password, UserType user_type)
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
    client_data.type = user_type;  // 保存用户类型

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

void handle_manager_statistics() {
    Message msg = {0};
    msg.user.type = manager;
    strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
    strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
    msg.request.type = statistics;

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);
    
    Response res;
    deserialize_response(res_buffer, &res);
    printf("Statistics: %s\n", res.message);
}

void handle_user_query(const char *parcel_id) {
    Message msg = {0};
    msg.user.type = user;
    strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
    strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
    msg.request.type = query;
    strncpy(msg.request.arguments.assign_report.id, parcel_id, MAX_TASK_ID_LENGTH);

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);
    
    Response res;
    deserialize_response(res_buffer, &res);
    printf("Query Result: %s\n", res.message);
}

int main(int argc, char *argv[]) {
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
