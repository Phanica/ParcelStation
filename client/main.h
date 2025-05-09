#ifndef MAIN_H
#define MAIN_H

#include <winsock2.h>
#include <windows.h>

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_TASK_ID_LENGTH 32
#define MAX_ARGS_LENGTH 256
#define MAX_HEADER_LENGTH 256
#define MAX_REQ_LENGTH 256
#define MAX_MESSAGE_SIZE 2048
#define MAX_RESPONSE_SIZE 2048

typedef enum UserType {
    postman,
    manager,
    user,
} UserType;

typedef enum RequestType {
    signup,
    login,
    logout,
    get,
    list,
    assign,
    report_loss,
    statistics,
    query
} RequestType;

typedef struct User {
    UserType type;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

typedef struct SignupLoginArguments {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} SignupLoginArguments;

typedef struct AssignReportArguments {
    char id[MAX_TASK_ID_LENGTH];
} AssignReportArguments;

typedef union RequestArguments {
    SignupLoginArguments signup_login;
    AssignReportArguments assign_report;
} RequestArguments;

typedef struct Request {
    RequestType type;
    RequestArguments arguments;
} Request;

typedef struct Message {
    User user;
    Request request;
} Message;

typedef struct Response {
    char status[32];
    char message[256];
} Response;

typedef struct ClientData {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int has_login;
    UserType type;
} ClientData;

// 公共函数声明
void init_connection();
void close_connection();
void send_request(Message *msg);
void handle_login(const char *username, const char *password, UserType user_type);
int serialize_message(const Message *msg, char *buffer);
void deserialize_response(const char *buffer, Response *res);

#endif // MAIN_H
