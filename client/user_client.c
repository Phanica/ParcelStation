#include "common.h"

void user_query_parcel(const char *parcel_id) {
  Message msg = {0};
  msg.user.type = user;
  strncpy(msg.user.username, client_data.username, MAX_USERNAME_LENGTH);
  strncpy(msg.user.password, client_data.password, MAX_PASSWORD_LENGTH);
  msg.request.type = query;
  strncpy(msg.request.arguments.assign_report.id, parcel_id,
          MAX_TASK_ID_LENGTH);

  send_request(&msg);
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf(
        "Usage: user_client [login|query] [username] [password] [parcel_id]\n");
    return 1;
  }

  init_connection();

  if (strcmp(argv[1], "login") == 0) {
    handle_login(argv[2], argv[3], user);
  } else if (client_data.has_login && client_data.type == user) {
    if (strcmp(argv[1], "query") == 0) {
      user_query_parcel(argv[4]);
    }
  }

  close_connection();
  return 0;
}
