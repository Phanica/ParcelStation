#include "common.h"

void user_query_parcel(const char *parcel_id) {
  Message msg = {0};
  msg.user.type = user;
  strncpy(msg.user.username, client_data.username,
          strsize(client_data.username));
  strncpy(msg.user.password, client_data.password,
          strsize(client_data.password));
  msg.request.type = query;
  strncpy(msg.request.arguments.assign_report.id, parcel_id,
          strsize(parcel_id));

  send_request(&msg);
}

// 显示帮助信息的函数
void show_help() {
  printf("Usage: user_client [login|query] -u <username> -p <password> [--id "
         "<parcel_id>]\n");
  printf("Options:\n");
  printf("  -u, --username <username>  User name\n");
  printf("  -p, --password <password>  Password\n");
  printf(
      "  --id <parcel_id>           Parcel ID (required for query action)\n");
}

int main(int argc, char *argv[]) {
  const char *username = NULL;
  const char *password = NULL;
  const char *parcel_id = NULL;
  const char *action = NULL;
  int i;

  // 解析参数
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "login") == 0 || strcmp(argv[i], "query") == 0) {
      if (action == NULL) {
        action = argv[i];
      } else {
        printf("Error: Multiple actions specified.\n");
        show_help();
        return 1;
      }
    } else if (strcmp(argv[i], "-u") == 0 ||
               strcmp(argv[i], "--username") == 0) {
      if (i + 1 < argc) {
        username = argv[++i];
      } else {
        printf("Error: Missing argument for %s.\n", argv[i]);
        show_help();
        return 1;
      }
    } else if (strcmp(argv[i], "-p") == 0 ||
               strcmp(argv[i], "--password") == 0) {
      if (i + 1 < argc) {
        password = argv[++i];
      } else {
        printf("Error: Missing argument for %s.\n", argv[i]);
        show_help();
        return 1;
      }
    } else if (strcmp(argv[i], "--id") == 0) {
      if (i + 1 < argc) {
        parcel_id = argv[++i];
      } else {
        printf("Error: Missing argument for %s.\n", argv[i]);
        show_help();
        return 1;
      }
    } else {
      printf("Error: Unknown option %s.\n", argv[i]);
      show_help();
      return 1;
    }
  }

  // 检查必要参数
  if (action == NULL) {
    printf("Error: Action (login or query) is required.\n");
    show_help();
    return 1;
  }
  if ((strcmp(action, "login") == 0 || strcmp(action, "query") == 0) &&
      (username == NULL || password == NULL)) {
    printf("Error: Username and password are required for %s action.\n",
           action);
    show_help();
    return 1;
  }
  if (strcmp(action, "query") == 0 && parcel_id == NULL) {
    printf("Error: Parcel ID is required for query action.\n");
    show_help();
    return 1;
  }

  init_connection();

  if (strcmp(action, "login") == 0) {
    handle_login(username, password);
  } else if (client_data.has_login && client_data.type == user) {
    if (strcmp(action, "query") == 0) {
      user_query_parcel(parcel_id);
    }
    
    if (strcmp(action, "signup") == 0) {
        handle_signup(username, password);
    }
    else if (strcmp(action, "logout") == 0) {
        handle_logout();
    }
  }

  close_connection();
  return 0;
}
