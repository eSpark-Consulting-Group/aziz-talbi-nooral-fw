#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include "http_get.h"
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(httptest, CONFIG_LOG_DEFAULT_LEVEL);

void http_response_callback(struct http_response *rsp, enum http_final_call final_data, void *user_data)
{
    if (final_data == HTTP_DATA_MORE)
    {
        LOG_INF("Partial data received (%zd bytes)", rsp->data_len);
    }
    else if (final_data == HTTP_DATA_FINAL)
    {
        LOG_INF("All the data received (%zd bytes)", rsp->data_len);
    }
    LOG_INF("Response status %s", rsp->http_status);

    if (rsp->body_found)
    {
        LOG_INF("Response Body:%.*s\n", rsp->body_frag_len, rsp->body_frag_start);
    }
    else
    {
        LOG_INF("No response body received\n");
    }
}

static int setup_socket(sa_family_t family, const char *server, int port,
                        int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    const char *family_str = family == AF_INET ? "IPv4" : "IPv6";
    int ret = 0;

    memset(addr, 0, addr_len);

    if (family == AF_INET)
    {
        net_sin(addr)->sin_family = AF_INET;
        net_sin(addr)->sin_port = htons(port);
        inet_pton(family, server, &net_sin(addr)->sin_addr);
    }

    *sock = zsock_socket(family, SOCK_STREAM, IPPROTO_TCP);

    if (*sock < 0)
    {
        LOG_ERR("Failed to create %s HTTP socket (%d)", family_str,
                -errno);
    }
    else
    {
        LOG_INF("Socket created successfully");
    }

    return ret;
}

static int connect_socket(sa_family_t family, const char *server, int port,
                          int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    int ret;

    ret = setup_socket(family, server, port, sock, addr, addr_len);
    if (ret < 0 || *sock < 0)
    {
        return -1;
    }

    ret = zsock_connect(*sock, addr, addr_len);
    if (ret < 0)
    {
        LOG_ERR("Cannot connect to %s remote (%d)",
                family == AF_INET ? "IPv4" : "IPv6",
                -errno);
        ret = -errno;
    }
    else
    {
        LOG_INF("Connected to remote %d", ret);
    }

    return ret;
}

int http_test(void)
{
    int sock4 = -1, ret = 0;
    struct sockaddr_in addr4;
    connect_socket(AF_INET, SERVER_ADDR4, HTTP_PORT,
                   &sock4, (struct sockaddr *)&addr4,
                   sizeof(addr4));

    if (sock4 < 0)
    {
        LOG_ERR("Cannot create HTTP connection.");
        return -ECONNABORTED;
    }

    uint8_t recv_buf_ipv4[RESPONSE_BUFFER_SIZE];
    if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4))
    {
        const char *headers[] = {
            "Host: " SERVER_ADDR4,
            "User-Agent: MyClient/1.0",
            NULL // Null-terminate the array
        };
        struct http_request req;

        memset(&req, 0, sizeof(req));

        req.method = HTTP_GET;
        req.url = "/boot-config";
        req.host = SERVER_ADDR4;
        req.content_type_value = "text/plain";
        req.protocol = "HTTP/1.1";
        req.header_fields = headers;
        req.response = http_response_callback;
        req.recv_buf = recv_buf_ipv4;
        req.recv_buf_len = sizeof(recv_buf_ipv4);

        ret = http_client_req(sock4, &req, 100, "IPv4 GET");

        if (ret < 0)
        {
            LOG_ERR("HTTP request failed: %d", ret);
            LOG_WRN("Receive buffer content:\n%s", recv_buf_ipv4);
        }
        close(sock4);
    }

    return 0;
}