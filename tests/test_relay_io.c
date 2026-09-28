/* Host test for relay send_all / connect_retry / poll (POSIX sockets). */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static int g_socket_calls;
static int g_send_real = 1;
static int g_send_calls;
static int g_send_eintr_left;
static unsigned char g_sink[128];
static size_t g_sink_n;

static ssize_t test_send(int fd, const void *buf, size_t n, int flags)
{
    if (g_send_real)
        return send(fd, buf, n, flags);

    g_send_calls++;
    if (g_send_eintr_left > 0)
    {
        g_send_eintr_left--;
        errno = EINTR;
        return -1;
    }
    if (n == 0)
        return 0;
    if (g_sink_n >= sizeof(g_sink))
        return -1;
    memcpy(g_sink + g_sink_n, buf, 1);
    g_sink_n += 1;
    return 1;
}

static int test_socket(int domain, int type, int protocol)
{
    g_socket_calls++;
    return socket(domain, type, protocol);
}

#define RELAY_IO_SEND test_send
#define RELAY_IO_SOCKET test_socket
#include "relay_io.h"

volatile int running = 1;

static void close_if(int *fd)
{
    if (*fd >= 0)
    {
        close(*fd);
        *fd = -1;
    }
}

static int fail(const char *msg)
{
    perror(msg);
    return 1;
}

static int make_loopback_listener(int *out_fd, struct sockaddr_in *out_addr)
{
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0)
        return -1;

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(out_addr, 0, sizeof(*out_addr));
    out_addr->sin_family = AF_INET;
    out_addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    out_addr->sin_port = 0;

    if (bind(srv, (struct sockaddr *)out_addr, sizeof(*out_addr)) < 0)
    {
        close(srv);
        return -1;
    }

    socklen_t alen = sizeof(*out_addr);
    if (getsockname(srv, (struct sockaddr *)out_addr, &alen) < 0)
    {
        close(srv);
        return -1;
    }

    if (listen(srv, 1) < 0)
    {
        close(srv);
        return -1;
    }

    *out_fd = srv;
    return 0;
}

static int bind_closed_port(struct sockaddr_in *out_addr)
{
    int probe = socket(AF_INET, SOCK_STREAM, 0);
    if (probe < 0)
        return -1;
    memset(out_addr, 0, sizeof(*out_addr));
    out_addr->sin_family = AF_INET;
    out_addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    out_addr->sin_port = 0;
    if (bind(probe, (struct sockaddr *)out_addr, sizeof(*out_addr)) < 0)
    {
        close(probe);
        return -1;
    }
    socklen_t plen = sizeof(*out_addr);
    if (getsockname(probe, (struct sockaddr *)out_addr, &plen) < 0)
    {
        close(probe);
        return -1;
    }
    close(probe);
    return 0;
}

static int test_send_all_live(void)
{
    int srv = -1, tcp_fd = -1, acc = -1;
    int rc = 1;
    struct sockaddr_in addr;
    unsigned char payload[48];
    unsigned char got[48];

    g_send_real = 1;
    if (make_loopback_listener(&srv, &addr) != 0)
        return fail("listen");

    if (connect_retry(&tcp_fd, &addr, 50, 1000) != 0)
    {
        fprintf(stderr, "connect_retry to listener failed\n");
        goto out;
    }

    acc = accept(srv, NULL, NULL);
    if (acc < 0)
    {
        perror("accept");
        goto out;
    }

    for (int i = 0; i < 48; ++i)
        payload[i] = (unsigned char)(i + 1);

    if (send_all(tcp_fd, payload, sizeof(payload)) != 0)
    {
        fprintf(stderr, "send_all failed\n");
        goto out;
    }

    size_t nread = 0;
    while (nread < sizeof(got))
    {
        ssize_t r = recv(acc, got + nread, sizeof(got) - nread, 0);
        if (r <= 0)
        {
            fprintf(stderr, "recv failed after %zu bytes\n", nread);
            goto out;
        }
        nread += (size_t)r;
    }

    if (memcmp(payload, got, sizeof(payload)) != 0)
    {
        fprintf(stderr, "payload mismatch\n");
        goto out;
    }
    rc = 0;
out:
    close_if(&acc);
    close_if(&tcp_fd);
    close_if(&srv);
    return rc;
}

static int test_send_all_short_and_eintr(void)
{
    unsigned char payload[48];
    for (int i = 0; i < 48; ++i)
        payload[i] = (unsigned char)(i + 3);

    g_send_real = 0;
    g_send_calls = 0;
    g_send_eintr_left = 1;
    g_sink_n = 0;
    memset(g_sink, 0, sizeof(g_sink));

    if (send_all(-1, payload, sizeof(payload)) != 0)
    {
        fprintf(stderr, "shim send_all failed\n");
        g_send_real = 1;
        return 1;
    }
    g_send_real = 1;

    if (g_send_eintr_left != 0 || g_send_calls != 49 || g_sink_n != 48)
    {
        fprintf(stderr, "short-write/EINTR not exercised (calls=%d eintr_left=%d n=%zu)\n",
                g_send_calls, g_send_eintr_left, g_sink_n);
        return 1;
    }
    if (memcmp(g_sink, payload, sizeof(payload)) != 0)
    {
        fprintf(stderr, "shim payload mismatch\n");
        return 1;
    }
    return 0;
}

static int test_connect_retry_new_fd_each_attempt(void)
{
    int orig = -1, fd = -1;
    int rc = 1;
    struct sockaddr_in ephemeral;
    const int attempts = 3;

    if (bind_closed_port(&ephemeral) != 0)
        return fail("bind probe");

    orig = socket(AF_INET, SOCK_STREAM, 0);
    if (orig < 0)
        return fail("socket orig");
    int keepalive = 1;
    if (setsockopt(orig, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) != 0)
    {
        perror("SO_KEEPALIVE");
        goto out;
    }

    fd = orig;
    orig = -1;
    g_socket_calls = 0;
    if (connect_retry(&fd, &ephemeral, attempts, 1000) == 0)
    {
        fprintf(stderr, "connect_retry unexpectedly succeeded\n");
        goto out;
    }
    if (fd < 0)
    {
        fprintf(stderr, "connect_retry left tcp fd invalid\n");
        goto out;
    }
    if (g_socket_calls != attempts)
    {
        fprintf(stderr, "connect_retry socket() calls=%d want=%d\n",
                g_socket_calls, attempts);
        goto out;
    }

    keepalive = 1;
    socklen_t klen = sizeof(keepalive);
    if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &klen) != 0)
    {
        perror("getsockopt SO_KEEPALIVE");
        goto out;
    }
    if (keepalive != 0)
    {
        fprintf(stderr, "connect_retry reused the failed-connect fd\n");
        goto out;
    }
    rc = 0;
out:
    close_if(&fd);
    close_if(&orig);
    return rc;
}

static int test_poll_tcp_hangup(void)
{
    int udp = -1, srv = -1, tcp_fd = -1, acc = -1;
    int rc = 1;
    struct sockaddr_in addr;
    char buf[256];

    g_send_real = 1;
    udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp < 0)
        return fail("udp socket");

    if (make_loopback_listener(&srv, &addr) != 0)
    {
        perror("listen hangup");
        goto out;
    }
    if (connect_retry(&tcp_fd, &addr, 50, 1000) != 0)
    {
        fprintf(stderr, "connect_retry for hangup test failed\n");
        goto out;
    }
    acc = accept(srv, NULL, NULL);
    if (acc < 0)
    {
        perror("accept hangup");
        goto out;
    }

    close_if(&acc);
    if (relay_poll_once(udp, tcp_fd, buf, sizeof(buf)) != 0)
    {
        fprintf(stderr, "relay_poll_once did not stop on TCP hangup\n");
        goto out;
    }
    rc = 0;
out:
    close_if(&acc);
    close_if(&tcp_fd);
    close_if(&srv);
    close_if(&udp);
    return rc;
}

int main(void)
{
    if (test_send_all_live() != 0)
        return 1;
    if (test_send_all_short_and_eintr() != 0)
        return 1;
    if (test_connect_retry_new_fd_each_attempt() != 0)
        return 1;
    if (test_poll_tcp_hangup() != 0)
        return 1;
    return 0;
}
