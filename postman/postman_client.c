#include "common.h"

void postman_assign_parcel(const char *parcel_id) {
    Message msg = {0};
    msg.user.type = postman;
    strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
    strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
    msg.request.type = assign;
    strncpy(msg.request.arguments.assign_report.id, parcel_id, MAX_TASK_ID_LENGTH);

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
    if (argc < 4) {
        printf("Usage: postman_client [login|assign|report] [username] [password] [parcel_id]\n");
        return 1;
    }
    
    init_connection();
    
    if (strcmp(argv[1], "login") == 0) {
        handle_login(argv[2], argv[3], postman);
    } else if (client_data.has_login && client_data.type == postman) {
        if (strcmp(argv[1], "assign") == 0) {
            postman_assign_parcel(argv[4]);
        } else if (strcmp(argv[1], "report") == 0) {
            postman_report_loss(argv[4]);
        }
    }
    
    close_connection();
    return 0;
}