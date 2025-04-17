#include "common.h"

ClientData client_data = {0};
SOCKET sock;
WSADATA wsaData;

void init_connection() {
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
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