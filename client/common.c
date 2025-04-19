#include "common.h"

#define DATA_FILE "data.dat"

// ClientData client_data = {0};
// SOCKET sock;
// WSADATA wsaData;

void init_connection() {
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int _ = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
}

void close_connection() {
    closesocket(sock);
    WSACleanup();
}

void send_request(Message *msg) {
    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);
    
    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);
    
    Response res;
    deserialize_response(res_buffer, &res);
    printf("Response: %s - %s\n", res.status, res.message);
}

size_t strsize(const char *str) { return strlen(str) + 1; }

void memcpyforwards(char **buffer_ptr, const void *src, size_t size) {
  memcpy(*buffer_ptr, src, size);
  *buffer_ptr += size;
}

int calculate_signup_login_args_size(const SignupLoginArguments *args,
                                     char *buffer) {
  int username_length = strsize(args->username);
  int password_length = strsize(args->password);
  int args_length = sizeof(username_length) + sizeof(password_length) +
                    username_length + password_length;

  memcpyforwards(&buffer, &username_length, sizeof(username_length));
  memcpyforwards(&buffer, &password_length, sizeof(password_length));
  memcpyforwards(&buffer, args->username, username_length);
  memcpyforwards(&buffer, args->password, password_length);

  return args_length;
}

/* calculate_get_list_args_size = delete */

int calculate_assign_report_args_size(const AssignReportArguments *args,
                                      char *buffer) {
  int id_length = strsize(args->id);
  int args_length = sizeof(id_length) + id_length;

  memcpyforwards(&buffer, &id_length, sizeof(id_length));
  memcpyforwards(&buffer, args->id, id_length);

  return args_length;
}

int calculate_arguments_size(const RequestArguments *args, char *buffer) {
  return 0; // Placeholder for actual implementation;
}

int calculate_request_size(const Request *req, char *buffer) {
  switch (req->type) {
  case signup:
  case login: {
    char args_buffer[MAX_ARGS_LENGTH];
    int args_length = calculate_signup_login_args_size(
        &req->arguments.signup_login, args_buffer);

    int request_size = sizeof(args_length) + sizeof(req->type) + args_length;

    memcpyforwards(&buffer, &args_length, sizeof(args_length));
    memcpyforwards(&buffer, &req->type, sizeof(req->type));
    memcpyforwards(&buffer, args_buffer, args_length);

    return request_size;
    break;
  }

  case get:
  case list: {
    int args_length = 0;

    int request_size = sizeof(args_length) + sizeof(req->type) + args_length;

    memcpyforwards(&buffer, &args_length, sizeof(args_length));
    memcpyforwards(&buffer, &req->type, sizeof(req->type));

    return request_size;
    break;
  }

  case assign:
  case report_loss: {
    char args_buffer[MAX_ARGS_LENGTH];
    int args_length = calculate_assign_report_args_size(
        &req->arguments.assign_report, args_buffer);

    int request_size = sizeof(args_length) + sizeof(req->type) + args_length;

    memcpyforwards(&buffer, &args_length, sizeof(args_length));
    memcpyforwards(&buffer, &req->type, sizeof(req->type));
    memcpyforwards(&buffer, args_buffer, args_length);

    return request_size;
    break;
  }

  default:
    return 0;
  }
}

int calculate_header_size(const User *user, char *buffer) {
  int username_length = strsize(user->username);
  int password_length = strsize(user->password);

  int header_size = sizeof(username_length) + sizeof(password_length) +
                    sizeof(user->type) + username_length + password_length;

  memcpyforwards(&buffer, &username_length, sizeof(username_length));
  memcpyforwards(&buffer, &password_length, sizeof(password_length));
  memcpyforwards(&buffer, &user->type, sizeof(user->type));
  memcpyforwards(&buffer, user->username, username_length);
  memcpyforwards(&buffer, user->password, password_length);

  return header_size;
}

int serialize_message(const Message *msg, char *buffer) {
  char header_buffer[MAX_HEADER_LENGTH];
  int header_length = calculate_header_size(&msg->user, header_buffer);

  char req_buffer[MAX_REQ_LENGTH];
  int req_length = calculate_request_size(&msg->request, req_buffer);

  int message_length =
      sizeof(header_length) + sizeof(req_length) + header_length + req_length;

  memcpyforwards(&buffer, &header_length, sizeof(header_length));
  memcpyforwards(&buffer, &req_length, sizeof(req_length));
  memcpyforwards(&buffer, header_buffer, header_length);
  memcpyforwards(&buffer, req_buffer, req_length);

  return message_length;
}

void deserialize_response(const char *buffer, Response *res) {
  memcpy(res, buffer, MAX_RESPONSE_SIZE);
}

void handle_signup(const char *username, const char *password) {
  Message msg;
  msg.user.type = postman;
  strncpy(msg.user.username, "default", strsize("default"));
  strncpy(msg.user.password, "default", strsize("default"));
  msg.request.type = signup;
  strncpy(msg.request.arguments.signup_login.username, username,
          strsize(username));
  strncpy(msg.request.arguments.signup_login.password, password,
          strsize(password));

  char buffer[MAX_MESSAGE_SIZE];
  int message_length = serialize_message(&msg, buffer);
  send(sock, buffer, message_length, 0);

  char res_buffer[MAX_RESPONSE_SIZE];
  recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

  Response res;
  deserialize_response(res_buffer, &res);
  if (strcmp(res.status, "success") == 0) {
    printf("%s\n", res.message);
  } else {
    fprintf(stderr, "Error: %s\n", res.message);
    exit(1);
  }
}

void handle_login(const char *username, const char *password) {
  // Try cached credentials first
  if (client_data.has_login) {
    Message msg = {0};
    msg.user.type = postman;
    strncpy(msg.user.username, "default", 32);
    strncpy(msg.user.password, "default", 32);
    msg.request.type = login;
    strncpy(msg.request.arguments.signup_login.username, client_data.username,
            32);
    strncpy(msg.request.arguments.signup_login.password, client_data.password,
            32);

    char buffer[MAX_MESSAGE_SIZE];
    serialize_message(&msg, buffer);
    send(sock, buffer, MAX_MESSAGE_SIZE, 0);

    char res_buffer[MAX_RESPONSE_SIZE];
    recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

    Response res;
    deserialize_response(res_buffer, &res);
    if (strcmp(res.status, "success") == 0) {
      printf("%s\n", res.message);
      return;
    }
  }

  // Try new credentials
  strncpy(client_data.username, username, 32);
  strncpy(client_data.password, password, 32);
  // client_data.type = user_type;  // 保存用户类型

  Message msg = {0};
  // 原代码尝试将字符串赋值给枚举类型变量，存在类型不匹配问题。
  // 这里直接将枚举类型的对应值赋给 msg.user.type。
  msg.user.type = postman;
  strncpy(msg.user.username, "default", 32);
  strncpy(msg.user.password, "default", 32);
  // 原代码尝试将字符串赋值给枚举类型变量，存在类型不匹配问题。
  // 这里直接将枚举类型的对应值赋给 msg.request.type。
  msg.request.type = login;
  strncpy(msg.request.arguments.signup_login.username, username, 32);
  strncpy(msg.request.arguments.signup_login.password, password, 32);

  char buffer[MAX_MESSAGE_SIZE];
  serialize_message(&msg, buffer);
  send(sock, buffer, MAX_MESSAGE_SIZE, 0);

  char res_buffer[MAX_RESPONSE_SIZE];
  recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

  Response res;
  deserialize_response(res_buffer, &res);
  if (strcmp(res.status, "success") == 0) {
    client_data.has_login = 1;
    printf("%s\n", res.message);
  } else {
    memset(&client_data, 0, sizeof(client_data));
    fprintf(stderr, "Error: %s\n", res.message);
    exit(1);
  }
}

void handle_logout() { memset(&client_data, 0, sizeof(client_data)); }

void handle_get() {
  Message msg = {0};
  msg.user.type = postman;
  strncpy(msg.user.username, client_data.username, 32);
  strncpy(msg.user.password, client_data.password, 32);
  msg.request.type = get;

  char buffer[MAX_MESSAGE_SIZE];
  serialize_message(&msg, buffer);
  send(sock, buffer, MAX_MESSAGE_SIZE, 0);

  char res_buffer[MAX_RESPONSE_SIZE];
  recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

  Response res;
  deserialize_response(res_buffer, &res);
  printf("%s\n", res.message);
}

void load_client_data() {
  FILE *fp = fopen(DATA_FILE, "rb");
  if (fp) {
    fread(&client_data, sizeof(client_data), 1, fp);
    fclose(fp);
  }
}

void save_client_data() {
  FILE *fp = fopen(DATA_FILE, "wb");
  if (fp) {
    fwrite(&client_data, sizeof(client_data), 1, fp);
    fclose(fp);
  }
}

void cleanup() {
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
  strncpy(msg.request.arguments.assign_report.id, parcel_id,
          MAX_TASK_ID_LENGTH);

  char buffer[MAX_MESSAGE_SIZE];
  serialize_message(&msg, buffer);
  send(sock, buffer, MAX_MESSAGE_SIZE, 0);

  char res_buffer[MAX_RESPONSE_SIZE];
  recv(sock, res_buffer, MAX_RESPONSE_SIZE, 0);

  Response res;
  deserialize_response(res_buffer, &res);
  printf("Query Result: %s\n", res.message);
}
