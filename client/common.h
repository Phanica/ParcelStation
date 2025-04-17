#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include "main.h"  // 包含原有定义

extern ClientData client_data;
extern SOCKET sock;

void init_connection();
void close_connection();
void send_request(Message *msg);
void handle_login(const char *username, const char *password, UserType user_type);

#endif