
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/socket.h>

#define RESPONSE_BUFFER_SIZE (5*1024)
#define SERVER_ADDR4 "www.httpbin.org"
#define SERVER_ENDPOINT "/get"
#define HTTP_PORT 80
#define DNS_TIMEOUT 5000

int http_test(void);