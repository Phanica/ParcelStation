#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_TASK_ID_LENGTH 32

typedef enum UserType { POSTMAN, MANAGER, USER } UserType;

typedef enum RequestType {
  SIGNUP,
  LOGIN,
  LOGOUT,
  GET,
  LIST,
  ASSIGN,
  REPORT_LOSS,
  STATISTICS,
  QUERY
} RequestType;

typedef enum ResponseStatus { SUCCESS, ERR } ResponseStatus;

typedef struct Parcel {
  char id[MAX_TASK_ID_LENGTH];
  struct Parcel *next;
} Parcel;

typedef struct UserData {
  char username[MAX_USERNAME_LENGTH];
  char password[MAX_PASSWORD_LENGTH];
  Parcel *parcels;
  struct UserData *next;
} UserData;

typedef struct Database {
  UserData *postmen;
  UserData *managers;
  UserData *users;
} Database;

Database database;

void initialize_database() {
  memset(&database, 0, sizeof(Database));

  // 添加默认管理员
  UserData *default_manager = (UserData *)malloc(sizeof(UserData));
  strcpy(default_manager->username, "admin");
  strcpy(default_manager->password, "admin123");
  default_manager->parcels = NULL;
  default_manager->next = NULL;
  database.managers = default_manager;

  // 添加默认快递员
  UserData *default_postman = (UserData *)malloc(sizeof(UserData));
  strcpy(default_postman->username, "default");
  strcpy(default_postman->password, "default");
  default_postman->parcels = NULL;
  default_postman->next = NULL;
  database.postmen = default_postman;

  // 添加默认普通用户
  UserData *default_user = (UserData *)malloc(sizeof(UserData));
  strcpy(default_user->username, "user");
  strcpy(default_user->password, "user123");
  default_user->parcels = NULL;
  default_user->next = NULL;
  database.users = default_user;
}

UserData *find_user(UserData *list, const char *username) {
  UserData *current = list;
  while (current != NULL) {
    if (strcmp(current->username, username) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void send_response(SOCKET sock, ResponseStatus status, const char *message) {
  int message_length = strlen(message) + 1;
  int total_size = sizeof(int) + sizeof(ResponseStatus) + message_length;

  char *buffer = (char *)malloc(total_size);
  char *ptr = buffer;

  memcpy(ptr, &message_length, sizeof(int));
  ptr += sizeof(int);

  memcpy(ptr, &status, sizeof(ResponseStatus));
  ptr += sizeof(ResponseStatus);

  memcpy(ptr, message, message_length);

  send(sock, buffer, total_size, 0);
  free(buffer);
}

void parse_request(const char *buffer, char **username, char **password,
                   RequestType *request_type, char **arguments,
                   int *arguments_length) {
  const char *ptr = buffer;

  // 解析header
  int username_length = *(int *)ptr;
  ptr += sizeof(int);
  int password_length = *(int *)ptr;
  ptr += sizeof(int);

  *username = (char *)malloc(username_length);
  memcpy(*username, ptr, username_length);
  ptr += username_length;

  *password = (char *)malloc(password_length);
  memcpy(*password, ptr, password_length);
  ptr += password_length;

  // 解析request
  *request_type = *(RequestType *)ptr;
  ptr += sizeof(RequestType);

  *arguments_length = *(int *)ptr;
  ptr += sizeof(int);

  *arguments = (char *)malloc(*arguments_length);
  memcpy(*arguments, ptr, *arguments_length);
}

void handle_postman_request(SOCKET sock, UserData *user,
                            RequestType request_type, const char *arguments,
                            int arguments_length) {
  switch (request_type) {
  case ASSIGN: {
    char id[MAX_TASK_ID_LENGTH];
    memcpy(id, arguments, MAX_TASK_ID_LENGTH);

    // 检查包裹是否已存在
    Parcel *current = user->parcels;
    while (current != NULL) {
      if (strcmp(current->id, id) == 0) {
        send_response(sock, ERR, "parcel already exists");
        return;
      }
      current = current->next;
    }

    // 添加新包裹
    Parcel *new_parcel = (Parcel *)malloc(sizeof(Parcel));
    strcpy(new_parcel->id, id);
    new_parcel->next = user->parcels;
    user->parcels = new_parcel;

    send_response(sock, SUCCESS, "parcel added");
    break;
  }
  case SIGNUP:
    send_response(sock, ERR, "Postman cannot perform signup");
    break;
  case LOGIN:
    send_response(sock, ERR, "Postman login should be handled elsewhere");
    break;
  case LOGOUT:
    send_response(sock, SUCCESS, "Postman logged out");
    break;
  case GET: {
    if (arguments_length >= MAX_TASK_ID_LENGTH) {
      char id[MAX_TASK_ID_LENGTH];
      memcpy(id, arguments, MAX_TASK_ID_LENGTH);
      Parcel *current = user->parcels;
      while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
          send_response(sock, SUCCESS, "Parcel found");
          return;
        }
        current = current->next;
      }
      send_response(sock, ERR, "Parcel not found");
    } else {
      send_response(sock, ERR, "Invalid arguments for GET");
    }
    break;
  }
  case LIST: {
    char response[BUFFER_SIZE] = "";
    Parcel *current = user->parcels;
    if (current == NULL) {
      send_response(sock, SUCCESS, "No parcels assigned");
    } else {
      strcat(response, "Assigned parcels: ");
      while (current != NULL) {
        strcat(response, current->id);
        if (current->next != NULL) {
          strcat(response, ", ");
        }
        current = current->next;
      }
      send_response(sock, SUCCESS, response);
    }
    break;
  }
  case REPORT_LOSS: {
    if (arguments_length >= MAX_TASK_ID_LENGTH) {
      char id[MAX_TASK_ID_LENGTH];
      memcpy(id, arguments, MAX_TASK_ID_LENGTH);
      Parcel *prev = NULL;
      Parcel *current = user->parcels;
      while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
          if (prev == NULL) {
            user->parcels = current->next;
          } else {
            prev->next = current->next;
          }
          free(current);
          send_response(sock, SUCCESS, "Parcel reported lost and removed");
          return;
        }
        prev = current;
        current = current->next;
      }
      send_response(sock, ERR, "Parcel not found");
    } else {
      send_response(sock, ERR, "Invalid arguments for REPORT_LOSS");
    }
    break;
  }
  case STATISTICS:
    send_response(sock, ERR, "Postman cannot perform statistics");
    break;
  case QUERY:
    send_response(sock, ERR, "Postman cannot perform query");
    break;
  default:
    send_response(sock, ERR, "Unknown request type");
    break;
  }
}

void process_request(SOCKET sock, const char *buffer, int buffer_size) {
  char *username = NULL;
  char *password = NULL;
  RequestType request_type;
  char *arguments = NULL;
  int arguments_length = 0;

  parse_request(buffer, &username, &password, &request_type, &arguments,
                &arguments_length);

  // 查找用户
  UserData *user = find_user(database.postmen, username);
  if (user == NULL) {
    user = find_user(database.managers, username);
  }
  if (user == NULL) {
    user = find_user(database.users, username);
  }

  if (user != NULL && strcmp(user->password, password) == 0) {
    if (user->parcels != NULL) { // 快递员
      handle_postman_request(sock, user, request_type, arguments,
                             arguments_length);
    } else if (request_type == STATISTICS) { // 管理员
      // 处理统计请求
      char response[64];
      int count = 0;
      UserData *current = database.postmen;
      while (current != NULL) {
        count++;
        current = current->next;
      }
      sprintf(response, "postman_count: %d", count);
      send_response(sock, SUCCESS, response);
    } else if (request_type == QUERY) { // 普通用户
      // 处理查询请求
      send_response(sock, SUCCESS, "query result");
    }
  } else {
    send_response(sock, ERR, "login failed");
  }

  free(username);
  free(password);
  free(arguments);
}

int main() {
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  initialize_database();

  SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8080);

  int _ = bind(server_socket, (SOCKADDR *)&server_addr, sizeof(server_addr));
  listen(server_socket, SOMAXCONN);

  while (1) {
    SOCKET client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET)
      continue;

    char buffer[BUFFER_SIZE];
    int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (recv_size > 0) {
      process_request(client_socket, buffer, recv_size);
    }

    closesocket(client_socket);
  }

  closesocket(server_socket);
  WSACleanup();
  return 0;
}
