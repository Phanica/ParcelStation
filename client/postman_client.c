#include "common.h"

void postman_assign_parcel(const char *parcel_id) {
  Message msg = {0};
  msg.user.type = postman;
  strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
  strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
  msg.request.type = assign;
  strncpy(msg.request.arguments.assign_report.id, parcel_id,
          MAX_TASK_ID_LENGTH);

  send_request(&msg);
}

void postman_report_loss(const char *parcel_id) {
  Message msg = {0};
  msg.user.type = postman;
  // ... 类似assign的代码 ...
  msg.request.type = report_loss;
  // ...
  send_request(&msg);
}

int main(int argc, char *argv[]) {
    const char *username = NULL;
    const char *password = NULL;
    const char *parcel_id = NULL;
    const char *command = NULL;

    // 帮助信息
    const char *help_info = "Usage: postman_client [login|assign|report] -u <username> -p <password> [--id <parcel_id>]\n";

    if (argc < 2) {
        printf("%s", help_info);
        return 1;
    }

    command = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--username") == 0) {
            if (i + 1 < argc) {
                username = argv[++i];
            } else {
                printf("Error: Missing argument for %s\n", argv[i]);
                printf("%s", help_info);
                return 1;
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--password") == 0) {
            if (i + 1 < argc) {
                password = argv[++i];
            } else {
                printf("Error: Missing argument for %s\n", argv[i]);
                printf("%s", help_info);
                return 1;
            }
        } else if (strcmp(argv[i], "--id") == 0) {
            if (i + 1 < argc) {
                parcel_id = argv[++i];
            } else {
                printf("Error: Missing argument for %s\n", argv[i]);
                printf("%s", help_info);
                return 1;
            }
        } else {
            printf("Error: Unknown option %s\n", argv[i]);
            printf("%s", help_info);
            return 1;
        }
    }

    if (strcmp(command, "login") == 0) {
        if (username == NULL || password == NULL) {
            printf("Error: Username and password are required for login.\n");
            printf("%s", help_info);
            return 1;
        }
    } else if (strcmp(command, "assign") == 0 || strcmp(command, "report") == 0) {
        if (username == NULL || password == NULL || parcel_id == NULL) {
            printf("Error: Username, password and parcel ID are required for %s.\n", command);
            printf("%s", help_info);
            return 1;
        }
    } else {
        printf("Error: Unknown command %s\n", command);
        printf("%s", help_info);
        return 1;
    }

    init_connection();

    if (strcmp(command, "login") == 0) {
        handle_login(username, password);
    } else if (client_data.has_login && client_data.type == postman) {
        if (strcmp(command, "assign") == 0) {
            postman_assign_parcel(parcel_id);
        } else if (strcmp(command, "report") == 0) {
            postman_report_loss(parcel_id);
        }
    }

    if (strcmp(command, "signup") == 0) {
        if (username == NULL || password == NULL) {
            printf("Error: 注册需要用户名和密码\n");
            return 1;
        }
        handle_signup(username, password);
    } 
    else if (strcmp(command, "logout") == 0) {
        handle_logout();
    }
    close_connection();
    return 0;
}
