#ifndef COMMON_H
#define COMMON_H

#include "main.h"  // 包含原有定义

size_t _placeholder_1_in_common_h;

void init_connection();
void close_connection();
void send_request(Message *msg);

#endif
