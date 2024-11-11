
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/socket.h>

#define RESPONSE_BUFFER_SIZE 1024
#define SERVER_ADDR4 "192.168.1.40"
#define HTTP_PORT 80

int http_test(void);