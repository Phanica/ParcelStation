#include "common.h"

void manager_get_statistics() {
    Message msg = {0};
    msg.user.type = manager;
    strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
    strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
    msg.request.type = statistics;

    send_request(&msg);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: manager_client [login|stats] [username] [password]\n");
        return 1;
    }
    
    init_connection();
    
    if (strcmp(argv[1], "login") == 0) {
        handle_login(argv[2], argv[3]);
    } else if (client_data.has_login && client_data.type == manager) {
        if (strcmp(argv[1], "stats") == 0) {
            manager_get_statistics();
        }
    }
    
    if (strcmp(argv[1], "signup") == 0) {
        handle_signup(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "logout") == 0) {
        handle_logout();
    }
    close_connection();
    return 0;
}
