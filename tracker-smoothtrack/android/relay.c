/* Copyright (c) 2026 CyberoniOntoni
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "relay_io.h"

#define PACKET_SIZE 48 // 6 * sizeof(double)
#define MAX_BUFFER 256

volatile int running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char* argv[])
{
    int udp_port = 4242;
    int tcp_port = 4242;
    const char* bind_ip = "127.0.0.1";
    const char* connect_ip = "127.0.0.1";

    if (argc >= 2)
        udp_port = atoi(argv[1]);
    if (argc >= 3)
        tcp_port = atoi(argv[2]);
    if (argc >= 4)
        bind_ip = argv[3];
    if (argc >= 5)
        connect_ip = argv[4];

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGPIPE, SIG_IGN);

    // 1. Setup UDP listener (SmoothTrack sends to 127.0.0.1:udp_port)
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0)
    {
        perror("socket(SOCK_DGRAM)");
        return 1;
    }

    int opt = 1;
    setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(udp_port);
    if (inet_pton(AF_INET, bind_ip, &udp_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid bind IP: %s\n", bind_ip);
        close(udp_fd);
        return 1;
    }

    if (bind(udp_fd, (struct sockaddr*)&udp_addr, sizeof(udp_addr)) < 0)
    {
        perror("bind(udp)");
        close(udp_fd);
        return 2;
    }

    // 2. Setup TCP connection to ADB reverse port (tunnels to host PC OpenTrack)
    struct sockaddr_in tcp_addr;
    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(tcp_port);
    if (inet_pton(AF_INET, connect_ip, &tcp_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid connect IP: %s\n", connect_ip);
        close(udp_fd);
        return 4;
    }

    int tcp_fd = -1;
    // Retry connect for up to 5 seconds while host PC finishes starting listener
    if (connect_retry(&tcp_fd, &tcp_addr, 50, 100000) != 0)
    {
        fprintf(stderr, "Failed to connect to TCP reverse tunnel %s:%d: %s\n",
                connect_ip, tcp_port, strerror(errno));
        close(udp_fd);
        if (tcp_fd >= 0)
            close(tcp_fd);
        return 5;
    }

    fprintf(stderr, "st-relay: ready, bridging UDP %s:%d -> TCP %s:%d\n",
            bind_ip, udp_port, connect_ip, tcp_port);

    // 3. Relay datagrams; poll TCP so a host close is noticed without waiting for UDP
    char buf[MAX_BUFFER];
    while (running)
    {
        if (relay_poll_once(udp_fd, tcp_fd, buf, sizeof(buf)) <= 0)
            break;
    }

    close(udp_fd);
    if (tcp_fd >= 0)
        close(tcp_fd);
    return 0;
}
