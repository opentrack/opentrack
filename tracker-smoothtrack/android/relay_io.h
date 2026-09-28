/* Copyright (c) 2026 CyberoniOntoni
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#ifndef RELAY_IO_H
#define RELAY_IO_H

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <poll.h>

#ifndef RELAY_IO_SEND
#define RELAY_IO_SEND send
#endif
#ifndef RELAY_IO_SOCKET
#define RELAY_IO_SOCKET socket
#endif

extern volatile int running;

static int send_all(int fd, const void *buf, size_t n)
{
    const char *p = buf;
    size_t left = n;
    while (left)
    {
        ssize_t w = RELAY_IO_SEND(fd, p, left, 0);
        if (w < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        /* peer closed the stream; a short write would be w > 0 */
        if (w == 0)
            return -1;
        p += (size_t)w;
        left -= (size_t)w;
    }
    return 0;
}

/* POSIX leaves socket state unspecified after a failed connect(); recreate fd. */
static int connect_retry(int *tcp_fd, const struct sockaddr_in *addr,
                         int attempts, int delay_us)
{
    for (int retry = 0; retry < attempts && running; ++retry)
    {
        if (*tcp_fd >= 0)
        {
            close(*tcp_fd);
            *tcp_fd = -1;
        }
        *tcp_fd = RELAY_IO_SOCKET(AF_INET, SOCK_STREAM, 0);
        if (*tcp_fd < 0)
            return -1;
        int nodelay = 1;
        setsockopt(*tcp_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
        if (connect(*tcp_fd, (const struct sockaddr *)addr, sizeof(*addr)) == 0)
            return 0;
        usleep(delay_us);
    }
    return -1;
}

/* 1 = keep going, 0 = hangup/stop, -1 = poll error */
static int relay_poll_once(int udp_fd, int tcp_fd, char *buf, size_t bufsz)
{
    struct pollfd pfds[2];
    pfds[0].fd = udp_fd;
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;
    pfds[1].fd = tcp_fd;
    pfds[1].events = POLLIN;
    pfds[1].revents = 0;

    int pr = poll(pfds, 2, -1);
    if (pr < 0)
    {
        if (errno == EINTR)
            return 1;
        return -1;
    }

    if (pfds[1].revents & (POLLHUP | POLLERR | POLLNVAL))
        return 0;
    if (pfds[1].revents & POLLIN)
    {
        char dummy;
        ssize_t r = recv(tcp_fd, &dummy, 1, 0);
        if (r <= 0)
            return 0;
    }

    if (!(pfds[0].revents & POLLIN))
        return 1;

    ssize_t n = recv(udp_fd, buf, bufsz, 0);
    if (n <= 0)
    {
        if (n < 0 && errno == EINTR)
            return 1;
        return 0;
    }

    if (send_all(tcp_fd, buf, (size_t)n) != 0)
        return 0;
    return 1;
}

#endif /* RELAY_IO_H */
