#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include "http_get.h"
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(httptest, CONFIG_LOG_DEFAULT_LEVEL);

void print_addrinfo_results(struct zsock_addrinfo **results)
{
	char ipv4[INET_ADDRSTRLEN];
	char ipv6[INET6_ADDRSTRLEN];
	struct sockaddr_in *sa;
	struct sockaddr_in6 *sa6;
	struct zsock_addrinfo *rp;
	
	for (rp = *results; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_addr->sa_family == AF_INET) {
			// IPv4 Address
			sa = (struct sockaddr_in *) rp->ai_addr;
			zsock_inet_ntop(AF_INET, &sa->sin_addr, ipv4, INET_ADDRSTRLEN);
			LOG_INF("IPv4: %s\n", ipv4);
		}
		if (rp->ai_addr->sa_family == AF_INET6) {
			// IPv6 Address
			sa6 = (struct sockaddr_in6 *) rp->ai_addr;
			zsock_inet_ntop(AF_INET6, &sa6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			LOG_INF("IPv6: %s\n", ipv6);
		}
	}
}

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
    struct zsock_addrinfo hints, *res, *p;
    char port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", port);

    // Set up hints for the type of socket we want
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket

    // Perform DNS lookup
    ret = zsock_getaddrinfo(server, port_str, &hints, &res);
    if (ret != 0)
    {
        LOG_ERR("DNS lookup failed for %s (%s)", server, gai_strerror(ret));
        return -1;
    }
    print_addrinfo_results(&res);

    // Loop through results and bind to the first we can
    for (p = res; p != NULL; p = p->ai_next)
    {
        *sock = zsock_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*sock < 0)
        {
            LOG_ERR("Failed to create %s HTTP socket (%d)", family_str, -errno);
            continue;
        }

        // Copy the resolved address into the addr struct
        memcpy(addr, p->ai_addr, p->ai_addrlen);
        addr_len = p->ai_addrlen;

        LOG_INF("Socket created successfully with resolved IP");

        break;
    }

    // Clean up the addrinfo structure
    freeaddrinfo(res);

    if (*sock < 0)
    {
        LOG_ERR("Could not create socket after DNS resolution");
        ret = -1;
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
        LOG_INF("Connected to remote successfully");
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

    static uint8_t recv_buf_ipv4[RESPONSE_BUFFER_SIZE];
    if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4))
    {
        const char *headers[] = {
            "Transfer-Encoding: chunked\r\n",
            NULL // Null-terminate the array
        };

        struct http_request req;

        memset(&req, 0, sizeof(req));

        req.method = HTTP_GET;
        req.url = SERVER_ENDPOINT;
        req.host = SERVER_ADDR4;
        req.protocol = "HTTP/1.1";
        // req.header_fields = headers;
        req.response = http_response_callback;
        req.recv_buf = recv_buf_ipv4;
        req.recv_buf_len = sizeof(recv_buf_ipv4);

        ret = http_client_req(sock4, &req, 2000, "IPv4 GET");

        if (ret < 0)
        {
            LOG_ERR("HTTP request failed: %d", ret);
            LOG_WRN("Receive buffer content:\n%s", recv_buf_ipv4);
        }
        zsock_close(sock4);
    }

    return 0;
}

void dns_result_cb(enum dns_resolve_status status,
		   struct dns_addrinfo *info,
		   void *user_data)
{
	char hr_addr[NET_IPV6_ADDR_LEN];
	char *hr_family;
	void *addr;

	switch (status) {
	case DNS_EAI_CANCELED:
		LOG_INF("DNS query was canceled");
		return;
	case DNS_EAI_FAIL:
		LOG_INF("DNS resolve failed");
		return;
	case DNS_EAI_NODATA:
		LOG_INF("Cannot resolve address");
		return;
	case DNS_EAI_ALLDONE:
		LOG_INF("DNS resolving finished");
		return;
	case DNS_EAI_INPROGRESS:
		break;
	default:
		LOG_INF("DNS resolving error (%d)", status);
		return;
	}

	if (!info) {
		return;
	}

	if (info->ai_family == AF_INET) {
		hr_family = "IPv4";
		addr = &net_sin(&info->ai_addr)->sin_addr;
	} else if (info->ai_family == AF_INET6) {
		hr_family = "IPv6";
		addr = &net_sin6(&info->ai_addr)->sin6_addr;
	} else {
		LOG_ERR("Invalid IP address family %d", info->ai_family);
		return;
	}

	LOG_INF("%s %s address: %s", user_data ? (char *)user_data : "<null>",
		hr_family,
		net_addr_ntop(info->ai_family, addr,
					 hr_addr, sizeof(hr_addr)));
}
